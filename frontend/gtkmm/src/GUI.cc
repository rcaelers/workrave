// GUI.cc --- The WorkRave GUI
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"
#include "nls.h"
#include "debug.hh"

#if defined(PLATFORM_OS_WIN32)
#include "w32debug.hh"
#endif

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <glibmm/refptr.h>

#include <unistd.h>
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

#if defined(PLATFORM_OS_WIN32)
#include "W32AppletWindow.hh"
#include <gdk/gdkwin32.h>
#include <pbt.h>
#endif

#if defined(PLATFORM_OS_OSX)
#include "OSXUtil.hh"
#include <strings.h>
#include <mach-o/dyld.h>
#include <sys/param.h>
#import <Cocoa/Cocoa.h>
#import "AppController.h"
#include <Carbon/Carbon.h>
#endif

#if defined(HAVE_GCONF)
#include <gconf/gconf-client.h>
#endif

#if defined(HAVE_GNOMEMM)
#include "libgnomeuimm/wrap_init.h"
#endif

#if defined(HAVE_KDE)
#include <dcopclient.h>
#include <kapp.h>
#endif

#if defined(HAVE_DBUS)
#include "DBus.hh"
#include "DBusException.hh"
#endif

GUI *GUI::instance = NULL;


//! GUI Constructor.
/*!
 *  \param argc number of command line parameters.
 *  \param argv all command line parameters.
 */
GUI::GUI(int argc, char **argv) :
  configurator(NULL),
  core(NULL),
  sound_player(NULL),
  break_windows(NULL),
  prelude_windows(NULL),
  active_break_count(0),
  active_prelude_count(0),
  response(NULL),
  active_break_id(BREAK_ID_NONE),
  main_window(NULL),
  menus(0),
  tooltips(NULL),
  break_window_destroy(false),
  prelude_window_destroy(false),
  heads(NULL),
  num_heads(-1),
  screen_width(-1),
  screen_height(-1),
#if defined(PLATFORM_OS_UNIX)
  grab_wanted(false),
#endif
  grab_handle(NULL),
  status_icon(NULL),
  applet_control(NULL)
{
  TRACE_ENTER("GUI:GUI");

  assert(! instance);
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

  delete [] prelude_windows;
  delete [] break_windows;
  delete [] heads;

  delete sound_player;

  TRACE_EXIT();
}


//! Forces a restbreak.
void
GUI::restbreak_now()
{
  core->force_break(BREAK_ID_REST_BREAK, true);
}


//! The main entry point.
void
GUI::main()
{
  TRACE_ENTER("GUI::main");

#if defined (PLATFORM_OS_WIN32) || defined(PLATFORM_OS_OSX)
  // Win32/OSX need this....
  if (!g_thread_supported())
    {
      g_thread_init(NULL);
    }
#endif
  
  Gtk::Main kit(argc, argv);

  init_core();
  init_nls();
  init_platform();
  init_debug();
  init_sound_player();
  init_multihead();
  init_dbus();
  init_gui();
  
  on_timer();

#if defined(PLATFORM_OS_WIN32)
  // FIXME: debug, remove later
  APPEND_TIME( "Workrave started and initialized", "Entering event loop." );
#endif

  // Enter the event loop
  gdk_threads_enter();
  Gtk::Main::run();
  gdk_threads_leave();

  delete main_window;
  main_window = NULL;

  delete applet_control;
  applet_control = NULL;

  TRACE_EXIT();
}


//! Terminates the GUI.
void
GUI::terminate()
{
  TRACE_ENTER("GUI::terminate");

  // HACK: Without it status icon keeps on dangling in tray
  // Nicer solution: nicely cleanup complete gui ~GUI()
  if (status_icon)
    {
      delete status_icon;
      status_icon = 0;
    }

  CoreFactory::get_configurator()->save();

  collect_garbage();

  Gtk::Main::quit();
  TRACE_EXIT();
}


//! Opens the main window.
void
GUI::open_main_window()
{
  if (main_window != NULL)
    {
      main_window->open_window();
    }
}


//! Closes the main window.
void
GUI::close_main_window()
{
  if (main_window != NULL)
    {
      main_window->close_window();
    }
}


//! Periodic heartbeat.
bool
GUI::on_timer()
{
  std::string tip = get_timers_tooltip();
  
  if (core != NULL)
    {
      core->heartbeat();
    }

  if (main_window != NULL)
    {
      main_window->update();
    }

  if (applet_control != NULL)
    {
      applet_control->heartbeat();
      applet_control->set_timers_tooltip(tip);
    }

  if (status_icon)
    {
      status_icon->set_timers_tooltip(tip);
    }

  heartbeat_signal();

  collect_garbage();

  return true;
}

