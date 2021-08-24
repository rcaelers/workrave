// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "commonui/nls.h"
#include "debug.hh"

#include <iostream>
#include <gtkmm.h>

#include <glibmm.h>
#include <gtk/gtk.h>

#ifdef PLATFORM_OS_WINDOWS_NATIVE
#  undef HAVE_UNISTD_H
#endif

#include <cstdio>
#include <cassert>
#include <fcntl.h>

#include "GUI.hh"

// Library includes
#include "config/IConfigurator.hh"
// Backend includes.
#include "core/IBreak.hh"
#include "core/IBreakResponse.hh"
#include "IBreakWindow.hh"
#include "core/ICore.hh"

#include "commonui/Backend.hh"

#include "utils/Exception.hh"
#include "Locale.hh"
#include "utils/Platform.hh"
#include "AppletControl.hh"
#include "AppletWindow.hh"
#include "BreakWindow.hh"
#include "DailyLimitWindow.hh"
#include "commonui/GUIConfig.hh"
#include "MainWindow.hh"
#include "Menus.hh"
#include "MicroBreakWindow.hh"
#include "PreludeWindow.hh"
#include "RestBreakWindow.hh"
#include "commonui/SoundTheme.hh"
#include "StatusIcon.hh"
#include "session/System.hh"
#include "Text.hh"
#include "Grab.hh"
#include "commonui/Session.hh"
#include "commonui/TimerBoxControl.hh"

#if defined(PLATFORM_OS_WINDOWS)
#  include "W32AppletWindow.hh"
#  include <gdk/gdkwin32.h>
#  ifndef PLATFORM_OS_WINDOWS_NATIVE
#    include <pbt.h>
#  endif
#  include <wtsapi32.h>
#  include <dbt.h>
#  include <windows.h>
#endif

#if defined(PLATFORM_OS_MACOS)
#  include "MacOSUtil.hh"
#  include <strings.h>
#  include <mach-o/dyld.h>
#  include <sys/param.h>
#  import <Cocoa/Cocoa.h>
#  import "AppController.h"
#  include <Carbon/Carbon.h>
#endif

#if defined(PLATFORM_OS_UNIX)
#  include <X11/Xlib.h>
#endif

#if defined(interface)
#  undef interface
#endif
#include "dbus/IDBus.hh"

#ifdef HAVE_GTK_MAC_INTEGRATION
#  include "gtkosxapplication.h"
#endif

GUI *GUI::instance = nullptr;

using namespace std;
using namespace workrave;
using namespace workrave::utils;

GUI::GUI(int argc, char **argv, std::shared_ptr<IToolkit> toolkit)
  : toolkit(toolkit)
{
  TRACE_ENTER("GUI:GUI");

  assert(!instance);
  instance = this;

  this->argc = argc;
  this->argv = argv;

  TRACE_EXIT();
}

GUI::~GUI()
{
  TRACE_ENTER("GUI:~GUI");

  assert(instance);
  instance = nullptr;

  ungrab();

  core.reset();
  delete main_window;

  delete applet_control;

  Backend::core.reset();

  TRACE_EXIT();
}

void
GUI::restbreak_now()
{
  core->force_break(BREAK_ID_REST_BREAK, BreakHint::UserInitiated);
}

