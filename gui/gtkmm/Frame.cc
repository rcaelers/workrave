// FrameWindow.hh --- Gtk::Frame like widget 
//
// Copyright (C) 2001, 2002 Raymond Penners <raymond@dotsphinx.com>
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

#include "Frame.hh"

Frame::Frame()
  : frame_width(1),
    frame_color(Gdk::Color("black")),
    frame_style(STYLE_SOLID),
    frame_visible(true),
    flash_delay(-1)
{
}

Frame::~Frame()
{
  if (flash_signal.connected())
    {
      flash_signal.disconnect();
    }
  // FIXME: delete gc?
}

void
Frame::set_frame_visible(bool visible)
{
  frame_visible = visible;
  queue_draw();
}

void
Frame::set_frame_style(const Style style)
{
  frame_style = style;
  int dfw;
  switch (style)
    {
    case STYLE_BREAK_WINDOW:
      dfw = 3;
      break;
    case STYLE_SOLID:
      dfw = 1;
      break;
    }
  set_frame_width(dfw);
}

void
Frame::set_frame_color(const Gdk::Color &col )
{
  frame_color = col;
  if (color_map)
    {
      color_map->alloc_color(frame_color);
      
    }
}

void
Frame::set_frame_width(guint width)
{
  frame_width = width;
}


void
Frame::set_frame_flashing(int delay)
{
  if (delay != 0)
    {
      if (flash_delay != delay)
        {
          if (flash_signal.connected())
            {
              flash_signal.disconnect();
            }
          flash_signal = Glib::signal_timeout().connect(SigC::slot(*this, &Frame::on_timer), delay);
        }
    }
  else
    {
      if (flash_signal.connected())
        {
          flash_signal.disconnect();
          set_frame_visible(true);
        }
    }
  flash_delay = delay;
}

bool
Frame::on_timer()
{
  set_frame_visible(! frame_visible);
  return true;
}

void
Frame::on_realize()
{
  Gtk::Bin::on_realize();

  Glib::RefPtr<Gdk::Window> window = get_window();
  gc = Gdk::GC::create(window);

  color_map = get_default_colormap();
  color_black.set_rgb(0, 0, 0);
  color_map->alloc_color(color_black);

  set_frame_color(frame_color);
}

void 
Frame::on_size_request(GtkRequisition *requisition)
{
  Gtk::Widget *widget = get_child();
  widget->size_request(requisition);
  guint d = 2*(get_border_width()+frame_width);
  requisition->width += d;
  requisition->height += d;
}

void
Frame::on_size_allocate(GtkAllocation* allocation)
{
  Gtk::Bin::on_size_allocate(allocation);
  
  Gtk::Widget *widget = get_child();
  guint b = get_border_width() + frame_width;

  GtkAllocation alloc;
  alloc.x = allocation->x + b;
  alloc.y = allocation->y + b;
  alloc.width = allocation->width - 2*b;
  alloc.height = allocation->height - 2*b;
  widget->size_allocate(&alloc);
}

bool
Frame::on_expose_event(GdkEventExpose* e)
{
  Glib::RefPtr<Gdk::Window> window = get_window();
  Glib::RefPtr<Gtk::Style> style = get_style();

  Gdk::Color bgCol = style->get_background(Gtk::STATE_NORMAL);

  GtkAllocation alloc = get_allocation();
  
  switch (frame_style)
    {
    case STYLE_SOLID:
      gc->set_foreground(frame_visible ? frame_color : bgCol);

      window->draw_rectangle(gc, true, alloc.x, alloc.y,
			     frame_width, alloc.height);
      window->draw_rectangle(gc, true, alloc.x+alloc.width-frame_width,
			     alloc.y, frame_width, alloc.height);
      window->draw_rectangle(gc, true, alloc.x+frame_width, alloc.y,
			     alloc.width-2*frame_width, frame_width);
      window->draw_rectangle(gc, true, alloc.x+frame_width,
			     alloc.y+alloc.height-frame_width,
			     alloc.width-2*frame_width, frame_width);
      break;

    case STYLE_BREAK_WINDOW:
      gc->set_foreground(color_black);
      window->draw_rectangle(gc, true, alloc.x, alloc.y,
			     alloc.width, alloc.height);
      Gdk::Rectangle area(&e->area);
      style->paint_box(window, Gtk::STATE_NORMAL, Gtk::SHADOW_OUT, area,
		       *this, "", alloc.x+1, alloc.y+1,
		       alloc.width-1, alloc.height-1);
      style->paint_box(window, Gtk::STATE_NORMAL, Gtk::SHADOW_OUT, area,
		       *this, "", alloc.x+2, alloc.y+2,
		       alloc.width-3, alloc.height-3);
      break;
    }

  bool rc = Gtk::Bin::on_expose_event(e);

  return rc;
}

