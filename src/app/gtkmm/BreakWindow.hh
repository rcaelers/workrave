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

#include "CoreInterface.hh"
#include "BreakWindowInterface.hh"
#include "HeadInfo.hh"
#include "WindowHints.hh"
#include "GUI.hh"

class BreakResponseInterface;

namespace Gtk
{
  class Button;
  class HButtonBox;
}

class BreakWindow :
  public Gtk::Window,
  public BreakWindowInterface
{
public:
  BreakWindow(BreakId break_id, HeadInfo &head, bool ignorable,
              GUI::BlockMode block_mode);
  virtual ~BreakWindow();

  void set_response(BreakResponseInterface *bri);

  virtual void start();
  virtual void stop();
  virtual void destroy();
  void refresh();
  
protected:
  virtual Gtk::Widget *create_gui() = 0;
  void init_gui();
  
  bool grab();
  void ungrab();
  void center();

  Gtk::HButtonBox *create_break_buttons(bool lockable, bool shutdownable);
  void on_lock_button_clicked();
  void on_shutdown_button_clicked();
  void on_skip_button_clicked();
  void on_postpone_button_clicked();
  bool on_delete_event(GdkEventAny *);
  
  //! Information about the (multi)head.
  HeadInfo head;

  //! Insist
  GUI::BlockMode block_mode;

  //! Ignorable
  bool ignorable_break;

private:
  Gtk::Button *create_skip_button();
  Gtk::Button *create_postpone_button();
  Gtk::Button *create_lock_button();
  Gtk::Button *create_shutdown_button();
  void set_background_pixmap();
  
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

  //! Send response to this interface.
  BreakResponseInterface *break_response;

  //! Break ID
  BreakId break_id;

  //! GUI
  Gtk::Widget *gui;
};

#endif // BREAKWINDOW_HH