void
GUI::main()
{
  TRACE_ENTER("GUI::main");

#ifdef PLATFORM_OS_UNIX
  XInitThreads();
#endif

  app = Gtk::Application::create(argc, argv, "org.workrave.WorkraveApplication");
  app->hold();

  init_core();

  menu_model = std::make_shared<MenuModel>();
  menus = std::make_shared<Menus>(shared_from_this(), toolkit, core, menu_model);

  // TODO: move
#ifdef HAVE_INDICATOR
  indicator_menu = std::make_shared<IndicatorAppletMenu>(menu_model);
#endif

  init_nls();
  init_debug();
  init_sound_player();

  toolkit->init(menu_model, sound_theme);

  init_multihead();
  init_dbus();
  init_platform();
  init_session();
  init_gui();
  init_operation_mode_warning();

#ifdef HAVE_GTK_MAC_INTEGRATION
  GtkosxApplication *theApp = (GtkosxApplication *)g_object_new(GTKOSX_TYPE_APPLICATION, NULL);
  gtkosx_application_set_dock_icon_pixbuf(theApp, gdk_pixbuf_new_from_file(WORKRAVE_PKGDATADIR "/images/workrave.png", NULL));
  gtkosx_application_ready(theApp);
#endif

  on_timer();

  app->run();
  TRACE_MSG("loop ended");

  System::clear();

#if defined(PLATFORM_OS_WINDOWS)
  cleanup_session();
#endif

  for (auto &connection: event_connections)
    {
      connection.disconnect();
    }

  delete main_window;
  main_window = nullptr;

  delete applet_control;
  applet_control = nullptr;

  TRACE_EXIT();
}

//! Terminates the GUI.
void
GUI::terminate()
{
  TRACE_ENTER("GUI::terminate");

  // HACK: Without it status icon keeps on dangling in tray
  // Nicer solution: nicely cleanup complete gui ~GUI()
  delete status_icon;
  status_icon = nullptr;

  Backend::get_configurator()->save();

  app->release();

  TRACE_EXIT();
}

//! Opens the main window.
void
GUI::open_main_window()
{
  main_window->open_window();
}

//! Closes the main window.
void
GUI::close_main_window()
{
  main_window->close_window();
}

//! The user close the main window.
void
GUI::on_main_window_closed()
{
  TRACE_ENTER("GUI::on_main_window_closed");
  bool closewarn = GUIConfig::closewarn_enabled()();
  TRACE_MSG(closewarn);
  if (closewarn && !closewarn_shown)
    {
      status_icon->show_balloon("closewarn",
                                _("Workrave is still running. "
                                  "You can access Workrave by clicking on the white sheep icon. "
                                  "Click on this balloon to disable this message"));
      closewarn_shown = true;
    }

  TRACE_EXIT();
}

//! Periodic heartbeat.
bool
GUI::on_timer()
{
  std::string tip = get_timers_tooltip();

  core->heartbeat();
  main_window->update();

  applet_control->heartbeat();
  applet_control->set_tooltip(tip);
  status_icon->set_tooltip(tip);

  heartbeat_signal();

  if (!break_windows.empty() && muted)
    {
      bool user_active = core->is_user_active();

      if (user_active)
        {
          sound_theme->restore_mute();
          muted = false;
        }
    }

  return true;
}

#if defined(NDEBUG)
static void
my_log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
}
#endif

void
GUI::init_platform()
{
  TRACE_ENTER("GUI::init_platform");
#if defined(PLATFORM_OS_MACOS)
  [[AppController alloc] init];
#endif

  System::init();

  srand((unsigned int)time(nullptr));
  TRACE_EXIT();
}

#if defined(PLATFORM_OS_WINDOWS)
void
GUI::session_quit_cb(EggSMClient *client, GUI *gui)
{
  (void)client;
  (void)gui;

  TRACE_ENTER("GUI::session_quit_cb");

  Backend::get_configurator()->save();
  Gtk::Main::quit();

  TRACE_EXIT();
}

void
GUI::session_save_state_cb(EggSMClient *client, GKeyFile *key_file, GUI *gui)
{
  (void)client;
  (void)key_file;
  (void)gui;

  Backend::get_configurator()->save();
}

void
GUI::cleanup_session()
{
  EggSMClient *client = NULL;

  client = egg_sm_client_get();
  if (client)
    {
      g_signal_handlers_disconnect_by_func(client, (gpointer)G_CALLBACK(session_quit_cb), this);
      g_signal_handlers_disconnect_by_func(client, (gpointer)G_CALLBACK(session_save_state_cb), this);
    }
}
#endif

