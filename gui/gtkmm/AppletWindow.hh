// AppletWindow.hh --- Main info Window
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

#ifndef APPLETWINDOW_HH
#define APPLETWINDOW_HH

#include "preinclude.h"
#include <stdio.h>

#include <gnome.h>
#include <bonobo.h>
#include <bonobo/bonobo-xobject.h>
#include "Workrave-Applet.h"

class GUI;
class ControlInterface;
class TimeBar;
class NetworkLogDialog;

#include <gtkmm.h>
#include <gtkmm/plug.h>

#include "GUIControl.hh"

using namespace std;

class AppletWindow :
  public Gtk::HBox
{
public:  
  AppletWindow(GUI *gui, ControlInterface *controller);
  ~AppletWindow();

  void update();

  void on_menu_restbreak_now();

public:  
  static const string CFG_KEY_APPLET;
  static const string CFG_KEY_APPLET_HORIZONTAL;
  static const string CFG_KEY_APPLET_SHOW_MICRO_PAUSE;
  static const string CFG_KEY_APPLET_SHOW_REST_BREAK;
  static const string CFG_KEY_APPLET_SHOW_DAILY_LIMIT;
  
private:
  //! The controller that maintains the data and control over the breaks
  ControlInterface *core_control;

  //! Interface to the GUI.
  GUI *gui;

  //! Table containing all timer information
  Gtk::Table *timers_box;

  //! Array of timer name labels
  Gtk::Widget **timer_names;

  //! Araay of timer value widgets.
  TimeBar **timer_times;

  //! Breaks to show in applet.
  bool show_break[GUIControl::BREAK_ID_SIZEOF];

  //! Allign break horizontally.
  bool horizontal;
  
private:
  //
  void init();
  void init_widgets();
  void init_applet();
  void init_table();
  bool init_native_applet();
  void read_configuration();

  // Events.
  bool on_delete_event(GdkEventAny*);
};

#endif // APPLETWINDOW_HH
