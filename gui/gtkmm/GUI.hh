// GUI.hh --- The WorkRave GUI
//
// Copyright (C) 2001, 2002 Rob Caelers & Raymond Penners
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
// $Id$
//

#ifndef GUI_HH
#define GUI_HH

#include "preinclude.h"

#include <sigc++/object.h>
#include <glibmm.h>

#include "Mutex.hh"
#include "GUIInterface.hh"
#include "GUIFactoryInterface.hh"
#include "TimerInterface.hh"

// GTKMM classes
class MainWindow;
class MicroPauseWindow;
class RestBreakWindow;
class AppletWindow;

// Generic GUI
class GUIControl;
class BreakControl;
class SoundPlayerInterface;

// Core interfaces
class BreakInterface;
class ControlInterface;

class GUI :
  public GUIInterface,
  public GUIFactoryInterface,
  public SigC::Object
{
public:
  //! Destructor
  virtual ~GUI();
  
public:
  //! Constructor.
  GUI(ControlInterface *controller, int argc, char **argv);
  static GUI *get_instance();

  // GUIInterface methods
  void start();
  void run();
  Configurator *get_configurator();

  // GUIFactoryInterface methods
  PreludeWindowInterface *create_prelude_window();
  BreakWindowInterface *create_break_window(GUIControl::BreakId break_id, bool ignorable);
  SoundPlayerInterface *create_sound_player();
  
  // Internal public methods
  void restbreak_now();
  void set_operation_mode(GUIControl::OperationMode mode);
  void terminate();
  
private:
  void timer_action(string timer_id, TimerEvent event);
  void heartbeat();
  bool inter_thread_call(Glib::IOCondition ioCond);
  bool on_timer();
  
private:
  //! The one and only instance
  static GUI *instance;

  //! The Core controller
  ControlInterface *core_control;

  // The generic GUI controller.
  GUIControl *gui_control;
  
  //! The number of command line arguments.
  int argc;

  //! The command line arguments.
  char **argv;

  //! The main window, shows the timers
  MainWindow *main_window;

#ifdef HAVE_GNOME
  AppletWindow *applet_window;
#endif  
};

inline GUI *
GUI::get_instance()
{
  return instance;
}

#endif // GUI_HH