void
GUI::init_session()
{
  TRACE_ENTER("GUI::init_session");

#if defined(PLATFORM_OS_WINDOWS)
  EggSMClient *client = NULL;
  client = egg_sm_client_get();
  if (client)
    {
      g_signal_connect(client, "quit", G_CALLBACK(session_quit_cb), this);
      g_signal_connect(client, "save-state", G_CALLBACK(session_save_state_cb), this);
    }
#endif

  session = new Session();
  session->init();

  TRACE_EXIT();
}

//! Initializes messages hooks.
void
GUI::init_debug()
{
#if defined(NDEBUG)
  TRACE_ENTER("GUI::init_debug");
  const char *domains[] = {NULL, "Gtk", "GLib", "Gdk", "gtkmm", "GLib-GObject"};
  for (unsigned int i = 0; i < sizeof(domains) / sizeof(char *); i++)
    {
      g_log_set_handler(domains[i], (GLogLevelFlags)(G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION), my_log_handler, NULL);
    }
  TRACE_EXIT();
#endif
}

//! Initializes i18n.
void
GUI::init_nls()
{
#if defined(ENABLE_NLS)
  string language = GUIConfig::locale()();
  if (language != "")
    {
      g_setenv("LANGUAGE", language.c_str(), 1);
    }

  const char *locale_dir;

#  if defined(PLATFORM_OS_WINDOWS)
  string dir = Platform::get_application_directory();
  // Use the pre-install locale location if workrave is running from its MSVC build directory.
  // TODO:
  // dir += Util::file_exists( dir + "\\..\\Workrave.sln" ) ? "\\..\\ui" : "\\lib\\locale";
  dir += "\\lib\\locale";
  locale_dir = dir.c_str();
#  elif defined(PLATFORM_OS_MACOS)
  char locale_path[MAXPATHLEN * 4];
  char execpath[MAXPATHLEN + 1];
  uint32_t pathsz = sizeof(execpath);

  _NSetExecutablePath(execpath, &pathsz);

  gchar *dir_path = g_path_get_dirname(execpath);
  strcpy(locale_path, dir_path);
  g_free(dir_path);

  // Locale
  strcat(locale_path, "/../Resources/locale");
  locale_dir = locale_path;
#  else
  locale_dir = GNOMELOCALEDIR;
#  endif

#  ifdef HAVE_SETLOCALE
  setlocale(LC_ALL, "");
#  endif

#  if defined(PLATFORM_OS_WINDOWS)
  bindtextdomain("gtk20", locale_dir);
  bindtextdomain("iso_3166", locale_dir);
  bindtextdomain("iso_639", locale_dir);
  bindtextdomain("glib20", locale_dir);
  bind_textdomain_codeset("gk20", "UTF-8");
  bind_textdomain_codeset("glib20", "UTF-8");
  bind_textdomain_codeset("iso_3166", "UTF-8");
  bind_textdomain_codeset("iso_639", "UTF-8");

  GUIConfig::locale().connect(this, [&](const std::string &locale) {
    Locale::set_locale(locale);
    // TODO: menus->locale_changed();
  });
#  endif

  bindtextdomain(GETTEXT_PACKAGE, locale_dir);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

#endif
}

//! Initializes the core.
void
GUI::init_core()
{
  const char *display_name = nullptr;

#if defined(PLATFORM_OS_UNIX)
  display_name = gdk_display_get_name(gdk_display_get_default());
#endif

  core = Backend::get_core();
  core->init(argc, argv, this, display_name);
  core->set_core_events_listener(this);

  GUIConfig::init();
}

void
GUI::init_multihead()
{
  TRACE_ENTER("GUI::init_multihead");
  Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
  Glib::RefPtr<Gdk::Screen> screen = display->get_default_screen();
  screen->signal_monitors_changed().connect(sigc::mem_fun(*this, &GUI::update_multihead));

  update_multihead();
  TRACE_EXIT();
}

void
GUI::update_multihead()
{
  TRACE_ENTER("GUI::update_multihead");
  // TODO
  TRACE_EXIT();
}