#if defined(NDEBUG)
static void my_log_handler(const gchar *log_domain, GLogLevelFlags log_level,
                           const gchar *message, gpointer user_data)
{
}
#endif

void
GUI::init_platform()
{
#if defined(HAVE_GNOME)
  init_gnome();
#endif
#if defined(HAVE_KDE)
  init_kde();
#endif
#if defined(PLATFORM_OS_OSX)
  [ [ AppController alloc ] init ];
#endif
  
#if defined (PLATFORM_OS_WIN32) || defined(PLATFORM_OS_OSX)
  // Win32/OSX need this....
  if (!g_thread_supported())
    {
      g_thread_init(NULL);
    }
#endif

#if defined(PLATFORM_OS_UNIX)
  char *display = gdk_get_display();
  System::init(display);
  g_free(display);
#else
  System::init();
#endif
  
  srand(time(NULL));
}


#if defined(HAVE_GNOME)
void
GUI::init_gnome()
{
  TRACE_ENTER("GUI::init_gnome");

  gnome_init("workrave", VERSION, argc, argv);
#if defined(HAVE_GNOMEMM)
  Gnome::UI::wrap_init();

  Gnome::UI::Client *client = Gnome::UI::Client::master_client();
  if (client != NULL)
    {
      client->signal_save_yourself().connect(sigc::mem_fun(*this, &GUI::on_save_yourself));
      client->signal_die().connect(sigc::mem_fun(*this, &GUI::on_die));
    }

  TRACE_EXIT();
#endif
}

#if defined(HAVE_GNOMEMM)
void
GUI::on_die()
{
  TRACE_ENTER("GUI::on_die");

  CoreFactory::get_configurator()->save();
  Gtk::Main::quit();

  TRACE_EXIT();
}


bool
GUI::on_save_yourself(int phase, Gnome::UI::SaveStyle save_style, bool shutdown,
                      Gnome::UI::InteractStyle interact_style, bool fast)
{
  TRACE_ENTER("GUI::on_save_yourself");

  (void) phase;
  (void) save_style;
  (void) shutdown;
  (void) interact_style;
  (void) fast;

  Gnome::UI::Client *client = Gnome::UI::Client::master_client();

  vector<string> args;
  args.push_back(argv[0] != NULL ? argv[0] : "workrave");

  bool skip = false;

  if (applet_control != NULL)
    {
      if (applet_control->is_visible(AppletControl::APPLET_GNOME))
        {
          skip = true;
        }
    }

  if (skip)
    {
      client->set_restart_style(GNOME_RESTART_NEVER);
    }
  else
    {
      client->set_restart_style(GNOME_RESTART_IF_RUNNING);

      char *display_name = gdk_get_display();
      if (display_name != NULL)
        {
          args.push_back("--display");
          args.push_back(display_name);
        }
    }

  client->set_clone_command(args);
  client->set_restart_command(args);

  TRACE_EXIT();
  return true;
}

#endif // defined HAVE_GNOMEMM
#endif // defined HAVE_GNOME


#if defined(HAVE_KDE)
void
GUI::init_kde()
{
  // FIXME: memory leaks in here.
  TRACE_ENTER("GUI::init_kde");
  new KApplication(argc, argv, "Workrave");
  bool rc = kapp->dcopClient()->attach();
  TRACE_MSG(rc);

  TRACE_EXIT();
}
#endif

