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

class GUI;
class ControlInterface;
class TimeBar;
class NetworkLogDialog;

#include <gtkmm.h>
#include <gtkmm/plug.h>

using namespace std;

class AppletWindow :
  public Gtk::HBox
{
public:  
  AppletWindow(GUI *gui, ControlInterface *controller);
  ~AppletWindow();

  void update();

private:
  //! The controller that maintains the data and control over the breaks
  ControlInterface *core_control;

  //! Interface to the GUI.
  GUI *gui;

  //! Araay of timer value widgets.
  TimeBar *bar;

private:
  //
  void init();

  // Events.
  bool on_delete_event(GdkEventAny*);
};

#endif // APPLETWINDOW_HH
