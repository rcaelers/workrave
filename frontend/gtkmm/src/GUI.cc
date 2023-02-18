// GUI.cc --- The WorkRave GUI
//
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

#include "preinclude.h"
#include "nls.h"
#include "debug.hh"

#include <iostream>
#include <gtkmm.h>

#include <glibmm.h>
#include <gtk/gtk.h>

#ifdef PLATFORM_OS_WINDOWS_NATIVE
#  undef HAVE_UNISTD_H
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>

#include "GUI.hh"

// Backend includes.
#include "IBreak.hh"
#include "IBreakResponse.hh"
#include "IBreakWindow.hh"
#include "IConfigurator.hh"
#include "ICore.hh"
#include "CoreFactory.hh"

#include "Exception.hh"
#include "AppletControl.hh"
#include "AppletWindow.hh"
#include "BreakWindow.hh"
#include "DailyLimitWindow.hh"
#include "GUIConfig.hh"
#include "MainWindow.hh"
#include "Menus.hh"
#include "MicroBreakWindow.hh"
#include "PreludeWindow.hh"
#include "RestBreakWindow.hh"
#include "SoundPlayer.hh"
#include "StatusIcon.hh"
#include "System.hh"
#include "Text.hh"
#include "Util.hh"
#include "WindowHints.hh"
#include "Locale.hh"
#include "Session.hh"
#include "TimerBoxControl.hh"

#if defined(PLATFORM_OS_WINDOWS)
#  include "W32AppletWindow.hh"
#  include <gdk/gdkwin32.h>
#  include <pbt.h>
#  include <wtsapi32.h>
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

#if defined(HAVE_GCONF)
#  include <gconf/gconf-client.h>
#endif

#if defined(HAVE_DBUS)
#  if defined(interface)
#    undef interface
#  endif
#  include "dbus/IDBus.hh"
#  include "dbus/DBusException.hh"
#endif

#ifdef HAVE_GTK_MAC_INTEGRATION
#  include "gtkosxapplication.h"
#endif

GUI *GUI::instance = NULL;

//! GUI Constructor.
/*!
 *  \param argc number of command line parameters.
 *  \param argv all command line parameters.
 */
GUI::GUI(int argc, char **argv)
  : core(NULL)
  , sound_player(NULL)
  , break_windows(NULL)
  , prelude_windows(NULL)
  , active_break_count(0)
  , active_prelude_count(0)
  , response(NULL)
  , active_break_id(BREAK_ID_NONE)
  , main_window(NULL)
  , menus(0)
  , break_window_destroy(false)
  , prelude_window_destroy(false)
  , heads(NULL)
  , num_heads(-1)
  , screen_width(-1)
  , screen_height(-1)
  ,
#if defined(PLATFORM_OS_UNIX)
  grab_wanted(false)
  ,
#endif
  grab_handle(NULL)
  , status_icon(NULL)
  , applet_control(NULL)
  , muted(false)
  , closewarn_shown(false)
{
  TRACE_ENTER("GUI:GUI");

  assert(!instance);
  instance = this;

  this->argc = argc;
  this->argv = argv;

  TRACE_EXIT();
}

//! Destructor.
GUI::~GUI()
{
  TRACE_ENTER("GUI:~GUI");

  assert(instance);
  instance = NULL;

  ungrab();

  delete core;
  delete main_window;

  delete applet_control;
  delete menus;

  delete[] prelude_windows;
  delete[] break_windows;
  delete[] heads;

  delete sound_player;

  TRACE_EXIT();
}

//! Forces a restbreak.
void
GUI::restbreak_now()
{
  core->force_break(BREAK_ID_REST_BREAK, BREAK_HINT_USER_INITIATED);
}

