// TimerWindow.hh --- Main info Window
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

#ifndef TIMERWINDOW_HH
#define TIMERWINDOW_HH

#include "preinclude.h"
#include <stdio.h>

#ifdef HAVE_GNOME
#include <gnome.h>
#endif

class GUI;
class TimeBar;
class NetworkLogDialog;

#include <gtkmm.h>

using namespace std;

class TimerWindow
{
public:  
  TimerWindow();
  ~TimerWindow();

  void init_widgets();
  void update_widgets();

protected:
  virtual bool on_delete_event(GdkEventAny *) = 0;
  
protected:
  //! Array of timer name labels
  Gtk::Widget **timer_names;

  //! Araay of timer value widgets.
  TimeBar **timer_times;
};

#endif // TIMERWINDOW_HH
