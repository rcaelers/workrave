// GUI.cc --- The WorkRave GUI
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"
#include "nls.h"
#include "debug.hh"

#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>

#include "GUI.hh"

#include "CoreFactory.hh"
#include "CoreInterface.hh"
#include "ConfiguratorInterface.hh"

#include "BreakResponseInterface.hh"
#include "Dispatcher.hh"
#include "DailyLimitWindow.hh"
#include "MainWindow.hh"
#include "TimerBox.hh"
#include "BreakWindowInterface.hh"
#include "BreakWindow.hh"
#include "MicroPauseWindow.hh"
#include "PreludeWindow.hh"
#include "RestBreakWindow.hh"
#include "Util.hh"
#include "WindowHints.hh"

#ifdef HAVE_X
#include "AppletWindow.hh"
#endif

#include <gtk/gtk.h>

#ifdef HAVE_GCONF
#include <gconf/gconf-client.h>
#endif

#include "SoundPlayer.hh"

#ifdef HAVE_GNOME
#include "RemoteControl.hh"
#include "libgnomeuimm/wrap_init.h"
#endif

#ifdef WIN32
#include "crashlog.h"
#endif

GUI *GUI::instance = NULL;

//! GUI Constructor.
/*!
 *  \param argc number of command line parameters.
 *  \param argv all command line parameters.
 */
GUI::GUI(int argc, char **argv)  :
  configurator(NULL),
  core(NULL),
  sound_player(NULL),
  break_windows(NULL),
  prelude_windows(NULL),
  active_break_count(0),
  active_prelude_count(0),
  response(NULL),
  active_break_id(BREAK_ID_NONE),
#ifdef HAVE_X
  applet_window(NULL),
#endif  
  main_window(NULL),
  tooltips(NULL),
  break_window_destroy(false),
  prelude_window_destroy(false),
  heads(NULL),
  num_heads(-1)
#ifdef WIN32
  ,
  enum_monitors(NULL),
  user_lib(NULL),
  current_monitor(0)
#endif
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
  
  if (dispatcher != NULL)
    {
      dispatch_connection.disconnect();
      delete dispatcher;
    }

  if (core != NULL)
    {
      // FIXME: cannot delete interface. delete core;
    }

  if (main_window != NULL)
    {
      delete main_window;
    }

#ifdef HAVE_X
  if (applet_window != NULL)
    {
      delete applet_window;
    }
#endif
  
#ifdef WIN32
  if (user_lib != NULL)
    {
      FreeLibrary(user_lib);
    }
#endif
  
  TRACE_EXIT();
}


//! Forces a restbreak.
void
GUI::restbreak_now()
{
  core->force_break(BREAK_ID_REST_BREAK);
}

//! The main entry point.
void
GUI::main()
{
  TRACE_ENTER("GUI::main");

#ifdef WIN32
  __try1(exception_handler);
#endif

//    char *x = NULL;
//    *x = 1;
  Gtk::Main kit(argc, argv);
  
#ifdef HAVE_GNOME
  init_gnome();
#endif

#ifdef WIN32
  // Win32 needs this....
  if (!g_thread_supported())
    {
      g_thread_init (NULL);
    }
#endif
  
  init_debug();
  init_nls();
  init_core();
  init_sound_player();
  init_multihead();
  init_remote_control();
  init_gui();

  on_timer();
  
  // Enter the event loop
  gdk_threads_enter();
  Gtk::Main::run();
  gdk_threads_leave();

  delete main_window;
  main_window = NULL;

#ifdef HAVE_X
  delete applet_window;
  applet_window = NULL;
#endif  

#ifdef WIN32
  __except1;
#endif
  
  TRACE_EXIT();
}


