// PreferencesDialog.hh --- Preferences Dialog
//
// Copyright (C) 2002, 2003 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef PREFERENCESDIALOG_HH
#define PREFERENCESDIALOG_HH

#include <stdio.h>

#include "preinclude.h"
#include "Hig.hh"
#include "IconListNotebook.hh"
#include "CoreInterface.hh"

class TimeEntry;

#include <gtkmm.h>

class PreferencesDialog : public HigDialog
{
public:  
  PreferencesDialog();
  ~PreferencesDialog();

  int run();
  
private:
  void add_page(const char *label, const char *image, Gtk::Widget &widget);
  Gtk::Widget *create_gui_page();
  Gtk::Widget *create_timer_page();
#ifdef HAVE_DISTRIBUTION
  Gtk::Widget *create_network_page();
#endif
#ifdef HAVE_X
  Gtk::Widget *create_applet_page();
#endif
  Gtk::Widget *create_mainwindow_page();
  bool on_focus_in_event(GdkEventFocus *event);
  bool on_focus_out_event(GdkEventFocus *event);  
  
  void on_sound_changed();
  
  Gtk::OptionMenu *sound_button;

  // Mode before focus in.
  OperationMode mode;
  IconListNotebook notebook;
};

#endif // PREFERENCESWINDOW_HH