//! The main entry point.
void
GUI::main()
{
  TRACE_ENTER("GUI::main");

  init_core();

#if defined(PLATFORM_OS_UNIX)
  if (GUIConfig::get_force_x11_enabled())
    {
      g_setenv("GDK_BACKEND", "x11", TRUE);
    }
#endif

#ifdef PLATFORM_OS_UNIX
  XInitThreads();
#endif

#ifdef HAVE_GTK3
  app = Gtk::Application::create(argc, argv, "org.workrave.WorkraveApplication");
  app->hold();
#else
#  if defined(PLATFORM_OS_WINDOWS)
  Glib::OptionContext option_ctx;
  Glib::OptionGroup *option_group = new Glib::OptionGroup(egg_sm_client_get_option_group());
  option_ctx.add_group(*option_group);
#  endif

  Gtk::Main *kit = NULL;
  try
    {
#  if defined(PLATFORM_OS_WINDOWS)
      kit = new Gtk::Main(argc, argv, option_ctx);
#  else
      kit = new Gtk::Main(argc, argv);
#  endif
    }
  catch (const Glib::OptionError &e)
    {
      std::cout << "Failed to initialize: " << e.what() << std::endl;
      exit(1);
    }
#endif

  init_nls();
  init_debug();
  init_sound_player();
  init_multihead();
#ifdef HAVE_DBUS
  init_dbus();
#endif
  init_platform();
  init_session();
  init_gui();
  init_operation_mode_warning();

#ifdef HAVE_GTK_MAC_INTEGRATION
  GtkosxApplication *theApp = (GtkosxApplication *)g_object_new(GTKMacOS_TYPE_APPLICATION, NULL);
  gtkosx_application_set_dock_icon_pixbuf(theApp, gdk_pixbuf_new_from_file(WORKRAVE_PKGDATADIR "/images/workrave.png", NULL));
  gtkosx_application_ready(theApp);
#endif

  on_timer();

#ifdef HAVE_GTK3
  app->run();
#else
  Gtk::Main::run();
#endif
  TRACE_MSG("loop ended");

  System::clear();
#if defined(PLATFORM_OS_WINDOWS)
  cleanup_session();
#endif

  for (list<sigc::connection>::iterator i = event_connections.begin(); i != event_connections.end(); i++)
    {
      i->disconnect();
    }

  delete main_window;
  main_window = NULL;

  delete applet_control;
  applet_control = NULL;

#if !defined(HAVE_GTK3)
  delete kit;
#endif

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
  status_icon = 0;

  CoreFactory::get_configurator()->save();

  collect_garbage();

#ifdef HAVE_GTK3
  app->release();
#else
  Gtk::Main::quit();
#endif

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
  bool closewarn = false;
  CoreFactory::get_configurator()->get_value(GUIConfig::CFG_KEY_CLOSEWARN_ENABLED, closewarn);
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

  collect_garbage();

  if (active_break_count == 0 && muted)
    {
      bool user_active = core->is_user_active();

      if (user_active)
        {
          sound_player->restore_mute();
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

#if defined(PLATFORM_OS_UNIX)
  const char *display = gdk_display_get_name(gdk_display_get_default());
  System::init(display);
#else
  System::init();
#endif

  srand((unsigned int)time(NULL));
  TRACE_EXIT();
}

#if defined(PLATFORM_OS_WINDOWS)
void
GUI::session_quit_cb(EggSMClient *client, GUI *gui)
{
  (void)client;
  (void)gui;

  TRACE_ENTER("GUI::session_quit_cb");

  CoreFactory::get_configurator()->save();
  Gtk::Main::quit();

  TRACE_EXIT();
}

void
GUI::session_save_state_cb(EggSMClient *client, GKeyFile *key_file, GUI *gui)
{
  (void)client;
  (void)key_file;
  (void)gui;

  CoreFactory::get_configurator()->save();
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
      g_log_set_handler(
        domains[i], (GLogLevelFlags)(G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION), my_log_handler, NULL);
    }
  TRACE_EXIT();
#endif
}

//! Initializes i18n.
void
GUI::init_nls()
{
#if defined(ENABLE_NLS)
  string language = GUIConfig::get_locale();
  if (language != "")
    {
      g_setenv("LANGUAGE", language.c_str(), 1);
    }

#  if !defined(HAVE_GTK3)
  gtk_set_locale();
#  endif
  const char *locale_dir;

#  if defined(PLATFORM_OS_WINDOWS)
  string dir = Util::get_application_directory();
  // Use the pre-install locale location if workrave is running from its MSVC build directory.
  dir += Util::file_exists(dir + "\\..\\Workrave.sln") ? "\\..\\frontend" : "\\lib\\locale";
  locale_dir = dir.c_str();
#  elif defined(PLATFORM_OS_MACOS)
  char locale_path[MAXPATHLEN * 4];
  char execpath[MAXPATHLEN + 1];
  uint32_t pathsz = sizeof(execpath);

  _NSGetExecutablePath(execpath, &pathsz);

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

  CoreFactory::get_configurator()->add_listener(GUIConfig::CFG_KEY_LOCALE, this);
#  endif

  bindtextdomain(PACKAGE, locale_dir);
  bind_textdomain_codeset(PACKAGE, "UTF-8");
  textdomain(PACKAGE);

#endif
}

//! Initializes the core.
void
GUI::init_core()
{
  const char *display_name = NULL;

#if defined(PLATFORM_OS_UNIX)
  display_name = gdk_display_get_name(gdk_display_get_default());
#endif

  core = CoreFactory::get_core();
  core->init(argc, argv, this, display_name);
  core->set_core_events_listener(this);

  GUIConfig::init();
}

void
GUI::init_multihead()
{
  TRACE_ENTER("GUI::init_multihead");

  init_gtk_multihead();
  init_multihead_desktop();
  TRACE_EXIT();
}

void
GUI::init_multihead_mem(int new_num_heads)
{
  TRACE_ENTER("GUI::init_multihead_mem");
  if (new_num_heads != num_heads || num_heads <= 0)
    {
      delete[] heads;
      heads = new HeadInfo[new_num_heads];

      PreludeWindow **old_prelude_windows = prelude_windows;
      IBreakWindow **old_break_windows = break_windows;

      prelude_windows = new PreludeWindow *[new_num_heads]; /* LEAK */
      break_windows = new IBreakWindow *[new_num_heads];    /* LEAK */

      int max_heads = new_num_heads > num_heads ? new_num_heads : num_heads;

      // Copy existing breaks windows.
      for (int i = 0; i < max_heads; i++)
        {
          if (i < new_num_heads)
            {
              if (i < num_heads)
                {
                  prelude_windows[i] = old_prelude_windows[i];
                  break_windows[i] = old_break_windows[i];
                }
              else
                {
                  prelude_windows[i] = NULL;
                  break_windows[i] = NULL;
                }
            }

          if (new_num_heads < num_heads && i >= new_num_heads)
            {
              // Number of heads get smaller,
              // destroy breaks/preludes
              if (old_prelude_windows != NULL && old_prelude_windows[i] != NULL)
                {
                  old_prelude_windows[i]->destroy();
                }
              if (old_break_windows != NULL && old_break_windows[i] != NULL)
                {
                  old_break_windows[i]->destroy();
                }
            }
        }

      if (active_prelude_count > new_num_heads)
        {
          active_prelude_count = new_num_heads;
        }

      if (active_break_count > new_num_heads)
        {
          active_break_count = new_num_heads;
        }

      delete[] old_prelude_windows;
      delete[] old_break_windows;

      num_heads = new_num_heads;
    }
  TRACE_EXIT();
}

void
GUI::init_multihead_desktop()
{
  TRACE_ENTER("GUI::init_multihead_desktop");

  int width = 0;
  int height = 0;

  for (int i = 0; i < num_heads; i++)
    {
      int w = heads[i].geometry.get_x() + heads[i].geometry.get_width();
      int h = heads[i].geometry.get_y() + heads[i].geometry.get_height();

      if (w > width)
        {
          width = w;
        }
      if (h > height)
        {
          height = h;
        }
    }

  TRACE_MSG("width x height " << width << " " << height);
  if (screen_width != width || screen_height != height)
    {
      if (main_window != NULL)
        {
          main_window->relocate_window(width, height);
        }
      screen_width = width;
      screen_height = height;
    }
}

void
GUI::init_gtk_multihead()
{
  TRACE_ENTER("GUI::init_gtk_multihead");

  int new_num_heads = 0;

  Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
  int num_screens = display->get_n_screens();

  TRACE_MSG("screens = " << num_screens);
  if (num_screens >= 1)
    {
      for (int i = 0; i < num_screens; i++)
        {
          Glib::RefPtr<Gdk::Screen> screen = display->get_screen(i);
          if (screen)
            {
              new_num_heads += screen->get_n_monitors();
              TRACE_MSG("num monitors on screen " << i << " = " << screen->get_n_monitors());
            }
        }

      init_multihead_mem(new_num_heads);

      int count = 0;
      for (int i = 0; i < num_screens; i++)
        {
          Glib::RefPtr<Gdk::Screen> screen = display->get_screen(i);
          if (screen)
            {
              int num_monitors = screen->get_n_monitors();
              TRACE_MSG("monitors = " << num_monitors);
              for (int j = 0; j < num_monitors && count < new_num_heads; j++)
                {
                  Gdk::Rectangle rect;
                  screen->get_monitor_geometry(j, rect);

#ifdef HAVE_GTK3
                  gint scale = screen->get_monitor_scale_factor(j);
                  rect =
                    Gdk::Rectangle(rect.get_x() / scale, rect.get_y() / scale, rect.get_width() / scale, rect.get_height() / scale);
#endif
                  bool overlap = false;
                  for (int k = 0; !overlap && k < count; k++)
                    {
                      Gdk::Rectangle irect = rect;

                      if (heads[k].screen->get_number() == i)
                        {
                          irect.intersect(heads[k].geometry, overlap);
                        }
                    }

                  if (!overlap)
                    {
                      heads[count].screen = screen;
                      heads[count].monitor = j;
                      heads[count].count = count;
                      heads[count].geometry = rect;
                      count++;
                    }

                  TRACE_MSG("Screen #" << i << " Monitor #" << j << "  " << rect.get_x() << " " << rect.get_y() << " "
                                       << rect.get_width() << " " << rect.get_height() << " "
                                       << " intersects " << overlap);
                }
            }
        }
      num_heads = count;
      TRACE_MSG("# Heads = " << num_heads);
    }
  TRACE_EXIT();
}

//! Initializes the GUI
void
GUI::init_gui()
{
  menus = new Menus();

  // The main status window.
  main_window = new MainWindow();
  main_window->init();

  // The applet window.
  applet_control = new AppletControl();
  applet_control->init();

  // Menus
  menus->init(applet_control);
  menus->resync();

  // Status Icon
  status_icon = new StatusIcon();
  status_icon->init();

  // Events
  event_connections.push_back(main_window->signal_closed().connect(sigc::mem_fun(*this, &GUI::on_main_window_closed)));
  event_connections.push_back(main_window->signal_visibility_changed().connect(sigc::mem_fun(*this, &GUI::on_visibility_changed)));
  event_connections.push_back(
    applet_control->signal_visibility_changed().connect(sigc::mem_fun(*this, &GUI::on_visibility_changed)));
  event_connections.push_back(
    status_icon->signal_balloon_activate().connect(sigc::mem_fun(*this, &GUI::on_status_icon_balloon_activate)));
  event_connections.push_back(status_icon->signal_activate().connect(sigc::mem_fun(*this, &GUI::on_status_icon_activate)));
  event_connections.push_back(status_icon->signal_visibility_changed().connect(sigc::mem_fun(*this, &GUI::on_visibility_changed)));

  process_visibility();

#ifdef HAVE_DBUS
  workrave::dbus::IDBus::Ptr dbus = CoreFactory::get_dbus();

  if (dbus->is_available())
    {
      dbus->connect("/org/workrave/Workrave/UI", "org.workrave.ControlInterface", menus);
    }
#endif

#if defined(PLATFORM_OS_WINDOWS)
  win32_init_filter();
#endif

  // Periodic timer.
  Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUI::on_timer), 1000);

