// GUI.hh --- The WorkRave GUI
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
// $Id$
//

#ifndef GUI_HH
#define GUI_HH

#include "GUIInterface.hh"
#include "GUIFactoryInterface.hh"

// Generic GUI
class GUIControl;
class BreakControl;
class SoundPlayerInterface;

// Core interfaces
class BreakInterface;
class ControlInterface;

class GUI :
  public GUIInterface,
  public GUIFactoryInterface
{
public:
  //! Destructor
  virtual ~GUI();
  
public:
  //! Constructor.
  GUI(ControlInterface *controller, int argc, char **argv);
  static GUI *get_instance();

  // GUIInterface methods
  void main();

  // GUIFactoryInterface methods
  PreludeWindowInterface *create_prelude_window();
  BreakWindowInterface *create_break_window(GUIControl::BreakId break_id, bool ignorable);
  SoundPlayerInterface *create_sound_player();
  Configurator *create_configurator();
  
private:
  //! The one and only instance
  static GUI *instance;

  //! The Configurator.
  Configurator *configurator;

  //! The Core controller
  ControlInterface *core_control;

  // The generic GUI controller.
  GUIControl *gui_control;
  
  //! The number of command line arguments.
  int argc;

  //! The command line arguments.
  char **argv;
};


//! Returns the only instance of GUI
inline GUI *
GUI::get_instance()
{
  return instance;
}

#endif // GUI_HH
