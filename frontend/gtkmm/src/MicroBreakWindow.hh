// MicroBreakWindow.hh --- window for the microbreak
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Rob Caelers <robc@krandor.org>
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

#ifndef MICROBREAKWINDOW_HH
#define MICROBREAKWINDOW_HH

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

class MicroBreakWindow :
  public BreakWindow
{
public:
  MicroBreakWindow(HeadInfo &head, bool ignorable, GUI::BlockMode mode);
  virtual ~MicroBreakWindow();

  void set_progress(int value, int max_value);
  void heartbeat();

protected:
  Gtk::Widget *create_gui();
  void on_restbreaknow_button_clicked();
  
private:
  void refresh();
  void refresh_time_bar();
  void refresh_label();
  Gtk::Button *create_restbreaknow_button(bool label);

private:
  //! Time bar
  TimeBar *time_bar;

  // Label
  Gtk::Label *label;

  //! Progress
  int progress_value;

  //! Progress
  int progress_max_value;

  //! Is currently flashing because user is active?
  bool is_flashing;
};



#endif // MICROBREAKWINDOW_HH
