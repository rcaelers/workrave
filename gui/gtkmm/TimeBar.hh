// TimeBar.hh --- Time Bar
//
// Copyright (C) 2002 Rob Caelers <robc@krandor.org>
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

#ifndef TIMEBAR_HH
#define TIMEBAR_HH

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <gtkmm/drawingarea.h>
#include <gdkmm/colormap.h>
#include <gdkmm/window.h>
#include <gtkmm/box.h>

using namespace std;

class TimeBar : public Gtk::DrawingArea
{
public:
  enum ColorId
    {
      COLOR_ID_ACTIVE = 0,
      COLOR_ID_INACTIVE,
      COLOR_ID_OVERDUE,
      COLOR_ID_INACTIVE_OVER_ACTIVE,
      COLOR_ID_INACTIVE_OVER_OVERDUE,
      COLOR_ID_SIZEOF
    };

  TimeBar();
  virtual ~TimeBar();

  void set_progress(int value, int max_value);
  void set_secondary_progress(int value, int max_value);
  
  void set_text(string text);

  void update();
  void set_bar_color(ColorId color);
  void set_secondary_bar_color(ColorId color);
  void set_text_color(Gdk::Color color); 
  void set_text_alignment(int align);

  void set_border_size(int size);
  static string time_to_string(time_t t);
  
protected:
  //Overridden default signal handlers:
  virtual void on_realize();
  virtual bool on_expose_event(GdkEventExpose *event);
  virtual void on_size_request(GtkRequisition *requisition);

private:
  static Gdk::Color bar_colors[COLOR_ID_SIZEOF];
  
  //! Graphic context.
  Glib::RefPtr<Gdk::GC> window_gc;

  //! Color of the time-bar.
  ColorId bar_color;

  //! Color of the time-bar.
  ColorId secondary_bar_color;
  
  //! Color of the text.
  Gdk::Color bar_text_color;

  //! The current value.
  int bar_value;

  //! The maximum value.
  int bar_max_value;

  //! The current value.
  int secondary_bar_value;

  //! The maximum value.
  int secondary_bar_max_value;

  //! Text to show;
  string bar_text;

  //! Text alignment
  int bar_text_align;
};


#endif // TIMEBAR_HH