//! Terminates the GUI.
void
GUI::terminate()
{
  TRACE_ENTER("GUI::terminate");

  CoreFactory::get_configurator()->save();
  
  if (main_window != NULL)
    {
      // Remember position
      main_window->remember_position();
    }

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


//! Toggles the main window.
void
GUI::toggle_main_window()
{
  if (main_window != NULL)
    {
      main_window->toggle_window();
    }
}


//! Periodic heartbeat.
bool
GUI::on_timer()
{
  if (core != NULL)
    {
      core->heartbeat();
    }
  
  if (main_window != NULL)
    {
      main_window->update();
    }

#ifdef HAVE_X
  if (applet_window != NULL)
    {
      applet_window->update();
    }
#endif

  heartbeat_signal();

  collect_garbage();
  
  return true;
}

#ifdef NDEBUG
static void my_log_handler(const gchar *log_domain, GLogLevelFlags log_level,
                           const gchar *message, gpointer user_data)
{
}
#endif


#ifdef HAVE_GNOME
void
GUI::init_gnome()
{
  TRACE_ENTER("GUI::init_gnome");
  
  gnome_init("workrave", VERSION, argc, argv);

  Gnome::UI::wrap_init();

  Gnome::UI::Client *client = Gnome::UI::Client::master_client();
  if (client != NULL)
    {
      client->signal_save_yourself().connect(SigC::slot(*this, &GUI::on_save_yourself));
      client->signal_die().connect(SigC::slot(*this, &GUI::on_die));
    }
  
  TRACE_EXIT();
}

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
  
  if (main_window != NULL)
    {
      // Remember position
      main_window->remember_position();
    }

  Gnome::UI::Client *client = Gnome::UI::Client::master_client();

  vector<string> args;
  args.push_back(argv[0] != NULL ? argv[0] : "workrave");
  
  bool skip = false;
  if (applet_window != NULL)
    {
      if (applet_window->get_applet_mode() == AppletWindow::APPLET_GNOME)
        {
          skip = true;
        }
    }

  if (main_window != NULL)
    {
      bool iconified = main_window->get_iconified();
      TimerBox::set_enabled("main_window", !iconified);
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
          // g_free(display_name);
        }
    }

  client->set_clone_command(args);
  client->set_restart_command(args);

  TRACE_EXIT();
  return true;  
}

#endif  


//! Initializes messages hooks.
void
GUI::init_debug()
{
#ifdef NDEBUG
  char *domains[] = { NULL, "Gtk", "GLib", "Gdk", "gtkmm" };
  for (int i = 0; i < sizeof(domains)/sizeof(char *); i++)
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
#ifdef ENABLE_NLS
#  ifndef HAVE_GNOME 
  gtk_set_locale();
#  endif
  const char *locale_dir;
#ifdef WIN32
  string dir = Util::get_application_directory() + "\\lib\\locale";
  locale_dir = dir.c_str();
#else
  locale_dir = GNOMELOCALEDIR;
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
#ifdef HAVE_X
  char *display_name = gdk_get_display();
#else
  char *display_name = NULL;
#endif

  core = CoreFactory::get_core();
  core->init(argc, argv, this, display_name);
  core->set_core_events_listener(this);
  
// #ifdef HAVE_X
//    g_free(display_name);
// #endif
}


void
GUI::init_multihead()
{
  TRACE_ENTER("GUI::init_multihead");

#if defined(HAVE_GTK_MULTIHEAD)
  init_gtk_multihead();
#elif defined(WIN32)
  init_win32_multihead();
#endif
  if (num_heads == -1)
    {
      init_multihead_mem(1);
      heads[0].valid = false;
      heads[0].count = 0;
    }

  //init_multihead_desktop();
  TRACE_EXIT();
}

void
GUI::init_multihead_mem(int new_num_heads)
{
  TRACE_ENTER("GUI::init_multihead_mem");
  if (new_num_heads != num_heads || num_heads <= 0)
    {
      num_heads = new_num_heads;
                                  
      delete [] heads;
      heads = new HeadInfo[num_heads];

      TRACE_MSG("prelude_window_destroy = true");
      prelude_window_destroy = true;
      break_window_destroy = true;
      collect_garbage();

      delete [] prelude_windows;
      delete [] break_windows;
      prelude_windows = new PreludeWindow*[num_heads];
      break_windows = new BreakWindowInterface*[num_heads];
    }
  TRACE_EXIT();
}

void
GUI::init_multihead_desktop()
{
  TRACE_ENTER("GUI::init_multihead_desktop");
  TRACE_MSG("width x height " << gdk_screen_width() << " " << gdk_screen_height());
    
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
      int h = heads[i].geometry.get_y() + heads[i].geometry.get_width();

      if (w > width)
        {
          width = w;
        }
      if (h > height)
        {
          height = h;
        }
    }

  if (screen_width != width || screen_height != height)
    {
      relocate_main_window(width, height);
      screen_width = width;
      screen_height = height;
    }
}

