// BreakWindow.hh --- base class for the break windows
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

#ifndef BREAKWINDOW_HH
#define BREAKWINDOW_HH

#include <stdio.h>

#include "preinclude.h"

#include <gtkmm.h>

#include "BreakWindowInterface.hh"
#include "WindowHints.hh"
#ifdef WIN32
#include "InputMonitorListenerInterface.hh"
#endif

#ifdef WIN32
class InputMonitor;
#endif

class Frame;

class BreakWindow :
  public Gtk::Window
#ifdef WIN32
  , public InputMonitorListenerInterface
#endif
{
public:
  BreakWindow();
  virtual ~BreakWindow();

protected:
  bool grab();
  void ungrab();
  void center();
  void add(Gtk::Widget& widget);
  void set_border_width(guint border_width);
  void set_avoid_pointer(bool avoid_pointer);
  
private:
#ifdef WIN32
  void action_notify();
  void mouse_notify(int x, int y, int wheel);
#else
  bool on_grab_retry_timer();
  bool on_enter_notify_event(GdkEventCrossing* event);
#endif
  void avoid_pointer(int x, int y);
  
private:
#ifdef HAVE_X
  //! Do we want a keyboard/pointer grab
  bool grab_wanted;
#endif

#ifdef WIN32
  //! Input monitor
  InputMonitor *input_monitor;
#endif

  //! Do we want a to avoid pointer?
  bool avoid_wanted;
  
  //! Grab
  WindowHints::Grab *grab_handle;

  //! Frame
  Frame *frame;

  //! Border
  guint border_width;
};

#endif // RESTBREAKWINDOW_HH