#ifndef HAVE_GTK3
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

#ifdef HAVE_DBUS
void
GUI::init_dbus()
{
  workrave::dbus::IDBus::Ptr dbus = CoreFactory::get_dbus();

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

          extern void init_DBusGUI(workrave::dbus::IDBus::Ptr dbus);
          init_DBusGUI(dbus);
        }
      catch (workrave::dbus::DBusException &)
        {
        }
    }
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
#endif

void
GUI::init_operation_mode_warning()
{
  OperationMode mode = core->get_operation_mode();
  if (mode != OPERATION_MODE_NORMAL)
    {
      Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUI::on_operational_mode_warning_timer), 5000);
    }
}

//! Returns a break window for the specified break.
IBreakWindow *
GUI::create_break_window(HeadInfo &head, BreakId break_id, BreakWindow::BreakFlags break_flags)
{
  IBreakWindow *ret = NULL;
  GUIConfig::BlockMode block_mode = GUIConfig::get_block_mode();
  if (break_id == BREAK_ID_MICRO_BREAK)
    {
      ret = new MicroBreakWindow(head, break_flags, block_mode);
    }
  else if (break_id == BREAK_ID_REST_BREAK)
    {
      ret = new RestBreakWindow(head, break_flags, block_mode);
    }
  else if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      ret = new DailyLimitWindow(head, break_flags, block_mode);
    }

  return ret;
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

      sound_player = new SoundPlayer(); /* LEAK */
      sound_player->init();
    }
  catch (Exception &)
    {
      TRACE_MSG("No sound");
    }
  TRACE_EXIT();
}

