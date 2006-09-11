// StatusIcon.hh --- Status icon
//
// Copyright (C) 2006 Rob Caelers & Raymond Penners
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

#ifndef STATUSICON_HH
#define STATUSICON_HH

#include "preinclude.h"

#include <gtkmm/statusicon.h>
#include "CoreInterface.hh"

class MainWindow;

class StatusIcon 
{
public:  
  StatusIcon(MainWindow& mw);
  ~StatusIcon();

  void set_operation_mode(OperationMode m);
  void set_timers_tooltip(std::string& tip);
  
private:
  void on_activate();
  void on_popup_menu(guint button, guint activate_time);

  static void activate_callback(GtkStatusIcon *si, gpointer callback_data);
  static void popup_menu_callback(GtkStatusIcon *si, guint button, guint activate_time,
                                  gpointer callback_data);

  Glib::RefPtr<Gtk::StatusIcon> status_icon;
  MainWindow& main_window;
  Glib::RefPtr<Gdk::Pixbuf> mode_icons[OPERATION_MODE_SIZEOF];
};


#endif // STATUSICON_HH
