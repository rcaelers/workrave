// GUI.cc --- The WorkRave GUI
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
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

#include "debug.hh"
#include <sstream>

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>

#include "Util.hh"

#include "GUI.hh"
#include "GUIControl.hh"
#include "Configurator.hh"
#include "ControlInterface.hh"
#include "TimerInterface.hh"

#include "BreakControl.hh"
#include "MainWindow.hh"
#include "PreludeWindow.hh"
#include "MicroPauseWindow.hh"
#include "RestBreakWindow.hh"
#include "WindowHints.hh"

GUI *GUI::instance = NULL;

//! GUI Constructor.
/*!
 *  \param controller interface to the controller.
 *  \param argc number of command line parameters.
 *  \param argv all command line parameters.
 */
GUI::GUI(ControlInterface *controller, int argc, char **argv)
{
  TRACE_ENTER("GUI:GUI");

  assert(! instance);
  instance = this;
  
  main_window = NULL;
  core_control = controller;
  this->argc = argc;
  this->argv = argv;

  gui_control = new GUIControl(this, controller);
  
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


void
GUI::restbreak_now()
{
  gui_control->break_action(GUIControl::BREAK_ID_REST_BREAK,
                            GUIControl::BREAK_ACTION_FORCE_START_BREAK);
}

void
GUI::set_operation_mode(GUIControl::OperationMode mode)
{
  assert(gui_control != NULL);
  gui_control->set_operation_mode(mode);
}


//! GUI Thread.
void
GUI::run()
{
  TRACE_ENTER("GUI:run");

  // Initialize Gtkmm
  Gtk::Main kit(argc, argv);

  // Setup the window hints module.
  WindowHints::init();

  // Initialize the core.
  core_control->init();
  
  // Create all windows.
  gui_control->init();

  // The main status window.
  main_window = new MainWindow(this, core_control);

  // Periodic timer.
  Glib::signal_timeout().connect(SigC::slot(*this, &GUI::on_timer), 1000);
  
  // Enter the event loop
  Gtk::Main::run(*main_window);
  TRACE_MSG("end of Gtk::Main::run");
    
  delete main_window;
 
  TRACE_EXIT();
}


void
GUI::terminate()
{
  TRACE_ENTER("GUI::terminate");
  Gtk::Main::quit();
  TRACE_EXIT();
}


//! Notication of a timer action.
/*!
 *  \param timerId ID of the timer that caused the action.
 *  \param action action that is performed by the timer.
*/
void
GUI::timer_action(string timer_id, TimerEvent event)
{
  TRACE_ENTER_MSG("GUI::timer_action", timer_id << " " << event);

  GUIControl::BreakId id = GUIControl::BREAK_ID_NONE;
  GUIControl::BreakAction act = GUIControl::BREAK_ACTION_NONE;

  // Parse timer_id
  if (timer_id == "micro_pause")
    {
      id = GUIControl::BREAK_ID_MICRO_PAUSE;
    }
  else if (timer_id == "rest_break")
    {
      id = GUIControl::BREAK_ID_REST_BREAK;
    }
  else if (timer_id == "daily_limit")
    {
      id = GUIControl::BREAK_ID_DAILY_LIMIT;
    }

  if (id != GUIControl::BREAK_ID_NONE)
    {
      // Parse action.
      if (event == TIMER_EVENT_LIMIT_REACHED)
        {
          gui_control->break_action(id, GUIControl::BREAK_ACTION_START_BREAK);
        }
      else if (event == TIMER_EVENT_RESET)
        {
          gui_control->break_action(id, GUIControl::BREAK_ACTION_STOP_BREAK);
        }
      else if (event == TIMER_EVENT_NATURAL_RESET)
        {
          gui_control->break_action(id, GUIControl::BREAK_ACTION_NATURAL_STOP_BREAK);
        }
    }

  TRACE_EXIT();
}


//! Update the main window screen.
void
GUI::heartbeat()
{
  map<string, TimerInfo> infos;
  core_control->process_timers(infos);
  for (map<string, TimerInfo>::iterator i = infos.begin(); i != infos.end(); i++)
    {
      string id = i->first;
      TimerInfo &info = i->second;

      timer_action(id, info.event);
    }

  if (main_window != NULL)
    {
      main_window->update();
    }

  if (gui_control != NULL)
    {
      gui_control->heartbeat();
    }
}


//! Returns the configurator.
Configurator *
GUI::get_configurator()
{
  return gui_control->get_configurator();
}


PreludeWindowInterface *
GUI::create_prelude_window()
{
  return new PreludeWindow();
}


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

  return ret;
}


bool
GUI::on_timer()
{
  heartbeat();
  return true;
}