void
GUI::relocate_main_window(int width, int height)
{
  TRACE_ENTER_MSG("GUI::relocate_main_window", width << " " << height);
  int x, y, w, h;

  // Get main window geometry
  GtkRequisition size;
  main_window->size_request(&size);
  main_window->get_position(x, y);
  w = size.width;
  h = size.height;

  TRACE_MSG("main window = " << x << " " << y << " " << w << " " << h);

  if (x + w >= width)
    {
      // relocate X.
    }
}

#ifdef HAVE_GTK_MULTIHEAD
void
GUI::init_gtk_multihead()
{
  int new_num_heads = 0;

  Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
  int num_screens = display->get_n_screens();
  
  for (int i = 0; i < num_screens; i++)
    {
      Glib::RefPtr<Gdk::Screen> screen = display->get_screen(i);
      if (!screen.is_null())
        {
          new_num_heads += screen->get_n_monitors();
        }
    }

  init_multihead_mem(new_num_heads);
  
  int count = 0;
  for (int i = 0; i < num_screens; i++)
    {
      Glib::RefPtr<Gdk::Screen> screen = display->get_screen(i);
      if (!screen.is_null())
        {
          int num_monitors = screen->get_n_monitors();
          for (int j = 0; j < num_monitors && count < num_heads; j++)
            {
              heads[count].screen = screen;
              heads[count].monitor = j;
              heads[count].valid = true;
              heads[count].count = count;
              
              screen->get_monitor_geometry(j, heads[count].geometry);

              count++;
            }
        }
    }
}
#endif


#ifdef WIN32
BOOL CALLBACK enum_monitor_callback(HMONITOR monitor, HDC, LPRECT rc, LPARAM dwData)
{
  (void) monitor;
  
  GUI *gui = (GUI *) dwData;

  gui->enum_monitor_callback(rc);
  
  return TRUE;
};

BOOL CALLBACK
GUI::enum_monitor_callback(LPRECT rc)
{
  TRACE_ENTER("GUI::enum_monitor_callback");
  TRACE_MSG(current_monitor << " " << num_heads);

  TRACE_MSG(rc->left << " " << rc->top << " - " << rc->right << " " << rc->bottom);
  
  if (current_monitor < num_heads)
    {
      Gdk::Rectangle &geometry = heads[current_monitor].geometry;
      
      geometry.set_x(rc->left);
      geometry.set_y(rc->top);
      geometry.set_width(rc->right - rc->left + 1);
      geometry.set_height(rc->bottom - rc->top + 1);
      heads[current_monitor].valid = true;
      heads[current_monitor].count = current_monitor;
      
      current_monitor++;
    }
  
  return TRUE;
};

BOOL EnumDisplayDevices(
  LPCTSTR lpDevice,                // device name
  DWORD iDevNum,                   // display device
  PDISPLAY_DEVICE lpDisplayDevice, // device information
  DWORD dwFlags                    // reserved
);

void
GUI::init_win32_multihead()
{
  TRACE_ENTER("GUI::init_win32_multihead");
  
  if (num_heads == -1)
    {
      TRACE_MSG("init");
      if (user_lib == NULL)
        {
          user_lib = LoadLibrary("user32.dll");
        }
      
      if (user_lib != NULL)
        {
          TRACE_MSG("get num");
          enum_monitors = (LUENUMDISPLAYMONITORS)GetProcAddress(user_lib,"EnumDisplayMonitors");
        }
          
      if (enum_monitors == NULL)
        {
          TRACE_MSG("!enum");
          if (user_lib != NULL)
            {
              FreeLibrary(user_lib);
              user_lib = NULL;
            }
        }
    }

  if (enum_monitors != NULL)
    {
      TRACE_MSG("enum");
      int new_num_heads = GetSystemMetrics(SM_CMONITORS);

      TRACE_MSG("# = " << new_num_heads);
      init_multihead_mem(new_num_heads);
  
      if (num_heads > 1)
        {
          current_monitor = 0;
          (*enum_monitors)(NULL,NULL, ::enum_monitor_callback, (LPARAM)this);
        }
      else if (num_heads == 1)
        {
          heads[0].valid = false;
          heads[0].count = 0;
        }
    }
  else
    {
      init_multihead_mem(1);
      heads[0].valid = false;
      heads[0].count = 0;
    }
}

