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
#include "Workrave-Control.h"

class GUI;
class ControlInterface;
class TimeBar;
class NetworkLogDialog;

#include <gtkmm.h>
#include <gtkmm/plug.h>

#include "GUIControl.hh"
#include "TimerWindow.hh"

using namespace std;

class AppletWindow :
  public TimerWindow
{
public:  
  enum AppletMode { APPLET_DISABLED, APPLET_TRAY, APPLET_GNOME };
                    
  AppletWindow(GUI *gui, ControlInterface *controller);
  ~AppletWindow();

  void update();
  void fire();
  
  void on_menu_restbreak_now();
  void set_menu_active(int menu, bool active);
  bool get_menu_active(int menu);
  
public:  
  static const string CFG_KEY_APPLET;
  static const string CFG_KEY_APPLET_HORIZONTAL;
  static const string CFG_KEY_APPLET_ENABLED;
  static const string CFG_KEY_APPLET_SHOW_MICRO_PAUSE;
  static const string CFG_KEY_APPLET_SHOW_REST_BREAK;
  static const string CFG_KEY_APPLET_SHOW_DAILY_LIMIT;
  
private:
  //! Current Applet mode.
  AppletMode mode;

  //! Retry init panel again?
  bool retry_init;

  //!
  Gtk::Plug *plug;
  
  //! Table containing all timer information
  Gtk::Table *timers_box;

  //!
  Gtk::Menu *tray_menu;

  Gtk::EventBox *eventbox;
  
  //! Breaks to show in applet.
  bool show_break[GUIControl::BREAK_ID_SIZEOF];

  //! Allign break horizontally.
  bool horizontal;

  //! Applet enabled?
  bool applet_enabled;

  //
  GNOME_Workrave_AppletControl applet_control;
  
private:
  //
  void init();
  void init_applet();
  void init_table();
  bool init_tray_applet();
  bool init_gnome_applet();
  void destroy_applet();
  void destroy_tray_applet();
  void destroy_gnome_applet();
  void read_configuration();


  bool on_button_press_event(GdkEventButton *event);
  bool delete_event(GdkEventAny *event);

  // Events.
  bool on_delete_event(GdkEventAny*);
};

#endif // APPLETWINDOW_HH