void
GUI::core_event_notify(const CoreEvent event)
{
  TRACE_ENTER_MSG("GUI::core_event_sound_notify", event);

  if (sound_player != NULL)
    {
      if (event >= CORE_EVENT_SOUND_FIRST && event <= CORE_EVENT_SOUND_LAST)
        {
          bool mute = false;
          SoundEvent snd = (SoundEvent)((int)event - CORE_EVENT_SOUND_FIRST);
          TRACE_MSG("play " << event);

          if (event == CORE_EVENT_SOUND_REST_BREAK_STARTED || event == CORE_EVENT_SOUND_DAILY_LIMIT)
            {
              CoreFactory::get_configurator()->get_value(SoundPlayer::CFG_KEY_SOUND_MUTE, mute);
              if (mute)
                {
                  muted = true;
                }
            }
          TRACE_MSG("Mute after playback " << mute);
          sound_player->play_sound(snd, mute);
        }
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

  menus->resync();
}

void
GUI::core_event_usage_mode_changed(const UsageMode m)
{
  (void)m;
  menus->resync();
}

void
GUI::config_changed_notify(const std::string &key)
{
  TRACE_ENTER_MSG("GUI::config_changed_notify", key);

#if defined(HAVE_LANGUAGE_SELECTION)
  if (key == GUIConfig::CFG_KEY_LOCALE)
    {
      string locale = GUIConfig::get_locale();
      Locale::set_locale(locale);

      menus->locale_changed();
    }
#endif

  TRACE_EXIT();
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
  init_multihead();
  collect_garbage();

  active_break_id = break_id;
  for (int i = 0; i < num_heads; i++)
    {
      prelude_windows[i] = new PreludeWindow(heads[i], break_id);
    }

  active_prelude_count = num_heads;
  TRACE_EXIT();
}

void
GUI::create_break_window(BreakId break_id, BreakHint break_hint)
{
  TRACE_ENTER_MSG("GUI::create_break_window", break_id << " " << break_hint);
  hide_break_window();
  init_multihead();
  collect_garbage();

  BreakWindow::BreakFlags break_flags = BreakWindow::BREAK_FLAGS_NONE;
  bool ignorable = GUIConfig::get_ignorable(break_id);
  bool skippable = GUIConfig::get_skippable(break_id);

  if (break_hint & BREAK_HINT_USER_INITIATED)
    {
      break_flags = (BreakWindow::BREAK_FLAGS_POSTPONABLE | BreakWindow::BREAK_FLAGS_USER_INITIATED);

      if (skippable)
        {
          break_flags |= BreakWindow::BREAK_FLAGS_SKIPPABLE;
        }
    }
  else
    {
      if (ignorable)
        {
          break_flags |= BreakWindow::BREAK_FLAGS_POSTPONABLE;
        }

      if (skippable)
        {
          break_flags |= BreakWindow::BREAK_FLAGS_SKIPPABLE;
        }
    }

  if (break_hint & BREAK_HINT_NATURAL_BREAK)
    {
      break_flags |=
        (BreakWindow::BREAK_FLAGS_NO_EXERCISES | BreakWindow::BREAK_FLAGS_NATURAL | BreakWindow::BREAK_FLAGS_POSTPONABLE);
    }

  active_break_id = break_id;

  for (int i = 0; i < num_heads; i++)
    {
      IBreakWindow *break_window = create_break_window(heads[i], break_id, break_flags);

      break_windows[i] = break_window;

      break_window->set_response(response);
      break_window->init();
    }

  active_break_count = num_heads;

  TRACE_EXIT();
}

void
GUI::hide_break_window()
{
  TRACE_ENTER("GUI::hide_break_window");
  active_break_id = BREAK_ID_NONE;

  for (int i = 0; i < active_prelude_count; i++)
    {
      if (prelude_windows[i] != NULL)
        {
          prelude_windows[i]->stop();
        }
    }
  if (active_prelude_count > 0)
    {
      prelude_window_destroy = true;
    }

  for (int i = 0; i < active_break_count; i++)
    {
      if (break_windows[i] != NULL)
        {
          break_windows[i]->stop();
        }
    }
  if (active_break_count > 0)
    {
      TRACE_MSG("break_window_destroy = true");
      break_window_destroy = true;
    }

  ungrab();

  TRACE_EXIT();
}

void
GUI::show_break_window()
{
  TRACE_ENTER("GUI::hide_break_window");

  for (int i = 0; i < active_prelude_count; i++)
    {
      if (prelude_windows[i] != NULL)
        {
          prelude_windows[i]->start();
        }
    }
  for (int i = 0; i < active_break_count; i++)
    {
      if (break_windows[i] != NULL)
        {
          break_windows[i]->start();
        }
    }

  if (GUIConfig::get_block_mode() != GUIConfig::BLOCK_MODE_NONE)
    {
      grab();
    }

  TRACE_EXIT();
}

void
GUI::refresh_break_window()
{
  for (int i = 0; i < active_prelude_count; i++)
    {
      if (prelude_windows[i] != NULL)
        {
          prelude_windows[i]->refresh();
        }
    }
  for (int i = 0; i < active_break_count; i++)
    {
      if (break_windows[i] != NULL)
        {
          break_windows[i]->refresh();
        }
    }
}

void
GUI::set_break_progress(int value, int max_value)
{
  for (int i = 0; i < active_prelude_count; i++)
    {
      if (prelude_windows[i] != NULL)
        {
          prelude_windows[i]->set_progress(value, max_value);
        }
    }

  for (int i = 0; i < active_break_count; i++)
    {
      if (break_windows[i] != NULL)
        {
          break_windows[i]->set_progress(value, max_value);
        }
    }
}

void
GUI::set_prelude_stage(PreludeStage stage)
{
  for (int i = 0; i < active_prelude_count; i++)
    {
      if (prelude_windows[i] != NULL)
        {
          prelude_windows[i]->set_stage(stage);
        }
    }
}

void
GUI::set_prelude_progress_text(PreludeProgressText text)
{
  for (int i = 0; i < active_prelude_count; i++)
    {
      if (prelude_windows[i] != NULL)
        {
          prelude_windows[i]->set_progress_text(text);
        }
    }
}

//! Destroys the break/prelude windows, if requested.
void
GUI::collect_garbage()
{
  TRACE_ENTER("GUI::collect_garbage");
  if (prelude_window_destroy)
    {
      if (prelude_windows != NULL)
        {
          for (int i = 0; i < active_prelude_count; i++)
            {
              if (prelude_windows[i] != NULL)
                {
                  prelude_windows[i]->destroy();
                  prelude_windows[i] = NULL;
                }
            }
        }
      prelude_window_destroy = false;
      active_prelude_count = 0;
    }

  if (break_window_destroy)
    {
      if (break_windows != NULL)
        {
          TRACE_MSG("1");
          for (int i = 0; i < active_break_count; i++)
            {
              TRACE_MSG("2 " << i);
              if (break_windows[i] != NULL)
                {
                  TRACE_MSG("3");
                  break_windows[i]->destroy();
                  break_windows[i] = NULL;
                }
            }
        }
      break_window_destroy = false;
      active_break_count = 0;
    }
  TRACE_EXIT();
}

//! Grabs the pointer and the keyboard.
bool
GUI::grab()
{
  if (break_windows != NULL && active_break_count > 0)
    {
      GdkWindow **windows = new GdkWindow *[active_break_count];

      for (int i = 0; i < active_break_count; i++)
        {
          Glib::RefPtr<Gdk::Window> window = break_windows[i]->get_gdk_window();
          windows[i] = window->gobj();
        }

#if defined(PLATFORM_OS_UNIX)
      grab_wanted = true;
#endif
      if (!grab_handle)
        {
          grab_handle = WindowHints::grab(active_break_count, windows);
#if defined(PLATFORM_OS_UNIX)
          if (!grab_handle && !grab_retry_connection.connected())
            {
              grab_retry_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUI::on_grab_retry_timer), 2000);
            }
#endif
        }

      delete[] windows;
    }
  return grab_handle != NULL;
}