#endif


//! Initializes the GUI
void
GUI::init_gui()
{
  // Setup the window hints module.
  WindowHints::init();

  // The main status window.
  main_window = new MainWindow();

#ifdef HAVE_X  
  // The applet window.
  applet_window = new AppletWindow();
#endif
  
  tooltips = manage(new Gtk::Tooltips());
  tooltips->enable();

  // Periodic timer.
  Glib::signal_timeout().connect(SigC::slot(*this, &GUI::on_timer), 1000);

  dispatcher = new Dispatcher;
  dispatch_connection = dispatcher->connect(SigC::slot_class(*this, &GUI::on_activity));
}


//! Initializes the CORBA remote control interface.
void
GUI::init_remote_control()
{
#ifdef HAVE_GNOME
  if (!bonobo_init(&argc, argv))
    {
      g_error (_("I could not initialize Bonobo"));
    }

  RemoteControl::get_instance();
  
  BonoboGenericFactory *factory;
  factory = bonobo_generic_factory_new("OAFIID:GNOME_Workrave_Factory", workrave_component_factory, NULL);
  bonobo_running_context_auto_exit_unref (BONOBO_OBJECT (factory));
#endif
}


//! Returns a break window for the specified break.
BreakWindowInterface *
GUI::create_break_window(HeadInfo &head, BreakId break_id, bool ignorable, bool insist)
{
  BreakWindowInterface *ret = NULL;
  
  if (break_id == BREAK_ID_MICRO_PAUSE)
    {
      ret = new MicroPauseWindow(head, core->get_timer(BREAK_ID_REST_BREAK), ignorable, insist);
    }
  else if (break_id == BREAK_ID_REST_BREAK)
    {
      ret = new RestBreakWindow(head, ignorable, insist); 
    }
  else if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      ret = new DailyLimitWindow(head, ignorable, insist);
    }

  return ret;
}


//! Initializes the sound player.
void
GUI::init_sound_player()
{
  sound_player = new SoundPlayer();
}



void
GUI::core_event_notify(CoreEvent event)
{
  TRACE_ENTER_MSG("GUI::core_event_notify", event)
  // FIXME: HACK
  SoundPlayerInterface::Sound snd = (SoundPlayerInterface::Sound) event;
  if (sound_player != NULL)
    {
      TRACE_MSG("play");
      sound_player->play_sound(snd);
    }
  TRACE_EXIT();
}


void
GUI::set_break_response(BreakResponseInterface *rep)
{
  response = rep;
}


void
GUI::start_prelude_window(BreakId break_id)
{
  hide_break_window();
  init_multihead();
  collect_garbage();
  
  active_break_id = break_id;
  for (int i = 0; i < num_heads; i++)
    {
      PreludeWindow *prelude_window = new PreludeWindow(heads[i], break_id);

      prelude_windows[i] = prelude_window;
      
      prelude_window->set_response(response);
      prelude_window->start();
    }

  active_prelude_count = num_heads;

}


void
GUI::start_break_window(BreakId break_id, bool ignorable, bool insist)
{
  TRACE_ENTER_MSG("GUI::start_break_window", num_heads);
  hide_break_window();
  init_multihead();
  collect_garbage();
  
  active_break_id = break_id;

  for (int i = 0; i < num_heads; i++)
    {
      BreakWindowInterface *break_window = create_break_window(heads[i], break_id, ignorable, insist);

      break_windows[i] = break_window;
      
      break_window->set_response(response);
      break_window->start();
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


bool
GUI::delayed_hide_break_window()
{
  bool ret = false;
  if (active_prelude_count > 0)
    {
      core->set_activity_monitor_listener(this);
      ret = true;
    }
  return ret;
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


bool
GUI::action_notify()
{
  TRACE_ENTER("GUI::action_notify");
  if (dispatcher != NULL)
    {
      TRACE_MSG("notify");
      dispatcher->send_notification();
    }
  TRACE_EXIT();
  return false; // false: kill listener.
}


void
GUI::on_activity()
{
  TRACE_ENTER("GUI::on_activity");
  if (response != NULL)
    {
      TRACE_MSG("notify");
      response->stop_prelude(active_break_id);
    }
  TRACE_EXIT();
}