//! Initializes the GUI
void
GUI::init_gui()
{
#ifdef PLATFORM_OS_WINDOWS
  // No auto hide scrollbars
  g_setenv("GTK_OVERLAY_SCROLLING", "0", TRUE);
  // No Windows-7 style client-side decorations on Windows 10...
  g_setenv("GTK_CSD", "0", TRUE);
  g_setenv("GDK_WIN32_DISABLE_HIDPI", "1", TRUE);

  static const char css[] =
    R"(
       window decoration, tooltip decoration {
         all: unset;
       }
      )";

  auto provider = Gtk::CssProvider::create();
  provider->load_from_data(css);
  auto screen = Gdk::Screen::get_default();
  Gtk::StyleContext::add_provider_for_screen(screen, provider, GTK_STYLE_PROVIDER_PRIORITY_USER + 100);
#endif

  // The main status window.
  main_window = new MainWindow(shared_from_this(), menu_model);
  main_window->init();

  // The applet window.
  applet_control = new AppletControl(shared_from_this());
  applet_control->init(menu_model);

  // Status Icon
  auto status_icon_menu = std::make_shared<ToolkitMenu>(menu_model);
  status_icon_menu->get_menu()->attach_to_widget(*main_window);

  status_icon = new StatusIcon(status_icon_menu);
  status_icon->init();

  // Events
  event_connections.emplace_back(main_window->signal_closed().connect(sigc::mem_fun(*this, &GUI::on_main_window_closed)));
  event_connections.emplace_back(main_window->signal_visibility_changed().connect(sigc::mem_fun(*this, &GUI::on_visibility_changed)));
  event_connections.emplace_back(applet_control->signal_visibility_changed().connect(sigc::mem_fun(*this, &GUI::on_visibility_changed)));
  event_connections.emplace_back(
    status_icon->signal_balloon_activate().connect(sigc::mem_fun(*this, &GUI::on_status_icon_balloon_activate)));
  event_connections.emplace_back(status_icon->signal_activate().connect(sigc::mem_fun(*this, &GUI::on_status_icon_activate)));
  event_connections.emplace_back(status_icon->signal_visibility_changed().connect(sigc::mem_fun(*this, &GUI::on_visibility_changed)));

  process_visibility();

  workrave::dbus::IDBus::Ptr dbus = Backend::get_dbus();

  if (dbus->is_available())
    {
      dbus->connect("/org/workrave/Workrave/UI", "org.workrave.ControlInterface", menus.get());
    }

#if defined(PLATFORM_OS_WINDOWS)
  win32_init_filter();
#endif

  // Periodic timer.
  Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUI::on_timer), 1000);

#ifdef IS_THIS_NEEDED_FOR_GTK3
  static const gchar *rc_string = {
    "style \"progressBarWidth\"\n"
    "{\n"
    "   GtkProgressBar::min-horizontal-bar-width = 10\n"
    "   GtkProgressBar::min-horizontal-bar-height = 2\n"
    "}\n"
    "\n"
    "widget \"*.locked-progress\" style \"progressBarWidth\"\n"
    // "class \"GtkProgressBar\" style \"progressBarWidth\"\n"
  };

  gtk_rc_parse_string(rc_string);
#endif
}

void
GUI::init_dbus()
{
  workrave::dbus::IDBus::Ptr dbus = Backend::get_dbus();

  if (dbus->is_available())
    {
      if (dbus->is_running("org.workrave.Workrave"))
        {
          Gtk::MessageDialog dialog(_("Workrave failed to start"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
          dialog.set_secondary_text(_("Is Workrave already running?"));
          dialog.show();
          dialog.run();
          exit(1);
        }
      try
        {
          dbus->register_object_path("/org/workrave/Workrave/UI");
          dbus->register_service("org.workrave.Workrave", this);
        }
      catch (workrave::dbus::DBusException &)
        {
        }
    }

#ifdef HAVE_DBUS
  try
    {
      extern void init_DBusGUI(workrave::dbus::IDBus::Ptr dbus);
      init_DBusGUI(dbus);
    }
  catch (workrave::dbus::DBusException &)
    {
    }
#endif
}

void
GUI::bus_name_presence(const std::string &name, bool present)
{
  if (name == "org.workrave.Workrave" && !present)
    {
      // Silent exit
      exit(1);
    }
}

void
GUI::init_operation_mode_warning()
{
  OperationMode mode = core->get_operation_mode();
  if (mode != OperationMode::Normal)
    {
      Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUI::on_operational_mode_warning_timer), 5000);
    }
}

