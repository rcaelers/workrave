// X11AppletWindow.hh --- X11 Applet Window
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

#ifndef X11APPLETWINDOW_HH
#define X11APPLETWINDOW_HH

#include "preinclude.h"
#include <stdio.h>

#ifdef HAVE_GTKMM24
#include <sigc++/compatibility.h>
#endif

#include "ConfiguratorListener.hh"

#ifdef HAVE_GNOME
#include <gnome.h>
#include <bonobo.h>
#include <bonobo/bonobo-xobject.h>
#include <string>

#include "Workrave-Applet.h"
#include "Workrave-Control.h"
#endif

#include <gtkmm/bin.h>
#include <gtkmm/menu.h>
#include <gtkmm/plug.h>
#include <gtkmm/eventbox.h>

class TimerBoxControl;
class TimerBoxGtkView;

class X11AppletWindow :
  public ConfiguratorListener,
  public SigC::Object
{
public:  
  X11AppletWindow();
  virtual ~X11AppletWindow();

  void update();
  void fire_gnome_applet();
  void fire_kde_applet();

  AppletMode get_applet_mode() const;
  
  void on_menu_restbreak_now();
  void button_clicked(int button);
#ifdef HAVE_GNOME
  void set_menu_active(int menu, bool active);
  bool get_menu_active(int menu);
  void set_applet_vertical(bool vertical);
  void set_applet_size(int size);
  void set_applet_control(GNOME_Workrave_AppletControl applet_control);
  void set_applet_background(int type, GdkColor &color, long xid);
#endif

  void config_changed_notify(std::string key);
  void read_configuration();

private:
  //! Current applet mode.
  AppletMode mode;

  //! The Gtk+ plug in the panel.
  Gtk::Plug *plug;

  //! Container to put the timers in..
  Gtk::Bin *container;
  
  //! The system tray menu.
  Gtk::Menu *tray_menu;

#ifdef HAVE_GNOME
  // 
  GNOME_Workrave_AppletControl applet_control;
#endif

  //! Retry to initialize the panel again?
  bool retry_init;

  //! Allign break vertically.
  bool applet_vertical;

  //! Size of the applet
  int applet_size;

  //! Applet enabled?
  bool applet_enabled;

#ifdef HAVE_KDE
#ifdef HAVE_GTKMM24
  Gtk::Requisition last_size;
#else
  GtkRequisition last_size;
#endif
#endif
  
private:
  void init();
  void init_applet();
  bool init_tray_applet();
  void destroy_applet();
  void destroy_tray_applet();
  void set_mainwindow_applet_active(bool a);
    
#ifdef HAVE_GNOME
  bool init_gnome_applet();
  void destroy_gnome_applet();
  void setbackground(int type,
                     GtkRcStyle * rc_style,
                     GtkWidget * w, 
                     GdkColor * color,
                     GdkPixmap * pixmap);
#endif

#if defined(HAVE_GNOME) || defined(HAVE_KDE)
  static gboolean destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
#endif
  
#ifdef HAVE_KDE
  bool init_kde_applet();
  void destroy_kde_applet();
#endif
  // Events.
  void on_embedded();
  bool on_button_press_event(GdkEventButton *event);
  bool on_delete_event(GdkEventAny*);
  bool delete_event(GdkEventAny *event);
};

#endif // X11APPLETWINDOW_HH
