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

#ifdef HAVE_GNOME
#include <gnome.h>
#include <bonobo.h>
#include <bonobo/bonobo-xobject.h>
#include "Workrave-Applet.h"
#include "Workrave-Control.h"
#endif

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
  public TimerWindow,
  public ConfiguratorListener
{
public:  
  enum AppletMode { APPLET_DISABLED, APPLET_TRAY, APPLET_GNOME };

  AppletWindow(GUI *gui, ControlInterface *controller);
  ~AppletWindow();

  void update();
  void fire();
  
  void on_menu_restbreak_now();
#ifdef HAVE_GNOME
  void set_menu_active(int menu, bool active);
  bool get_menu_active(int menu);
  void set_applet_vertical(bool vertical);
  void set_applet_size(int size);
#endif

  void config_changed_notify(string key);

  
public:  
  static const string CFG_KEY_APPLET;
  static const string CFG_KEY_APPLET_HORIZONTAL;
  static const string CFG_KEY_APPLET_ENABLED;
  static const string CFG_KEY_APPLET_CYCLE_TIME;
  static const string CFG_KEY_APPLET_POSITION;
  static const string CFG_KEY_APPLET_FLAGS;
  static const string CFG_KEY_APPLET_IMMINENT;

  enum SlotType
    {
      BREAK_WHEN_IMMINENT = 1,
      BREAK_WHEN_FIRST = 2,
      BREAK_SKIP = 4,
      BREAK_EXCLUSIVE = 8,
      BREAK_DEFAULT = 16
    };
  
private:
      
  //! Current Applet mode.
  AppletMode mode;

  //! Retry init panel again?
  bool retry_init;

  //!
  Gtk::Plug *plug;

  //! Container for applet widget.
  Gtk::Bin *container;
  
  //! Table containing all timer information
  Gtk::Table *timers_box;

  //!
  Gtk::Menu *tray_menu;

  Gtk::EventBox *eventbox;
  Gtk::Alignment *frame;

  //!
  int number_of_timers;
  
  //! Position for the break timer.
  int break_position[GUIControl::BREAK_ID_SIZEOF];

  //! Flags for the break timer.
  int break_flags[GUIControl::BREAK_ID_SIZEOF];

  //! Flags for the break timer.
  int break_imminent_time[GUIControl::BREAK_ID_SIZEOF];
  
  //!
  int break_slots[GUIControl::BREAK_ID_SIZEOF][GUIControl::BREAK_ID_SIZEOF];

  //!
  int break_slot_cycle[GUIControl::BREAK_ID_SIZEOF];

  //!
  int cycle_time;
  
  //! Allign break vertically.
  bool applet_vertical;

  //!
  int applet_size;

  //!
  int applet_width;
  
  //!
  int applet_height;

  //! Applet enabled?
  bool applet_enabled;

#ifdef HAVE_GNOME
  //
  GNOME_Workrave_AppletControl applet_control;
#endif
  
  //
  bool reconfigure;
  
private:
  //
  void init();
  void init_applet();
  void init_table();
  bool init_tray_applet();

#ifdef HAVE_GNOME
  bool init_gnome_applet();
  void destroy_gnome_applet();
#endif

  void init_slot(int slot);
  void cycle_slots();
  
  void destroy_applet();
  void destroy_tray_applet();
  void read_configuration();
  
  bool on_button_press_event(GdkEventButton *event);
  bool delete_event(GdkEventAny *event);

  // Events.
  bool on_delete_event(GdkEventAny*);
};

#endif // APPLETWINDOW_HH