//! Initializes messages hooks.
void
GUI::init_debug()
{
#if defined(NDEBUG)
  const char *domains[] = { NULL, "Gtk", "GLib", "Gdk", "gtkmm", "GLib-GObject" };
  for (unsigned int i = 0; i < sizeof(domains)/sizeof(char *); i++)
    {
      g_log_set_handler(domains[i],
                        (GLogLevelFlags) (G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION),
                        my_log_handler, NULL);
    }

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

#  if !defined(HAVE_GNOME)
  gtk_set_locale();
#  endif
  const char *locale_dir;

#if defined(PLATFORM_OS_WIN32)
  string dir = Util::get_application_directory() + "\\lib\\locale";
  locale_dir = dir.c_str();
#elif defined(PLATFORM_OS_OSX)
  char locale_path[MAXPATHLEN * 4];
  char execpath[MAXPATHLEN+1];
  uint32_t pathsz = sizeof (execpath);

  _NSGetExecutablePath(execpath, &pathsz);

  gchar *dir_path = g_path_get_dirname(execpath);
  strcpy(locale_path, dir_path);
  g_free(dir_path);

  // Locale
  strcat(locale_path, "/../Resources/locale");
  locale_dir = locale_path;
#else
  locale_dir = GNOMELOCALEDIR;
#  endif

#  ifdef HAVE_SETLOCALE
  setlocale(LC_ALL, "");
#  endif

#if defined(PLATFORM_OS_WIN32)
  bindtextdomain("gtk20", locale_dir);
  bindtextdomain("iso_3166", locale_dir);
  bindtextdomain("iso_639", locale_dir);
  bindtextdomain("glib20", locale_dir);
  bind_textdomain_codeset("gk20", "UTF-8");
  bind_textdomain_codeset("glib20", "UTF-8");
  bind_textdomain_codeset("iso_3166", "UTF-8");
  bind_textdomain_codeset("iso_639", "UTF-8");

  CoreFactory::get_configurator()->add_listener(GUIConfig::CFG_KEY_LOCALE, this);
#endif
  
  bindtextdomain(PACKAGE, locale_dir);
  bind_textdomain_codeset(PACKAGE, "UTF-8");
  textdomain(PACKAGE);
 
#endif
}


//! Initializes the core.
void
GUI::init_core()
{
  string display_name;
  
#if defined(PLATFORM_OS_UNIX)
  char *display = gdk_get_display();
  if (display != NULL)
    {
      display_name = display;
    }
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
  if (num_heads == -1)
    {
      init_multihead_mem(1);

      heads[0].valid = false;
      heads[0].count = 0;
      heads[0].geometry.set_width(gdk_screen_width());
      heads[0].geometry.set_height(gdk_screen_height());
      heads[0].geometry.set_x(0);
      heads[0].geometry.set_y(0);
    }

  init_multihead_desktop();
  TRACE_EXIT();
}

void
GUI::init_multihead_mem(int new_num_heads)
{
  TRACE_ENTER("GUI::init_multihead_mem");
  if (new_num_heads != num_heads || num_heads <= 0)
    {
      delete [] heads;
      heads = new HeadInfo[new_num_heads];

      PreludeWindow **old_prelude_windows = prelude_windows;
      IBreakWindow **old_break_windows = break_windows;

      prelude_windows = new PreludeWindow*[new_num_heads];/* LEAK */
      break_windows = new IBreakWindow*[new_num_heads]; /* LEAK */

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
              if (old_prelude_windows != NULL &&
                  old_prelude_windows[i] != NULL)
                {
                  old_prelude_windows[i]->destroy();
                }
              if (old_break_windows != NULL &&
                  old_break_windows[i] != NULL)
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

      delete [] old_prelude_windows;
      delete [] old_break_windows;

      num_heads = new_num_heads;
    }
  TRACE_EXIT();
}

