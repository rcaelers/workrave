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

#include <gtkmm/box.h>

namespace Gtk
{
  class HButtonBox;
}

class RestBreakWindow :
  public BreakWindow
{
public:
  RestBreakWindow(HeadInfo &head, bool ignorable, bool insist);
  virtual ~RestBreakWindow();
  
  void start();
  void set_progress(int value, int max_value);
  void refresh();
  
protected:
  Gtk::Widget *create_gui();
  void draw_time_bar();
  
private:
  void suspend_break();
  Gtk::Widget *create_info_panel();
  void set_ignore_activity(bool i);
  
#ifdef HAVE_EXERCISES
  void install_exercises_panel();
  void install_info_panel();
  void clear_pluggable_panel();
  int get_exercise_count();
#endif

private:
  //! The Time
  TimeBar *timebar;
                   
  //! Progress
  int progress_value;

  //! Progress
  int progress_max_value;

#ifdef HAVE_EXERCISES
  Gtk::HBox pluggable_panel;
#endif
};


#endif // RESTBREAKWINDOW_HH
