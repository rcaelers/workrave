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

class TimeBar;
class Frame;
class Dispatcher;

namespace Gtk
{
  class Image;
  class Label;
}

class PreludeWindow :
  public Gtk::Window,
  public PreludeWindowInterface
{
public:
  PreludeWindow(HeadInfo &head, BreakId break_id);
  virtual ~PreludeWindow();

  void start();
  void stop();
  void destroy();
  void refresh();
  void set_progress(int value, int max_value);
  void set_stage(AppInterface::PreludeStage stage);
  void set_progress_text(AppInterface::PreludeProgressText text);
  void set_response(BreakResponseInterface *pri);
  
private:
  void on_frame_flash(bool frame_visible);
  void init_avoid_pointer();
  void add(Gtk::Widget& widget);
  
#ifdef WIN32
  bool on_avoid_pointer_timer();
#else
  bool on_enter_notify_event(GdkEventCrossing* event);
#endif
  void avoid_pointer(int x, int y);
  
private:
#ifdef WIN32
  //! Avoid time signal
  SigC::Connection avoid_signal;
#endif

  //! Avoid margin.
  const int SCREEN_MARGIN;

  //! Did we avoid the pointer?
  bool did_avoid;
  
  //!
  BreakId break_id;
  
  //! Time bar
  TimeBar *time_bar;

  //! Frame
  Frame *frame;

  //! Frame
  Frame *window_frame;

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

  //! Head
  HeadInfo head;

  //! Send response to this interface.
  BreakResponseInterface *prelude_response;

  //! Border
  guint border_width;
};


inline void
PreludeWindow::set_response(BreakResponseInterface *pri)
{
  prelude_response = pri;
}

#endif // PRELUDEWINDOW_HH