void
GUI::init_multihead_desktop()
{
  TRACE_ENTER("GUI::init_multihead_desktop");
  TRACE_MSG("gdk width x height " << gdk_screen_width() << " " << gdk_screen_height());

  int width = 0;
  int height = 0;

  // Use head info to determine screen size. I hope this results
  // in the same size as the gdk_screen_xxx....
  for (int i = 0; i < num_heads; i++)
    {
      if (!heads[i].valid)
        {
          // Not all heads have valid geometry. Use gdk.
          width = gdk_screen_width();
          height = gdk_screen_height();
          break;
        }

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

      for (int i = 0; i < num_screens; i++)
        {
          Glib::RefPtr<Gdk::Screen> screen = display->get_screen(i);
          if (screen)
            {
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
                      heads[count].valid = true;
                      heads[count].count = count;

                      heads[count].geometry = rect;
                      count++;
                    }

                  TRACE_MSG("Screen #" << i << " Monitor #" << j << "  "
                            << rect.get_x() << " "
                            << rect.get_y() << " "
                            << rect.get_width() << " "
                            << rect.get_height() << " "
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
  tooltips = manage(new Gtk::Tooltips());
  tooltips->enable();

  menus = new Menus();

  // The main status window.
  main_window = new MainWindow();

  // The applet window.
  applet_control = new AppletControl();
  applet_control->init();

  AppletWindow *applet_window = NULL;
#if defined(HAVE_GNOME)
  applet_window = applet_control->get_applet_window(AppletControl::APPLET_GNOME);
#endif
#if defined(PLATFORM_OS_WIN32)
  applet_window = applet_control->get_applet_window(AppletControl::APPLET_W32);
#endif

  menus->init(main_window, applet_window);
  menus->resync();

  // Status icon
  status_icon = new StatusIcon(*main_window);
  
#ifdef HAVE_DBUS
  DBus *dbus = CoreFactory::get_dbus();

  if (dbus != NULL && dbus->is_available())
    {
      dbus->connect("/org/workrave/Workrave/UI",
                    "org.workrave.ControlInterface",
                    menus);
      
    }
#endif
  
#if defined(PLATFORM_OS_WIN32)
  win32_init_filter();
#endif

  // Periodic timer.
  Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUI::on_timer), 1000);
}


void
GUI::init_dbus()
{
#if defined(HAVE_DBUS)
  DBus *dbus = CoreFactory::get_dbus();

  if (dbus != NULL && dbus->is_available())
    {
      if (!dbus->is_owner())
        {
          Gtk::MessageDialog dialog("Could not initialize Workrave. Is Workrave already running?",
                                    false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
          dialog.show();
          dialog.run();
          exit(1);
        }
      
      try
        {
          dbus->register_service("org.workrave.Workrave.Activator");
          
          dbus->register_object_path("/org/workrave/Workrave/UI");

          extern void init_DBusGUI(DBus *dbus);
          init_DBusGUI(dbus);
        }
      catch (DBusException &e)
        {
        }
    }  
#endif
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
  sound_player = new SoundPlayer(); /* LEAK */
}


void
GUI::core_event_notify(CoreEvent event)
{
  TRACE_ENTER_MSG("GUI::core_event_notify", event);
  // FIXME: HACK
  SoundPlayer::SoundEvent snd = (SoundPlayer::SoundEvent) event;
  if (sound_player != NULL)
    {
      TRACE_MSG("play");
      sound_player->play_sound(snd);
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
GUI::config_changed_notify(const std::string &key)
{
  (void) key;
  
#if defined(HAVE_LANGUAGE_SELECTION)
  if (key == GUIConfig::CFG_KEY_LOCALE)
    {
      string locale = GUIConfig::get_locale();
      Locale::set_locale(locale);

      menus->locale_changed();
    }
#endif
}


void
GUI::set_break_response(IBreakResponse *rep)
{
  response = rep;
}



void
GUI::create_prelude_window(BreakId break_id)
{
  hide_break_window();
  init_multihead();
  collect_garbage();

  active_break_id = break_id;
  for (int i = 0; i < num_heads; i++)
    {
      prelude_windows[i] = new PreludeWindow(heads[i], break_id);
      prelude_windows[i]->set_response(response);
    }

  active_prelude_count = num_heads;
}


void
GUI::create_break_window(BreakId break_id, bool user_initiated)
{
  TRACE_ENTER_MSG("GUI::start_break_window", num_heads);
  hide_break_window();
  init_multihead();
  collect_garbage();

  BreakWindow::BreakFlags break_flags = BreakWindow::BREAK_FLAGS_NONE;
  bool ignorable = GUIConfig::get_ignorable(break_id);

  if (user_initiated && !ignorable)
    {
      break_flags = BreakWindow::BREAK_FLAGS_POSTPONABLE;
    }
  else if (ignorable)
    {
      break_flags =  ( BreakWindow::BREAK_FLAGS_POSTPONABLE |
                       BreakWindow::BREAK_FLAGS_SKIPPABLE);
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
  if (GUIConfig::get_block_mode() != GUIConfig::BLOCK_MODE_NONE)
    {
      grab();
    }

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
      GdkWindow *windows[active_break_count];

      for (int i = 0; i < active_break_count; i++)
        {
          Glib::RefPtr<Gdk::Window> window = break_windows[i]->get_gdk_window();
          windows[i] = window->gobj();
        }

#if defined(PLATFORM_OS_UNIX)
      grab_wanted = true;
#endif
      if (! grab_handle)
        {
          grab_handle = WindowHints::grab(active_break_count, windows);
#if defined(PLATFORM_OS_UNIX)
          if (! grab_handle && !grab_retry_connection.connected())
            {
              grab_retry_connection =
                Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUI::on_grab_retry_timer), 2000);
            }
#endif
        }
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
GUI::bound_head(int &x, int &y, int width, int height, int head)
{
  bool ret = false;

  HeadInfo &h = get_head(head);
  if (x < - h.get_width())
    {
      x = 0;
      ret = true;
    }
  if (y < - h.get_height())
    {
      y = 0;
      ret = true;
    }

  // Make sure something remains visible..
  if (x > - 10 && x < 0)
    {
      x = - 10;
      ret = true;
    }
  if (y > - 10 && y < 0)
    {
      y = - 10;
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
  //FIXME: duplicate
  char *labels[] = { _("Micro-break"), _("Rest break"), _("Daily limit") };
  string tip = "";

  ICore *core = CoreFactory::get_core();
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

#if !defined(PLATFORM_OS_WIN32)
          // Win32 tip is limited in length
          if (tip == "")
            {
              tip = "Workrave";
            }
#endif
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


#if defined(PLATFORM_OS_WIN32)
void
GUI::win32_init_filter()
{
  GtkWidget *window = main_window->Gtk::Widget::gobj();
  GdkWindow *gdk_window = window->window;
  gdk_window_add_filter(gdk_window, win32_filter_func, this);
}

GdkFilterReturn
GUI::win32_filter_func (void     *xevent,
                        GdkEvent *event,
                        gpointer  data)
{
  TRACE_ENTER("GUI::win32_filter_func");
  (void) event;
  GUI *gui = static_cast<GUI*>(data);

  MSG *msg = (MSG *) xevent;
  GdkFilterReturn ret = GDK_FILTER_CONTINUE;
  switch (msg->message)
    {
    case WM_POWERBROADCAST:
      {
        TRACE_MSG("WM_POWERBROADCAST " << msg->wParam << " " << msg->lParam);

#if defined(PLATFORM_OS_WIN32)
// FIXME: debug, remove later
switch (msg->wParam)
{
case PBT_APMBATTERYLOW:
APPEND_TIME("WM_POWERBROADCAST", "PBT_APMBATTERYLOW");
break;
case PBT_APMOEMEVENT:
APPEND_TIME("WM_POWERBROADCAST", "PBT_APMOEMEVENT");
break;
case PBT_APMPOWERSTATUSCHANGE:
APPEND_TIME("WM_POWERBROADCAST", "PBT_APMPOWERSTATUSCHANGE");
break;
case PBT_APMQUERYSUSPEND:
APPEND_TIME("WM_POWERBROADCAST", "PBT_APMQUERYSUSPEND");
break;
case PBT_APMQUERYSUSPENDFAILED:
APPEND_TIME("WM_POWERBROADCAST", "PBT_APMQUERYSUSPENDFAILED");
break;
case PBT_APMRESUMEAUTOMATIC:
APPEND_TIME("WM_POWERBROADCAST", "PBT_APMRESUMEAUTOMATIC");
break;
case PBT_APMRESUMECRITICAL:
APPEND_TIME("WM_POWERBROADCAST", "PBT_APMRESUMECRITICAL");
break;
case PBT_APMRESUMESUSPEND:
APPEND_TIME("WM_POWERBROADCAST", "PBT_APMRESUMESUSPEND");
break;
case PBT_APMSUSPEND:
APPEND_TIME("WM_POWERBROADCAST", "PBT_APMSUSPEND");
break;
default:
APPEND_TIME("WM_POWERBROADCAST", "<UNKNOWN MESSAGE> : " << hex << msg->wParam );
}
#endif

          switch (msg->wParam)
            {
            case PBT_APMQUERYSUSPEND:
              TRACE_MSG("Query Suspend");
              break;

            case PBT_APMQUERYSUSPENDFAILED:
              TRACE_MSG("Query Suspend Failed");
              break;

            case PBT_APMRESUMESUSPEND:
              {
                TRACE_MSG("Resume suspend");
                ICore *core = CoreFactory::get_core();
                if (core != NULL)
                  {
                    core->set_powersave(false);
                  }
              }
              break;

            case PBT_APMSUSPEND:
              {
                TRACE_MSG("Suspend");
                ICore *core = CoreFactory::get_core();
                if (core != NULL)
                  {
                    core->set_powersave(true);
                  }
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
      W32AppletWindow *applet_window =
        static_cast<W32AppletWindow*>
        (gui->applet_control->get_applet_window(AppletControl::APPLET_W32));
      if (applet_window)
        {
          ret = applet_window->win32_filter_func(xevent, event);
        }
    }

  if (ret != GDK_FILTER_REMOVE && gui->status_icon)
    {
      ret = gui->status_icon->win32_filter_func(xevent, event);
    }

  TRACE_EXIT();
  return ret;
}


#endif