//! Releases the pointer and keyboard grab
void
GUI::ungrab()
{
#if defined(PLATFORM_OS_UNIX)
  grab_wanted = false;
#endif
  if (grab_handle)
    {
#if defined(PLATFORM_OS_UNIX)
      grab_retry_connection.disconnect();
#endif
      WindowHints::ungrab(grab_handle);
      grab_handle = NULL;
    }
}

void
GUI::interrupt_grab()
{
  if (grab_handle)
    {
#if defined(PLATFORM_OS_UNIX)
      grab_wanted = true;

      WindowHints::ungrab(grab_handle);
      grab_handle = NULL;
      if (!grab_retry_connection.connected())
        {
          Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUI::on_grab_retry_timer), 2000);
        }
#endif
    }
}

#if defined(PLATFORM_OS_UNIX)
//! Reattempt to get the grab
bool
GUI::on_grab_retry_timer()
{
  TRACE_ENTER("GUI::on_grab_retry_timer");
  bool ret = false;
  if (grab_wanted)
    {
      ret = !grab();
    }
  else
    {
      ret = false;
    }
  TRACE_MSG(ret);
  TRACE_EXIT();
  return ret;
}
#endif

bool
GUI::on_operational_mode_warning_timer()
{
  OperationMode mode = core->get_operation_mode();
  if (mode == OPERATION_MODE_SUSPENDED)
    {
      status_icon->show_balloon("operation_mode",
                                _("Workrave is in suspended mode. "
                                  "Mouse and keyboard activity will not be monitored."));
    }
  else if (mode == OPERATION_MODE_QUIET)
    {
      status_icon->show_balloon("operation_mode",
                                _("Workrave is in quiet mode. "
                                  "No break windows will appear."));
    }
  return false;
}

