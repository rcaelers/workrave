// FrameWindow.hh --- Gtk::Frame like widget
//
// Copyright (C) 2001, 2002, 2003, 2004, 2007, 2011 Raymond Penners <raymond@dotsphinx.com>
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

#include <gtkmm/frame.h>

class Frame : public Gtk::Bin
{
public:
  enum Style
  {
    STYLE_SOLID,
    STYLE_BREAK_WINDOW
  };

  Frame();
  virtual ~Frame();

  void set_frame_width(guint width);
  void set_frame_style(Style style);
  void set_frame_color(const Gdk::Color &color);
  void set_frame_flashing(int delay);
  void set_frame_visible(bool visible);
  sigc::signal1<void,bool> &signal_flash();

protected:
  bool on_timer();
  void on_size_allocate(Gtk::Allocation &allocation);

#ifdef HAVE_GTK3
  virtual Gtk::SizeRequestMode get_request_mode_vfunc() const;
  virtual void get_preferred_width_vfunc(int& minimum_width, int& natural_width) const;
  virtual void get_preferred_height_vfunc(int& minimum_height, int& natural_height) const;
  virtual void get_preferred_width_for_height_vfunc(int height, int& minimum_width, int& natural_width) const;
  virtual void get_preferred_height_for_width_vfunc(int width, int& minimum_height, int& natural_height) const;
  virtual bool on_draw(const Cairo::RefPtr< Cairo::Context >& cr);

  void set_color(const Cairo::RefPtr<Cairo::Context>& cr, const Gdk::Color &color);
  void set_color(const Cairo::RefPtr<Cairo::Context>& cr, const Gdk::RGBA &color);
#else
  void on_realize();
  bool on_expose_event(GdkEventExpose* e);
  void on_size_request(Gtk::Requisition *requisition);
#endif
  

private:
  //! Frame border width
  guint frame_width;

#ifndef HAVE_GTK3
  //! Graphic context.
  Glib::RefPtr<Gdk::GC> gc;

  //! Color map
  Glib::RefPtr<Gdk::Colormap> color_map;
#endif
  
  //! Color of the frame.
  Gdk::Color frame_color;

  //! Black
  Gdk::Color color_black;

  //! Style of the frame.
  Style frame_style;

  //! Visible;
  bool frame_visible;

  //! Flash delay;
  int flash_delay;

  //! Flash timeout signal
  sigc::connection flash_signal;

  //! Flash signal source
  sigc::signal1<void,bool> flash_signal_src;
};

