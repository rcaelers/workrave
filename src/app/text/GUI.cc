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

#include <unistd.h>

#include "debug.hh"
#include "GUI.hh"

#include "Configurator.hh"
#include "ControlInterface.hh"
#include "DailyLimitWindow.hh"
#include "GUIControl.hh"
#include "MicroPauseWindow.hh"
#include "PreludeWindow.hh"
#include "RestBreakWindow.hh"

#ifdef HAVE_GCONF
#include <gconf/gconf-client.h>
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
  core_control(controller)
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

  TRACE_EXIT();
}


//! The main entry point.
void
GUI::main()
{
  TRACE_ENTER("GUI::main");

  char *display_name = NULL;
  Configurator *config = create_configurator();
  core_control->init(config, display_name);

  gui_control = new GUIControl(this, core_control);
  gui_control->init();

  int count = 0;
  while (count < 4000)
    {
#if defined(WIN32)
      Sleep(1000);
#else
      sleep(1);
#endif
      if (gui_control != NULL)
        {
          gui_control->heartbeat();
        }

      count++;
    }
  TRACE_EXIT();
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
  SoundPlayerInterface *snd = NULL;
#if defined(WIN32)
  //snd = new Win32SoundPlayer();
#elif defined(HAVE_GNOME)
  snd = new GnomeSoundPlayer();
#else
#  warning Sound card support disabled.
#endif
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