//! Initializes the sound player.
void
GUI::init_sound_player()
{
  TRACE_ENTER("GUI:init_sound_player");
  try
    {
      // Tell pulseaudio were are playing sound events
      g_setenv("PULSE_PROP_media.role", "event", TRUE);

      sound_theme = std::make_shared<SoundTheme>();
      sound_theme->init();
    }
  catch (workrave::utils::Exception &)
    {
      TRACE_MSG("No sound");
    }
  TRACE_EXIT();
}

void
GUI::core_event_notify(const CoreEvent event)
{
  TRACE_ENTER_MSG("GUI::core_event_sound_notify", event);

  if (event >= CORE_EVENT_SOUND_FIRST && event <= CORE_EVENT_SOUND_LAST)
    {
      bool mute = false;
      SoundEvent snd = (SoundEvent)((int)event - CORE_EVENT_SOUND_FIRST);
      TRACE_MSG("play " << event);

      if (event == CORE_EVENT_SOUND_REST_BREAK_STARTED || event == CORE_EVENT_SOUND_DAILY_LIMIT)
        {
          bool mute = SoundTheme::sound_mute()();
          if (mute)
            {
              muted = true;
            }
        }
      TRACE_MSG("Mute after playback " << mute);
      sound_theme->play_sound(snd, mute);
    }

  if (event == CORE_EVENT_MONITOR_FAILURE)
    {
      string msg = _("Workrave could not monitor your keyboard and mouse activity.\n");

#ifdef PLATFORM_OS_UNIX
      msg += _("Make sure that the RECORD extension is enabled in the X server.");
#endif
      Gtk::MessageDialog dialog(_("Workrave failed to start"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
      dialog.set_secondary_text(msg);
      dialog.show();
      dialog.run();
      terminate();
    }
  TRACE_EXIT();
}

void
GUI::core_event_operation_mode_changed(const OperationMode m)
{
  if (status_icon)
    {
      status_icon->set_operation_mode(m);
    }
}

void
GUI::core_event_usage_mode_changed(const UsageMode m)
{
}

void
GUI::set_break_response(IBreakResponse *rep)
{
  response = rep;
}

void
GUI::create_prelude_window(BreakId break_id)
{
  TRACE_ENTER_MSG("GUI::create_prelude_window", break_id);
  hide_break_window();
  active_break_id = break_id;

  for (int i = 0; i < get_head_count(); i++)
    {
      prelude_windows.push_back(std::make_shared<PreludeWindow>(get_head_info(i), break_id));
    }

  TRACE_EXIT();
}

void
GUI::create_break_window(BreakId break_id, workrave::utils::Flags<BreakHint> break_hint)
{
  TRACE_ENTER_MSG("GUI::create_break_window", break_id << " " << break_hint);
  hide_break_window();

  BreakFlags break_flags = BREAK_FLAGS_NONE;
  bool ignorable = GUIConfig::break_ignorable(break_id)();
  bool skippable = GUIConfig::break_skippable(break_id)();

  if (break_hint & BreakHint::UserInitiated)
    {
      break_flags = (BREAK_FLAGS_POSTPONABLE | BREAK_FLAGS_USER_INITIATED);

      if (skippable)
        {
          break_flags |= BREAK_FLAGS_SKIPPABLE;
        }
    }
  else
    {
      if (ignorable)
        {
          break_flags |= BREAK_FLAGS_POSTPONABLE;
        }

      if (skippable)
        {
          break_flags |= BREAK_FLAGS_SKIPPABLE;
        }
    }

  if (break_hint & BreakHint::NaturalBreak)
    {
      break_flags |= (BREAK_FLAGS_NO_EXERCISES | BREAK_FLAGS_NATURAL | BREAK_FLAGS_POSTPONABLE);
    }

  active_break_id = break_id;

  for (int i = 0; i < get_head_count(); i++)
    {
      IBreakWindow::Ptr break_window = toolkit->create_break_window(i, break_id, break_flags);

      break_windows.push_back(break_window);

      break_window->set_response(response);
      break_window->init();
    }

  TRACE_EXIT();
}

void
GUI::hide_break_window()
{
  TRACE_ENTER("GUI::hide_break_window");
  active_break_id = BREAK_ID_NONE;

  for (auto &window: prelude_windows)
    {
      window->stop();
    }

  for (auto &window: break_windows)
    {
      window->stop();
    }

  break_windows.clear();
  prelude_windows.clear();

  ungrab();
  TRACE_EXIT();
}

void
GUI::show_break_window()
{
  TRACE_ENTER("GUI::hide_break_window");

  for (auto &window: prelude_windows)
    {
      window->start();
    }

  for (auto &window: break_windows)
    {
      window->start();
    }

  if (GUIConfig::block_mode()() != GUIConfig::BLOCK_MODE_NONE)
    {
      grab();
    }

  TRACE_EXIT();
}

void
GUI::refresh_break_window()
{
  for (auto &window: prelude_windows)
    {
      window->refresh();
    }

  for (auto &window: break_windows)
    {
      window->refresh();
    }
}

void
GUI::set_break_progress(int value, int max_value)
{
  for (auto &window: prelude_windows)
    {
      window->set_progress(value, max_value);
    }

  for (auto &window: break_windows)
    {
      window->set_progress(value, max_value);
    }
}

void
GUI::set_prelude_stage(PreludeStage stage)
{
  for (auto &window: prelude_windows)
    {
      window->set_stage(stage);
    }
}

void
GUI::set_prelude_progress_text(PreludeProgressText text)
{
  for (auto &window: prelude_windows)
    {
      window->set_progress_text(text);
    }
}

void
GUI::grab()
{
  if (!break_windows.empty())
    {
#if defined(PLATFORM_OS_WINDOWS)
      // TODO: check if this is still needed with Gtk3
      for (auto &window: break_windows)
        {
          HWND w = (HWND)GDK_WINDOW_HWND(window->get_gdk_window()->gobj());
          SetWindowPos(w, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
          BringWindowToTop(w);
        }
#endif

      Glib::RefPtr<Gdk::Window> window = break_windows[0]->get_gdk_window();
      Grab::instance()->grab(window->gobj());
    }
}

void
GUI::ungrab()
{
  Grab::instance()->ungrab();
}

void
GUI::interrupt_grab()
{
  ungrab();
  grab();
}

bool
GUI::on_operational_mode_warning_timer()
{
  OperationMode mode = core->get_operation_mode();
  if (mode == OperationMode::Suspended)
    {
      status_icon->show_balloon("operation_mode",
                                _("Workrave is in suspended mode. "
                                  "Mouse and keyboard activity will not be monitored."));
    }
  else if (mode == OperationMode::Quiet)
    {
      status_icon->show_balloon("operation_mode",
                                _("Workrave is in quiet mode. "
                                  "No break windows will appear."));
    }
  return false;
}

int
GUI::map_to_head(int &x, int &y)
{
  int ret = -1;

  for (int i = 0; i < get_head_count(); i++)
    {
      int left, top, width, height;

      HeadInfo head = get_head_info(i);

      left = head.get_x();
      top = head.get_y();
      width = head.get_width();
      height = head.get_height();

      if (x >= left && y >= top && x < left + width && y < top + height)
        {
          x -= left;
          y -= top;

          // Use coordinates relative to right and butto edges of the
          // screen if the mainwindow is closer to those edges than to
          // the left/top edges.

          if (x >= width / 2)
            {
              x -= width;
            }
          if (y >= height / 2)
            {
              y -= height;
            }
          ret = i;
          break;
        }
    }

  if (ret < 0)
    {
      ret = 0;
      x = y = 256;
    }
  return ret;
}

void
GUI::map_from_head(int &x, int &y, int head)
{
  HeadInfo h = get_head_info(head);
  if (x < 0)
    {
      x += h.get_width();
    }
  if (y < 0)
    {
      y += h.get_height();
    }

  x += h.get_x();
  y += h.get_y();
}

bool
GUI::bound_head(int &x, int &y, int width, int height, int &head)
{
  bool ret = false;

  if (head >= get_head_count())
    {
      head = 0;
    }

  HeadInfo h = get_head_info(head);
  if (x < -h.get_width())
    {
      x = 0;
      ret = true;
    }
  if (y < -h.get_height())
    {
      y = 0;
      ret = true;
    }

  // Make sure something remains visible..
  if (x > -10 && x < 0)
    {
      x = -10;
      ret = true;
    }
  if (y > -10 && y < 0)
    {
      y = -10;
      ret = true;
    }

  if (x + width >= h.get_width())
    {
      x = h.get_width() - width - 10;
      ret = true;
    }

  if (y + height >= h.get_height())
    {
      y = h.get_height() - height - 10;
      ret = true;
    }

  return ret;
}

std::string
GUI::get_timers_tooltip()
{
  // FIXME: duplicate
  const char *labels[] = {_("Micro-break"), _("Rest break"), _("Daily limit")};
  string tip = "";

  OperationMode mode = core->get_operation_mode();
  switch (mode)
    {
    case OperationMode::Suspended:
      tip = string(_("Mode: ")) + _("Suspended");
      break;

    case OperationMode::Quiet:
      tip = string(_("Mode: ")) + _("Quiet");
      break;

    case OperationMode::Normal:
    default:
#if !defined(PLATFORM_OS_WINDOWS)
      // Win32 tip is limited in length
      tip = "Workrave";
#endif
      break;
    }

  for (int count = 0; count < BREAK_ID_SIZEOF; count++)
    {
      IBreak *b = core->get_break(BreakId(count));
      bool on = b->is_enabled();

      if (b != nullptr && on)
        {
          // Collect some data.
          int64_t maxActiveTime = b->get_limit();
          int64_t activeTime = b->get_elapsed_time();
          std::string text;

          // Set the text
          if (b->is_limit_enabled() && maxActiveTime != 0)
            {
              text = Text::time_to_string(maxActiveTime - activeTime);
            }
          else
            {
              text = Text::time_to_string(activeTime);
            }

          if (tip != "")
            {
              tip += "\n";
            }

          tip += labels[count];
          tip += ": " + text;
        }
    }

  return tip;
}

void
GUI::on_status_icon_balloon_activate(const std::string &id)
{
  if (id == "closewarn")
    {
      GUIConfig::closewarn_enabled().set(false);
    }
}

void
GUI::on_status_icon_activate()
{
  main_window->toggle_window();
}

void
GUI::on_visibility_changed()
{
  TRACE_ENTER("GUI::on_visibility_changed");
  process_visibility();
  TRACE_EXIT();
}

void
GUI::process_visibility()
{
  TRACE_ENTER("GUI::process_visibility");
  TRACE_MSG(main_window->is_visible() << " " << applet_control->is_visible() << " " << status_icon->is_visible());
#ifdef PLATFORM_OS_WINDOWS
  if (!main_window->is_visible() && !applet_control->is_visible())
    {
      GUIConfig::trayicon_enabled().set(true);
    }
#else
  bool can_close_main_window = applet_control->is_visible() || status_icon->is_visible();
  main_window->set_can_close(can_close_main_window);
#endif
  TRACE_EXIT();
}

#if defined(PLATFORM_OS_WINDOWS)
#  ifndef GUID_DEVINTERFACE_MONITOR
static GUID GUID_DEVINTERFACE_MONITOR = {0xe6f07b5f, 0xee97, 0x4a90, {0xb0, 0x76, 0x33, 0xf5, 0x7b, 0xf4, 0xea, 0xa7}};
#  endif
void
GUI::win32_init_filter()
{
  GtkWidget *window = (GtkWidget *)main_window->gobj();
  GdkWindow *gdk_window = gtk_widget_get_window(window);
  gdk_window_add_filter(gdk_window, win32_filter_func, this);

  HWND hwnd = (HWND)GDK_WINDOW_HWND(gdk_window);

  WTSRegisterSessionNotification(hwnd, NOTIFY_FOR_THIS_SESSION);

  DEV_BROADCAST_DEVICEINTERFACE notification;
  ZeroMemory(&notification, sizeof(notification));
  notification.dbcc_size = sizeof(notification);
  notification.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
  notification.dbcc_classguid = GUID_DEVINTERFACE_MONITOR;
  RegisterDeviceNotification(hwnd, &notification, DEVICE_NOTIFY_WINDOW_HANDLE);
}

GdkFilterReturn
GUI::win32_filter_func(void *xevent, GdkEvent *event, gpointer data)
{
  TRACE_ENTER("GUI::win32_filter_func");
  (void)event;
  GUI *gui = static_cast<GUI *>(data);

  MSG *msg = (MSG *)xevent;
  GdkFilterReturn ret = GDK_FILTER_CONTINUE;
  switch (msg->message)
    {
    case WM_WTSSESSION_CHANGE:
      {
        if (msg->wParam == WTS_SESSION_LOCK)
          {
            gui->session->set_idle(true);
          }
        if (msg->wParam == WTS_SESSION_UNLOCK)
          {
            gui->session->set_idle(false);
            gui->init_operation_mode_warning();
          }
      }
      break;

    case WM_POWERBROADCAST:
      {
        TRACE_MSG("WM_POWERBROADCAST " << msg->wParam << " " << msg->lParam);

        switch (msg->wParam)
          {
          case PBT_APMQUERYSUSPEND:
            TRACE_MSG("Query Suspend");
            break;

          case PBT_APMQUERYSUSPENDFAILED:
            TRACE_MSG("Query Suspend Failed");
            break;

          case PBT_APMRESUMESUSPEND:
          case PBT_APMRESUMEAUTOMATIC:
          case PBT_APMRESUMECRITICAL:
            {
              TRACE_MSG("Resume suspend");
              auto core = Backend::get_core();
              core->set_powersave(false);
            }
            break;

          case PBT_APMSUSPEND:
            {
              TRACE_MSG("Suspend");
              auto core = Backend::get_core();
              core->set_powersave(true);
            }
            break;
          }
      }
      break;

    case WM_DISPLAYCHANGE:
      {
        TRACE_MSG("WM_DISPLAYCHANGE " << msg->wParam << " " << msg->lParam);
      }
      break;

    case WM_TIMECHANGE:
      {
        TRACE_MSG("WM_TIMECHANGE " << msg->wParam << " " << msg->lParam);
        auto core = Backend::get_core();
        core->time_changed();
      }
      break;

    case WM_DEVICECHANGE:
      {
        TRACE_MSG("WM_DEVICECHANGE " << msg->wParam << " " << msg->lParam);
        switch (msg->wParam)
          {
          case DBT_DEVICEARRIVAL:
          case DBT_DEVICEREMOVECOMPLETE:
            {
              HWND hwnd = FindWindowExA(NULL, NULL, "GdkDisplayChange", NULL);
              if (hwnd)
                {
                  SendMessage(hwnd, WM_DISPLAYCHANGE, 0, 0);
                }
            }
          default:
            break;
          }
        break;
      }

    default:
      std::shared_ptr<W32AppletWindow> applet_window =
        std::dynamic_pointer_cast<W32AppletWindow>(gui->applet_control->get_applet_window(AppletControl::AppletType::Windows));
      if (applet_window)
        {
          ret = applet_window->win32_filter_func(xevent, event);
        }
    }

#  ifndef USE_W32STATUSICON
  if (ret != GDK_FILTER_REMOVE && gui->status_icon)
    {
      ret = gui->status_icon->win32_filter_func(xevent, event);
    }
#  endif

  TRACE_EXIT();
  return ret;
}

#endif
