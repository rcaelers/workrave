// StatusIcon.cc --- Status icon
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

static const char rcsid[] = "$Id$";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <assert.h>

#include "MainWindow.hh"
#include "CoreFactory.hh"
#include "StatusIcon.hh"
#include "Util.hh"

StatusIcon::StatusIcon(MainWindow& mw)
  : main_window(mw)
{
  // Preload icons
  const char *mode_files[] =
    {
      "workrave-icon-large.png",
      "workrave-suspended-icon-medium.png",
      "workrave-quiet-icon-medium.png",
    };
  assert(sizeof(mode_files)/sizeof(mode_files[0]) == OPERATION_MODE_SIZEOF);
  for (size_t i = 0; i < OPERATION_MODE_SIZEOF; i++)
    {
      std::string file = Util::complete_directory(mode_files[i],
                                                  Util::SEARCH_PATH_IMAGES);
      mode_icons[i] = Gdk::Pixbuf::create_from_file(file);
    }
  

  // Create status icon
  CoreInterface *core = CoreFactory::get_core();
  OperationMode mode = core->get_operation_mode();      
  status_icon = Gtk::StatusIcon::create(mode_icons[mode]);

  // Hook up signals, missing from gtkmm
  GtkStatusIcon *gobj = status_icon->gobj();

  g_signal_connect(gobj, "activate",
                   reinterpret_cast<GCallback>(activate_callback), this);
  g_signal_connect(gobj, "popup-menu",
                   reinterpret_cast<GCallback>(popup_menu_callback), this);
}

StatusIcon::~StatusIcon()
{
}

void StatusIcon::set_operation_mode(OperationMode m)
{
  status_icon->set(mode_icons[m]);
}

void StatusIcon::on_activate()
{
  main_window.on_activate();
}

void StatusIcon::on_popup_menu(guint button, guint activate_time)
{
  main_window.on_popup_menu(button, activate_time);
}

void StatusIcon::activate_callback(GtkStatusIcon *,
                                   gpointer callback_data)
{
  static_cast<StatusIcon*>(callback_data)->on_activate();
}

void StatusIcon::popup_menu_callback(GtkStatusIcon *,
                                     guint button,
                                     guint activate_time,
                                     gpointer callback_data)
{
  static_cast<StatusIcon*>(callback_data)->on_popup_menu(button, activate_time);
}

void StatusIcon::set_timers_tooltip(std::string& tip)
{
  status_icon->set_tooltip(tip);
}

