// BreakWindow.hh --- base class for the break windows
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

#ifndef BREAKWINDOW_HH
#define BREAKWINDOW_HH

#include <stdio.h>

#include "preinclude.h"

#include <gtkmm.h>

#include "WindowHints.hh"

#ifdef HAVE_GTK_MULTIHEAD
#include <gdkmm/display.h>
#include <gdkmm/screen.h>
#define MULTIHEAD_PARAMS , Glib::RefPtr<Gdk::Screen> screen, int monitor
#define MULTIHEAD_ARGS , screen, monitor
#endif


class Frame;

class BreakWindow :
  public Gtk::Window
{
public:
  BreakWindow();
  virtual ~BreakWindow();

  static Gtk::Button *create_skip_button();
  static Gtk::Button *create_postpone_button();
  
protected:
  bool grab();
  void ungrab();
  void center();
  void add(Gtk::Widget& widget);
  void set_border_width(guint border_width);
  void set_avoid_pointer(bool avoid_pointer);
  bool did_avoid_pointer() const;

#ifdef HAVE_GTK_MULTIHEAD
  void set_screen(Glib::RefPtr<Gdk::Screen> screen, int monitor);
#endif  
  
  const int SCREEN_MARGIN;

private:
#ifdef WIN32
  bool on_avoid_pointer_timer();
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
  //! Avoid time signal
  SigC::Connection avoid_signal;
#endif

  //! Do we want a to avoid pointer?
  bool avoid_wanted;

  //! Did we avoid the pointer?
  bool did_avoid;
  
  //! Grab
  WindowHints::Grab *grab_handle;

  //! Frame
  Frame *frame;

  //! Border
  guint border_width;

#ifdef HAVE_GTK_MULTIHEAD
  Glib::RefPtr<Gdk::Screen> screen;
  int monitor;
#endif  
};


inline bool
BreakWindow::did_avoid_pointer() const
{
  return did_avoid;
}

#endif // RESTBREAKWINDOW_HH
