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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"
#include "debug.hh"

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
  int dfw = 1;
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
Frame::set_frame_color(const Gdk::Color &col)
{
  frame_color = col;
#ifndef HAVE_GTK3
  if (color_map)
    {
#if 1 // FIXME: bug66
      color_map->alloc_color(frame_color);
#endif
    }
#endif
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
          flash_signal = Glib::signal_timeout().connect(sigc::mem_fun(*this, &Frame::on_timer), delay);
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
  flash_signal_src(frame_visible);
  return true;
}

void
Frame::on_size_allocate(Gtk::Allocation &allocation)
{

  Gtk::Widget *widget = get_child();
  guint b = get_border_width() + frame_width;

  Gtk::Allocation alloc;
  alloc.set_x(allocation.get_x() + b);
  alloc.set_y(allocation.get_y() + b);
  alloc.set_width(allocation.get_width() - 2*b);
  alloc.set_height(allocation.get_height() - 2*b);

  widget->size_allocate(alloc);
  set_allocation(allocation);

}

#ifdef HAVE_GTK3
Gtk::SizeRequestMode
Frame::get_request_mode_vfunc() const
{
  return Gtk::Widget::get_request_mode_vfunc();
}

void
Frame::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{ 
  TRACE_ENTER("Frame::get_preferred_width_vfunc");
  const Gtk::Widget *widget = get_child();
  widget->get_preferred_width(minimum_width, natural_width);

  guint d = 2*(get_border_width()+frame_width);
  minimum_width += d;
  natural_width += d;
  TRACE_MSG(minimum_width << " " << natural_width);
  TRACE_EXIT();
}

void
Frame::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
  TRACE_ENTER("Frame::get_preferred_height_vfunc");
  const Gtk::Widget *widget = get_child();
  widget->get_preferred_height(minimum_height, natural_height);

  guint d = 2*(get_border_width()+frame_width);
  minimum_height += d;
  natural_height += d;
  TRACE_MSG(minimum_height << " " << natural_height);
  TRACE_EXIT();
}

void
Frame::get_preferred_width_for_height_vfunc(int /* height */, int &minimum_width, int &natural_width) const
{
  TRACE_ENTER("Frame::get_preferred_width_for_height_vfunc");
  get_preferred_width_vfunc(minimum_width, natural_width);
  TRACE_EXIT();
}

void
Frame::get_preferred_height_for_width_vfunc(int /* width */, int &minimum_height, int &natural_height) const
{
  TRACE_ENTER("Frame::get_preferred_height_for_width_vfunc");
  get_preferred_height_vfunc(minimum_height, natural_height);
  TRACE_EXIT();
}

bool
Frame::on_draw(const Cairo::RefPtr< Cairo::Context >& cr)
{
  Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();

  // Physical width/height
  Gtk::Allocation allocation = get_allocation();
  int width = allocation.get_width();
  int height = allocation.get_height();

  switch (frame_style)
    {
    case STYLE_SOLID:
      if (frame_visible)
        {
          set_color(cr, frame_color);

          cr->rectangle(0, 0, frame_width, height);
          cr->fill();
          cr->rectangle(0+width-frame_width, 0, frame_width, height);
          cr->fill();
          cr->rectangle(0+frame_width, 0, width-2*frame_width, frame_width);
          cr->fill();
          cr->rectangle(0+frame_width, 0+height-frame_width, width-2*frame_width, frame_width);
          cr->fill();
        }    
      break;

    case STYLE_BREAK_WINDOW:
      style_context->context_save();

      style_context->add_class(GTK_STYLE_CLASS_FRAME);
      style_context->set_state((Gtk::StateFlags)0);

      style_context->render_background(cr, 0, 0, width, height);
      style_context->render_frame(cr, 0, 0, width, height);

      style_context->render_background(cr, 1, 1, width - 2, height -2);
      style_context->render_frame(cr, 1, 1, width - 2, height -2);

      style_context->context_restore();
      break;
    }

  Gtk::Widget::on_draw(cr);

  return true;
}

void
Frame::set_color(const Cairo::RefPtr<Cairo::Context>& cr, const Gdk::Color &color)
{
  cr->set_source_rgb(color.get_red_p(), color.get_green_p(), color.get_blue_p());
}


void
Frame::set_color(const Cairo::RefPtr<Cairo::Context>& cr, const Gdk::RGBA &color)
{
  cr->set_source_rgb(color.get_red(), color.get_green(), color.get_blue());
}

#else

void
Frame::on_realize()
{
  Gtk::Bin::on_realize();

  Glib::RefPtr<Gdk::Window> window = get_window();
  gc = Gdk::GC::create(window);

  color_black.set_rgb(0, 0, 0);
#if 1 // FIXME: bug66
  color_map = get_colormap();
  color_map->alloc_color(color_black);
#endif
  set_frame_color(frame_color);
}

void
Frame::on_size_request(Gtk::Requisition *requisition)
{
  Gtk::Widget *widget = get_child();
  widget->size_request(*requisition);
  guint d = 2*(get_border_width()+frame_width);
  requisition->width += d;
  requisition->height += d;
}


bool
Frame::on_expose_event(GdkEventExpose* e)
{
  Glib::RefPtr<Gdk::Window> window = get_window();
  Glib::RefPtr<Gtk::Style> style = get_style();

  Gdk::Color bgCol = style->get_background(Gtk::STATE_NORMAL);

  // FIXME:
  Gtk::Allocation gtkmmalloc = get_allocation();
  GtkAllocation alloc;
  alloc.x = gtkmmalloc.get_x();
  alloc.y = gtkmmalloc.get_y();
  alloc.width = gtkmmalloc.get_width();
  alloc.height = gtkmmalloc.get_height();

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
#ifdef OLD_STYLE_BORDER
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
#else
      Gdk::Rectangle area(&e->area);
      style->paint_box(window, Gtk::STATE_NORMAL, Gtk::SHADOW_OUT, area,
                       *this, "base", alloc.x, alloc.y,
                       alloc.width, alloc.height);
      style->paint_box(window, Gtk::STATE_NORMAL, Gtk::SHADOW_OUT, area,
                       *this, "base", alloc.x+1, alloc.y+1,
                       alloc.width-2, alloc.height-2);
#endif
      break;
    }

  bool rc = Gtk::Bin::on_expose_event(e);

  return rc;
}
#endif

sigc::signal1<void,bool> &
Frame::signal_flash()
{
  return flash_signal_src;
}

