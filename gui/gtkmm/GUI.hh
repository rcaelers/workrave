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

#include "preinclude.h"

#include <sigc++/object.h>
#include <glibmm.h>

#include "Mutex.hh"
#include "GUIInterface.hh"
#include "GUIFactoryInterface.hh"
#include <gtkmm.h>

#ifdef HAVE_GNOME
#include <libgnomeuimm.h>
#endif

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
  void main();

  // GUIFactoryInterface methods
  PreludeWindowInterface *create_prelude_window();
  BreakWindowInterface *create_break_window(GUIControl::BreakId break_id, bool ignorable);
  SoundPlayerInterface *create_sound_player();
  Configurator *create_configurator();
  
  // Internal public methods
  void restbreak_now();
  void open_main_window();
  void close_main_window();
  void toggle_main_window();
  void terminate();

#ifdef HAVE_X  
  AppletWindow *get_applet_window() const;
#endif  
  MainWindow *get_main_window() const;
  Gtk::Tooltips *get_tooltips() const;
  
private:
  bool on_timer();
  void init_debug();
  void init_nls();
  void init_core_control();
  void init_gui_control();
  void init_gui();
  void init_remote_control();

#ifdef HAVE_GNOME
  void init_gnome();
  void on_die();
  bool on_save_yourself(int phase, Gnome::UI::SaveStyle save_style, bool shutdown,
                        Gnome::UI::InteractStyle interact_style, bool fast);
#endif
  
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

#ifdef HAVE_X  
  //! The applet window.
  AppletWindow *applet_window;
#endif  

  //! The main window, shows the timers.
  MainWindow *main_window;

  /** */
  Gtk::Tooltips *tooltips;
};


//! Returns the only instance of GUI
inline GUI *
GUI::get_instance() 
{
  return instance;
}

//! Returns tooltips
inline Gtk::Tooltips *
GUI::get_tooltips() const
{
  return tooltips;
}


#ifdef HAVE_X
//! Returns the applet window.
inline AppletWindow *
GUI::get_applet_window() const
{
  return applet_window;
}
#endif

//! Returns the main window.
inline MainWindow *
GUI::get_main_window() const
{
  return main_window;
}

#endif // GUI_HH
