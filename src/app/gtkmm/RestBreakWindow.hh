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

class TimeBar;

#include "BreakWindow.hh"
#include "BreakWindowInterface.hh"

class RestBreakWindow :
  public BreakWindow,
  public BreakWindowInterface
{
public:
  RestBreakWindow(bool ignorable, bool insist MULTIHEAD_PARAMS);
  virtual ~RestBreakWindow();
  
  void start();
  void stop();
  void destroy();
  void set_progress(int value, int max_value);
  void refresh();
  void set_response(BreakResponseInterface *bri);
  
protected:
  //Overridden default signal handlers:
  virtual void on_realize();
  virtual bool on_expose_event(GdkEventExpose* event);
  
  void on_postpone_button_clicked();
  void on_skip_button_clicked();

  void draw_time_bar();
  
private:
  void suspend_break();
  Gtk::Widget *RestBreakWindow::create_info_panel();
  
#ifdef HAVE_EXERCISES
  void install_exercises_panel();
  void install_info_panel();
  void clear_pluggable_panel();
  int get_exercise_count();
#endif

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
  
#ifdef HAVE_EXERCISES
  Gtk::HBox pluggable_panel;
#endif

  //! Send response to this interface.
  BreakResponseInterface *break_response;
};

inline void
RestBreakWindow::set_response(BreakResponseInterface *bri)
{
  break_response = bri;
}

#endif // RESTBREAKWINDOW_HH
