// Copyright (C) 2001 - 2011 Raymond Penners <raymond@dotsphinx.com>
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
#  include "config.h"
#endif

#include "Frame.hh"

Frame::Frame()
  : frame_color(Gdk::RGBA("black"))
{
}

Frame::~Frame()
{
  if (flash_signal.connected())
    {
      flash_signal.disconnect();
    }
#if GTK_CHECK_VERSION(4, 0, 0)
  if (child != nullptr)
    {
      child->unparent();
    }
#endif
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
Frame::set_frame_color(const Gdk::RGBA &col)
{
  frame_color = col;
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
  set_frame_visible(!frame_visible);
  flash_signal_src(frame_visible);
  return true;
}

#if GTK_CHECK_VERSION(4, 0, 0)
void
Frame::add(Gtk::Widget &child_widget)
{
  child = &child_widget;
  child_widget.set_parent(*this);
}

Gtk::Widget *
Frame::get_child() const
{
  return child;
}

void
Frame::set_border_width(guint width)
{
  border_width = width;
  queue_resize();
}

guint
Frame::get_border_width() const
{
  return border_width;
}

void
Frame::size_allocate_vfunc(int width, int height, int baseline)
{
  if (child != nullptr)
    {
      int b = static_cast<int>(border_width + frame_width);

      Gtk::Allocation alloc;
      alloc.set_x(b);
      alloc.set_y(b);
      alloc.set_width(width - 2 * b);
      alloc.set_height(height - 2 * b);

      child->size_allocate(alloc, baseline);
    }
}

void
Frame::measure_vfunc(Gtk::Orientation orientation,
                      int for_size,
                      int &minimum,
                      int &natural,
                      int &minimum_baseline,
                      int &natural_baseline) const
{
  minimum = natural = 0;
  minimum_baseline = natural_baseline = -1;

  if (child != nullptr)
    {
      child->measure(orientation, for_size, minimum, natural, minimum_baseline, natural_baseline);
    }

  int d = 2 * static_cast<int>(border_width + frame_width);
  minimum += d;
  natural += d;
  minimum_baseline = natural_baseline = -1;
}

void
Frame::snapshot_vfunc(const Glib::RefPtr<Gtk::Snapshot> &snapshot)
{
  int width = get_width();
  int height = get_height();

  auto cr = snapshot->append_cairo(Gdk::Rectangle(0, 0, width, height));
  draw(cr, width, height);

  if (child != nullptr)
    {
      snapshot_child(*child, snapshot);
    }
}
#else
void
Frame::on_size_allocate(Gtk::Allocation &allocation)
{
  Gtk::Widget *widget = get_child();
  guint b = get_border_width() + frame_width;

  Gtk::Allocation alloc;
  alloc.set_x(allocation.get_x() + b);
  alloc.set_y(allocation.get_y() + b);
  alloc.set_width(allocation.get_width() - 2 * b);
  alloc.set_height(allocation.get_height() - 2 * b);

  widget->size_allocate(alloc);
  set_allocation(allocation);
}

Gtk::SizeRequestMode
Frame::get_request_mode_vfunc() const
{
  return Gtk::Widget::get_request_mode_vfunc();
}

void
Frame::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
  const Gtk::Widget *widget = get_child();
  widget->get_preferred_width(minimum_width, natural_width);

  guint d = 2 * (get_border_width() + frame_width);
  minimum_width += d;
  natural_width += d;
}

void
Frame::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
  const Gtk::Widget *widget = get_child();
  widget->get_preferred_height(minimum_height, natural_height);

  guint d = 2 * (get_border_width() + frame_width);
  minimum_height += d;
  natural_height += d;
}

void
Frame::get_preferred_width_for_height_vfunc(int /* height */, int &minimum_width, int &natural_width) const
{
  get_preferred_width_vfunc(minimum_width, natural_width);
}

void
Frame::get_preferred_height_for_width_vfunc(int /* width */, int &minimum_height, int &natural_height) const
{
  get_preferred_height_vfunc(minimum_height, natural_height);
}

bool
Frame::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
  // Physical width/height
  Gtk::Allocation allocation = get_allocation();
  int width = allocation.get_width();
  int height = allocation.get_height();

  draw(cr, width, height);

  Gtk::Widget::on_draw(cr);

  return true;
}
#endif

void
Frame::draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height)
{
  Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();

  switch (frame_style)
    {
    case STYLE_SOLID:
      if (frame_visible)
        {
          set_color(cr, frame_color);

          cr->rectangle(0, 0, frame_width, height);
          cr->fill();
          cr->rectangle(0 + width - frame_width, 0, frame_width, height);
          cr->fill();
          cr->rectangle(0 + frame_width, 0, width - 2 * frame_width, frame_width);
          cr->fill();
          cr->rectangle(0 + frame_width, 0 + height - frame_width, width - 2 * frame_width, frame_width);
          cr->fill();
        }
      break;

    case STYLE_BREAK_WINDOW:
      style_context->context_save();

      style_context->set_state((Gtk::StateFlags)0);

#if GTK_CHECK_VERSION(4, 0, 0)
      style_context->add_class("background");
      style_context->render_background(cr, 0, 0, width, height);

      style_context->remove_class("background");
      style_context->add_class("frame");
      style_context->render_frame(cr, 0, 0, width, height);

      style_context->remove_class("frame");
      style_context->add_class("background");
      style_context->render_background(cr, 1, 1, width - 2, height - 2);

      style_context->remove_class("background");
      style_context->add_class("frame");
      style_context->render_frame(cr, 1, 1, width - 2, height - 2);
#else
      style_context->add_class(GTK_STYLE_CLASS_BACKGROUND);
      style_context->render_background(cr, 0, 0, width, height);

      style_context->remove_class(GTK_STYLE_CLASS_BACKGROUND);
      style_context->add_class(GTK_STYLE_CLASS_FRAME);
      style_context->render_frame(cr, 0, 0, width, height);

      style_context->remove_class(GTK_STYLE_CLASS_FRAME);
      style_context->add_class(GTK_STYLE_CLASS_BACKGROUND);
      style_context->render_background(cr, 1, 1, width - 2, height - 2);

      style_context->remove_class(GTK_STYLE_CLASS_BACKGROUND);
      style_context->add_class(GTK_STYLE_CLASS_FRAME);
      style_context->render_frame(cr, 1, 1, width - 2, height - 2);
#endif

      style_context->context_restore();
      break;
    }
}

void
Frame::set_color(const Cairo::RefPtr<Cairo::Context> &cr, const Gdk::RGBA &color)
{
  cr->set_source_rgb(color.get_red(), color.get_green(), color.get_blue());
}

Frame::flash_signal_t &
Frame::signal_flash()
{
  return flash_signal_src;
}