HeadInfo &
GUI::get_head(int head)
{
  return heads[head < num_heads ? head : 0];
}

int
GUI::map_to_head(int &x, int &y)
{
  int head = -1;

  for (int i = 0; i < num_heads; i++)
    {
      int left, top, width, height;

      left = heads[i].get_x();
      top = heads[i].get_y();
      width = heads[i].get_width();
      height = heads[i].get_height();

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
          head = i;
          break;
        }
    }

  if (head < 0)
    {
      head = 0;
      x = y = 256;
    }
  return head;
}

void
GUI::map_from_head(int &x, int &y, int head)
{
  HeadInfo &h = get_head(head);
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

  if (head >= num_heads)
    {
      head = 0;
    }

  HeadInfo &h = get_head(head);
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
    case OPERATION_MODE_SUSPENDED:
      tip = string(_("Mode: ")) + _("Suspended");
      break;

    case OPERATION_MODE_QUIET:
      tip = string(_("Mode: ")) + _("Quiet");
      break;

    case OPERATION_MODE_NORMAL:
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

      if (b != NULL && on)
        {
          // Collect some data.
          time_t maxActiveTime = b->get_limit();
          time_t activeTime = b->get_elapsed_time();
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
      CoreFactory::get_configurator()->set_value(GUIConfig::CFG_KEY_CLOSEWARN_ENABLED, false);
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
      GUIConfig::set_trayicon_enabled(true);
    }
#else
  bool can_close_main_window = applet_control->is_visible() || status_icon->is_visible();
  main_window->set_can_close(can_close_main_window);
#endif
  TRACE_EXIT();
}

#if defined(PLATFORM_OS_WINDOWS)
void
GUI::win32_init_filter()
{
  GtkWidget *window = (GtkWidget *)main_window->gobj();
  GdkWindow *gdk_window = gtk_widget_get_window(window);
  gdk_window_add_filter(gdk_window, win32_filter_func, this);

  HWND hwnd = (HWND)GDK_WINDOW_HWND(gdk_window);

  WTSRegisterSessionNotification(hwnd, NOTIFY_FOR_THIS_SESSION);
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
              ICore *core = CoreFactory::get_core();
              core->set_powersave(false);
            }
            break;

          case PBT_APMSUSPEND:
            {
              TRACE_MSG("Suspend");
              ICore *core = CoreFactory::get_core();
              core->set_powersave(true);
            }
            break;
          }
      }
      break;

    case WM_DISPLAYCHANGE:
      {
        TRACE_MSG("WM_DISPLAYCHANGE " << msg->wParam << " " << msg->lParam);
        gui->init_multihead();
      }
      break;

    case WM_TIMECHANGE:
      {
        TRACE_MSG("WM_TIMECHANGE " << msg->wParam << " " << msg->lParam);
        ICore *core = CoreFactory::get_core();
        core->time_changed();
      }
      break;

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
