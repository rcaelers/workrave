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
#include "nls.h"

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>

#include "GUI.hh"

#include "Configurator.hh"
#include "ControlInterface.hh"
#include "DailyLimitWindow.hh"
#include "GUIControl.hh"
#include "MainWindow.hh"
#include "MicroPauseWindow.hh"
#include "PreludeWindow.hh"
#include "RestBreakWindow.hh"
#include "Util.hh"
#include "WindowHints.hh"

#include <gtk/gtk.h>

#ifdef HAVE_GCONF
#include <gconf/gconf-client.h>
#endif

#ifdef WIN32
#include "Win32SoundPlayer.hh"
#elif defined(HAVE_GNOME)
#include "GnomeSoundPlayer.hh"
#endif

#ifdef HAVE_GNOME
#include "AppletWindow.hh"
#include "RemoteControl.hh"
#endif

GUI *GUI::instance = NULL;


//! GUI Constructor.
/*!
 *  \param controller interface to the controller.
 *  \param argc number of command line parameters.
 *  \param argv all command line parameters.
 */
GUI::GUI(ControlInterface *controller, int argc, char **argv)  :
  configurator(NULL),
  core_control(controller),
#ifdef HAVE_GNOME
  applet_window(NULL),
#endif
  main_window(NULL)
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
  
  if (gui_control != NULL)
    {
      delete gui_control;
    }

  if (main_window != NULL)
    {
      delete main_window;
    }

#ifdef GNOME  
  if (applet_window != NULL)
    {
      delete applet_window;
    }
#endif  
  TRACE_EXIT();
}


void
GUI::restbreak_now()
{
  gui_control->break_action(GUIControl::BREAK_ID_REST_BREAK,
                            GUIControl::BREAK_ACTION_FORCE_START_BREAK);
}


//! The main entry point.
void
GUI::main()
{
  TRACE_ENTER("GUI::main");

  Gtk::Main kit(argc, argv);

  init_debug();
  init_nls();
  init_core_control();
  init_gui_control();
  init_gui();
  init_remote_control();
  
  // Enter the event loop
  gdk_threads_enter();
  Gtk::Main::run();
  gdk_threads_leave();

  delete main_window;
  main_window = NULL;
  
#ifdef HAVE_GNOME
  delete applet_window;
  applet_window = NULL;
#endif  
  
  TRACE_EXIT();
}


//! Terminates the GUI.
void
GUI::terminate()
{
  TRACE_ENTER("GUI::terminate");
  Gtk::Main::quit();
  TRACE_EXIT();
}


//! Opens the main window.
void
GUI::open_main_window()
{
  if (main_window != NULL)
    {
      main_window->deiconify();
      main_window->raise();
    }
}


//! Periodic heartbeat.
bool
GUI::on_timer()
{
  if (gui_control != NULL)
    {
      gui_control->heartbeat();
    }
  
  if (main_window != NULL)
    {
      main_window->update();
    }
  
#ifdef HAVE_GNOME
  if (applet_window != NULL)
    {
      applet_window->update();
    }
#endif

  return true;
}


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
GUI::init_core_control()
{
#ifdef HAVE_X
  char *display_name = gdk_get_display();
#else
  char *display_name = NULL;
#endif
  Configurator *config = create_configurator();
  core_control->init(GUIControl::BREAK_ID_SIZEOF, config, display_name);

#ifdef HAVE_X
  g_free(display_name);
#endif
}


//! Initializes the GUI Controller.
void
GUI::init_gui_control()
{
  gui_control = new GUIControl(this, core_control);
  gui_control->init();
}



//! Initializes the GUI
void
GUI::init_gui()
{
  // Setup the window hints module.
  WindowHints::init();

  // The main status window.
  main_window = new MainWindow();
  
#ifdef HAVE_GNOME
  // The applet window.
  applet_window = new AppletWindow();
#endif  

  tooltips = manage(new Gtk::Tooltips());
  tooltips->enable();

  // Periodic timer.
  Glib::signal_timeout().connect(SigC::slot(*this, &GUI::on_timer), 1000);
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


//! Returns a prelude window.
PreludeWindowInterface *
GUI::create_prelude_window()
{
  return new PreludeWindow();
}


//! Returns a break window for the specified break.
BreakWindowInterface *
GUI::create_break_window(GUIControl::BreakId break_id, bool ignorable)
{
  BreakWindowInterface *ret = NULL;
  
  if (break_id == GUIControl::BREAK_ID_MICRO_PAUSE)
    {
      ret = new MicroPauseWindow(GUIControl::get_instance()->timers[GUIControl::BREAK_ID_REST_BREAK].timer,
                                 ignorable);
    }
  else if (break_id == GUIControl::BREAK_ID_REST_BREAK)
    {
      ret = new RestBreakWindow(ignorable); 
    }
  else if (break_id == GUIControl::BREAK_ID_DAILY_LIMIT)
    {
      ret = new DailyLimitWindow(ignorable);
    }

  return ret;
}


//! Returns a sound player object.
SoundPlayerInterface *
GUI::create_sound_player()
{
  TRACE_ENTER("GUI::create_sound_player");
  SoundPlayerInterface *snd = NULL;
#if defined(WIN32)
  snd = new Win32SoundPlayer();
#elif defined(HAVE_GNOME)
  snd = new GnomeSoundPlayer(); // FIXME: LEAK
#else
#  warning Sound card support disabled.
#endif
  TRACE_EXIT();
  return snd;
}


//! Returns the configurator.
Configurator *
GUI::create_configurator()
{
  if (configurator == NULL)
    {
#if defined(HAVE_REGISTRY)
      configurator = Configurator::create("w32");
#elif defined(HAVE_GCONF)
      gconf_init(argc, argv, NULL);
      g_type_init();
      configurator = Configurator::create("gconf");
#elif defined(HAVE_GDOME)
      string configFile = Util::complete_directory("config.xml", Util::SEARCH_PATH_CONFIG);

      configurator = Configurator::create("xml");
#if defined(HAVE_X)
      if (configFile == "" || configFile == "config.xml")
        {
          configFile = Util::get_home_directory() + "config.xml";
        }
#endif
      if (configFile != "")
        {
          configurator->load(configFile);
        }
#else
#error No configuator configured        
#endif
    }
  return configurator;
}

#ifdef NDEBUG
static void my_log_handler(const gchar *log_domain, GLogLevelFlags log_level,
                           const gchar *message, gpointer user_data)
{
}
#endif

