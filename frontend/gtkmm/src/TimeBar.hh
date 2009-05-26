// TimeBar.hh --- Time Bar
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2009 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef TIMEBAR_HH
#define TIMEBAR_HH

#include <string>

#include <gtkmm/drawingarea.h>
#include <gdkmm/colormap.h>
#include <gdkmm/window.h>
#include <gtkmm/box.h>

#include "ITimeBar.hh"

class TimeBar : public Gtk::DrawingArea, public ITimeBar
{
public:
  TimeBar();
  virtual ~TimeBar();

  void set_progress(int value, int max_value);
  void set_secondary_progress(int value, int max_value);

  void set_text(std::string text);

  void update();
  void set_bar_color(ColorId color);
  void set_secondary_bar_color(ColorId color);
  void set_text_alignment(int align);

  void set_border_size(int size);
  void set_rotation(int r);

  void get_minimum_size(int &width, int &height);
  void get_preferred_size(int &width, int &height);

private:
  void draw_bar(Glib::RefPtr<Gdk::Window> &window,
                const Glib::RefPtr<Gdk::GC> &gc,
                bool filled, int x, int y, int width, int height,
                int winw, int winh);

  void set_text_color(Gdk::Color color);
  
protected:
  //Overridden default signal handlers:
  virtual void on_realize();
  virtual bool on_expose_event(GdkEventExpose *event);
  virtual void on_size_request(GtkRequisition *requisition);
  virtual void on_size_allocate(Gtk::Allocation& allocation);

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
  std::string bar_text;

  //! Text alignment
  int bar_text_align;

  //! Bar rotation (clockwise degrees)
  int rotation;
};


#endif // TIMEBAR_HH
