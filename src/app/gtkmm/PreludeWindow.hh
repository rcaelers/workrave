// PreludeWindow.hh --- window for the micropause
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

#ifndef PRELUDEWINDOW_HH
#define PRELUDEWINDOW_HH

#include "BreakWindow.hh"
#include "PreludeWindowInterface.hh"
#include "ActivityMonitorListener.hh"

class TimeBar;
class Frame;
class Dispatcher;

class PreludeWindow :
  public BreakWindow,
  public PreludeWindowInterface,
  public ActivityMonitorListener
{
public:
  PreludeWindow(BreakId break_id);
  virtual ~PreludeWindow();

  void start();
  bool delayed_stop();
  void stop();
  void destroy();
  void refresh();
  void set_progress(int value, int max_value);
  void set_stage(Stage stage);
  void set_progress_text(ProgressText text);
  void set_prelude_response(PreludeResponseInterface *pri);
  
private:
  bool action_notify();
  void on_activity();
  void on_frame_flash(bool frame_visible);
  
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

  //! Progress values
  int progress_value;
  int progress_max_value;

  //! Flash
  bool flash_visible;;
  
  SigC::Connection dispatch_connection;
  Dispatcher *dispatcher;

  //! Send response to this interface.
  PreludeResponseInterface *prelude_response;
};


inline void
PreludeWindow::set_prelude_response(PreludeResponseInterface *pri)
{
  prelude_response = pri;
}

#endif // PRELUDEWINDOW_HH
