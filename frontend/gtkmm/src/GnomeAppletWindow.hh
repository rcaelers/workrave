// GnomeAppletWindow.hh --- X11 Applet Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Rob Caelers & Raymond Penners
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

#ifndef GNOMEAPPLETWINDOW_HH
#define GNOMEAPPLETWINDOW_HH

#include "preinclude.h"
#include <stdio.h>

#include "AppletWindow.hh"

#ifdef HAVE_GTKMM24
#include <sigc++/compatibility.h>
#endif

#include <gnome.h>
#include <bonobo.h>
#include <bonobo/bonobo-xobject.h>
#include <string>

#include "Workrave-Applet.h"
#include "Workrave-Control.h"

#include <gtkmm/bin.h>
#include <gtkmm/menu.h>
#include <gtkmm/plug.h>
#include <gtkmm/eventbox.h>

class TimerBoxGtkView;
class AppletControl;

class GnomeAppletWindow :
  public SigC::Object,
  public AppletWindow
{
public:  
  GnomeAppletWindow(AppletControl *control);
  virtual ~GnomeAppletWindow();

  void update();
  void fire_gnome_applet();

  void on_menu_restbreak_now();
  void button_clicked(int button);

  void set_menu_active(int menu, bool active);
  bool get_menu_active(int menu);
  void set_applet_vertical(bool vertical);
  void set_applet_size(int size);
  void set_applet_control(GNOME_Workrave_AppletControl applet_control);
  void set_applet_background(int type, GdkColor &color, long xid);

private:
  //! Gtk timerbox viewer
  TimerBoxGtkView *view;
   
  //! The Gtk+ plug in the panel.
  Gtk::Plug *plug;

  //! Container to put the timers in..
  Gtk::Bin *container;
  
  //! The system tray menu.
  Gtk::Menu *tray_menu;

  // 
  GNOME_Workrave_AppletControl applet_control;

  //! Allign break vertically.
  bool applet_vertical;

  //! Size of the applet
  int applet_size;

  //!
  AppletControl *control;

  //!
  bool applet_active;
  
private:
  void init_applet();
  void cleanup_applet();
  void deactivate_applet();
  AppletState activate_applet();
    
  bool init_gnome_applet();
  void destroy_gnome_applet();
  void setbackground(int type,
                     GtkRcStyle * rc_style,
                     GtkWidget * w, 
                     GdkColor * color,
                     GdkPixmap * pixmap);

  static gboolean destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);

  // Events.
  void on_embedded();
  bool on_button_press_event(GdkEventButton *event);
  bool on_delete_event(GdkEventAny*);
  bool delete_event(GdkEventAny *event);
};

#endif // GNOMEAPPLETWINDOW_HH
