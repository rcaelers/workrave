// RestBreakWindow.hh --- window for the micropause
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
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

#ifndef RESTBREAKWINDOW_HH
#define RESTBREAKWINDOW_HH

#include <stdio.h>

class BreakControl;
class TimeBar;

#include "BreakInterface.hh"
#include "BreakWindowInterface.hh"
#include "BreakWindow.hh"

class RestBreakWindow :
  public BreakWindow,
  public BreakWindowInterface
{
public:
  RestBreakWindow(bool ignorable);
  virtual ~RestBreakWindow();

  void start();
  void stop();
  void destroy();
  void set_progress(int value, int max_value);
  void set_insist_break(bool insist);
  void refresh();
  
protected:
  //Overridden default signal handlers:
  virtual void on_realize();
  virtual bool on_expose_event(GdkEventExpose* event);
  
  void on_postpone_button_clicked();
  void on_skip_button_clicked();

  void draw_time_bar();
  
private:
  void suspend_break();
  
private:
  //! Graphic context.
  Glib::RefPtr<Gdk::GC> window_gc;

  //!
  Gtk::HButtonBox *button_box;
  
  //! Color of the time-bar.
  Gdk::Color border_color;
  
  //! Color of the text.
  Gdk::Color text_color;

  //! Window width
  int window_width;

  //! Width height
  int window_height;

  //! The Time
  TimeBar *timebar;
                   
  //!
  int progress_value;

  //!
  int progress_max_value;

  //!
  bool insist_break;
};

#endif // RESTBREAKWINDOW_HH
