// PreludeWindow.hh --- window for the micropause
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
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

#ifndef PRELUDEWINDOW_HH
#define PRELUDEWINDOW_HH

#include <stdio.h>

#include "BreakWindow.hh"
#include "PreludeWindowInterface.hh"

class TimeBar;
class Frame;

class PreludeWindow :
  public BreakWindow,
  public PreludeWindowInterface
{
public:
  PreludeWindow();
  ~PreludeWindow();

  void start();
  void stop();
  void destroy();
  void refresh();
  void set_progress(int value, int max_value);
  void set_text(string text);
  void set_frame(int stage);
  void set_progress_text(string text);
  
private:  
  //! Time bar
  TimeBar *time_bar;

  //! Time bar
  Frame *frame;

  //! Warn color
  Gdk::Color color_warn;

  //! Alert color
  Gdk::Color color_alert;

  //! Label
  Gtk::Label *label;

  //! Icon
  Gtk::Image *image_icon;

  //! Final prelude
  string progress_text;
};

#endif // PRELUDEWINDOW_HH
