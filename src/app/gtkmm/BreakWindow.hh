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

#include <gtkmm/window.h>

#include "HeadInfo.hh"
#include "WindowHints.hh"

class Frame;
namespace Gtk
{
  class Button;
}

class BreakWindow :
  public Gtk::Window
{
public:
  BreakWindow();
  virtual ~BreakWindow();

  static Gtk::Button *create_skip_button();
  static Gtk::Button *create_postpone_button();
  Gtk::Button *create_lock_button();
  Gtk::Button *create_shutdown_button();
  
protected:
  bool grab();
  void ungrab();
  void center();

  void on_lock_button_clicked();
  void on_shutdown_button_clicked();
  void set_screen(HeadInfo &head);
  
  //! Information about the (multi)head.
  HeadInfo head;
  
private:
#if defined(HAVE_X)
  bool on_grab_retry_timer();
#endif

private:
#ifdef HAVE_X
  //! Do we want a keyboard/pointer grab
  bool grab_wanted;
#endif

  //! Grab
  WindowHints::Grab *grab_handle;
};

#endif // BREAKWINDOW_HH
