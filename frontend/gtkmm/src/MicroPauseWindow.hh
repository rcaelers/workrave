// MicroPauseWindow.hh --- window for the micropause
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

#ifndef MICROPAUSEWINDOW_HH
#define MICROPAUSEWINDOW_HH

#include <stdio.h>

#include "BreakWindow.hh"
#include "GUI.hh"

class TimerInterface;
class TimeBar;
class Frame;
namespace Gtk
{
  class Label;
}

class MicroPauseWindow :
  public BreakWindow
{
public:
  MicroPauseWindow(HeadInfo &head, TimerInterface *timer, bool ignorable,
                   GUI::BlockMode mode);
  virtual ~MicroPauseWindow();

  void set_progress(int value, int max_value);
  void heartbeat();

protected:
  Gtk::Widget *create_gui();
  
private:
  void refresh();
  void refresh_time_bar();
  void refresh_label();
  
private:
  //!
  TimerInterface *restbreak_timer;
  
  //! Time bar
  TimeBar *time_bar;

  // Label
  Gtk::Label *label;

  //! Progress
  int progress_value;

  //! Progress
  int progress_max_value;
};



#endif // MICROPAUSEWINDOW_HH
