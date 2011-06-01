// TimeBar.cc --- Time Bar
//
// Copyright (C) 2002 - 2009, 2011 Rob Caelers & Raymond Penners
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

#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "TimeBar.hh"
#include "Text.hh"

const int MARGINX = 4;
const int MARGINY = 2;
const int MIN_HORIZONTAL_BAR_WIDTH = 12;
const int MIN_HORIZONTAL_BAR_HEIGHT = 20; // stolen from gtk's progress bar

using namespace std;

Gdk::Color TimeBar::bar_colors[TimeBar::COLOR_ID_SIZEOF] =
  {
    Gdk::Color("lightblue"),
    Gdk::Color("lightgreen"),
    Gdk::Color("orange"),
    Gdk::Color("red"),
    Gdk::Color("#e00000"),
    Gdk::Color("#00d4b2"),
    Gdk::Color("lightgreen"),
  };


//! Constructor
TimeBar::TimeBar() :
  bar_value(0),
  bar_max_value(0),
  secondary_bar_value(0),
  secondary_bar_max_value(0),
  bar_text_align(0),
  rotation(0)
{
  add_events(Gdk::EXPOSURE_MASK);
  add_events(Gdk::BUTTON_PRESS_MASK);

  set_bar_color(COLOR_ID_INACTIVE);
  set_secondary_bar_color(COLOR_ID_INACTIVE);
  set_text_color(Gdk::Color("black"));
}


//! Destructor
TimeBar::~TimeBar()
{
}


//! Sets the time progress to be displayed.
void
TimeBar::set_progress(int value, int max_value)
{
  if (value > max_value)
    {
      value = max_value;
    }

  bar_value = value;
  bar_max_value = max_value;
}


//! Sets the secondary time progress to be displayed.
void
TimeBar::set_secondary_progress(int value, int max_value)
{
  if (value > max_value)
    {
      value = max_value;
    }

  secondary_bar_value = value;
  secondary_bar_max_value = max_value;
}


//! Sets the text to be displayed.
void
TimeBar::set_text(string text)
{
  bar_text = text;
}


//! Sets text alignment
void
TimeBar::set_text_alignment(int align)
{
  bar_text_align = align;
}


//! Sets the color of the bar.
void
TimeBar::set_bar_color(ColorId color)
{
  bar_color = color;
}


//! Sets the color of the secondary bar.
void
TimeBar::set_secondary_bar_color(ColorId color)
{
  secondary_bar_color = color;
}


//! Sets the text color.
void
TimeBar::set_text_color(Gdk::Color color)
{
  bar_text_color = color;
}


void
TimeBar::set_rotation(int r)
{
  rotation = r;
  queue_resize();
}


//! Updates the screen.
void TimeBar::update()
{
  queue_draw();
}


void
TimeBar::on_size_allocate(Gtk::Allocation &allocation)
{
  //Use the offered allocation for this container:
  set_allocation(allocation);

  if (
#ifdef HAVE_GTK3
      get_realized()
#else
      is_realized()
#endif
      )
    {
      get_window()->move_resize(allocation.get_x(),
                                allocation.get_y(),
                                allocation.get_width(),
                                allocation.get_height());
    }
}

//! Returns the preferred size.
void
TimeBar::get_preferred_size(int &width, int &height) const
{
  // Not sure why create_pango_layout is not const...
  Glib::RefPtr<Pango::Layout> pl = const_cast<TimeBar *>(this)->create_pango_layout(bar_text);

  string min_string = Text::time_to_string(-(59+59*60+9*60*60));;
  Glib::RefPtr<Pango::Layout> plmin = const_cast<TimeBar *>(this)->create_pango_layout(min_string);

  Glib::RefPtr<Pango::Context> pcl = pl->get_context();
  Glib::RefPtr<Pango::Context> pcmin = plmin->get_context();
  Pango::Matrix matrix = PANGO_MATRIX_INIT;

  pango_matrix_rotate(&matrix, 360 - rotation);

  pcl->set_matrix(matrix);
  pcmin->set_matrix(matrix);

  pl->get_pixel_size(width, height);

  int mwidth, mheight;
  plmin->get_pixel_size(mwidth, mheight);
  if (mwidth > width)
    width = mwidth;
  if (mheight > height)
    height = mheight;

  width = width + 2 * MARGINX;
  height = max(height + 2 * MARGINY, MIN_HORIZONTAL_BAR_HEIGHT);
}


#ifndef HAVE_GTK3

void TimeBar::on_realize()
{
  // FIXME: for some reason, the timebar get realized EACH TIME
  //        the timerbox is cycled...
  // We need to call the base on_realize()
  Gtk::DrawingArea::on_realize();

  // Now we can allocate any additional resources we need
  Glib::RefPtr<Gdk::Window> window = get_window();
  window_gc = Gdk::GC::create(window);

  Glib::RefPtr<Gtk::Style> style = get_style();
  Gdk::Color bg = style->get_bg(Gtk::STATE_NORMAL);
  bar_colors[COLOR_ID_BG] = bg;

  Glib::RefPtr<Gdk::Colormap> colormap = get_colormap();
  for (int i = 0; i < COLOR_ID_SIZEOF; i++)
    {
      colormap->alloc_color(bar_colors[i]);
    }
  window->clear();
}


void
TimeBar::on_size_request(GtkRequisition *requisition)
{
  int width, height;

  get_preferred_size(width, height);

  if (rotation == 0 || rotation == 180)
    {
      requisition->width = width;
      requisition->height = height;
    }
  else
    {
      requisition->width = height;
      requisition->height = width;
    }
}


//! Draws the timebar
bool
TimeBar::on_expose_event(GdkEventExpose *e)
{
  TRACE_ENTER("TimeBar::on_expose_event");
  const int border_size = 2;
  Gtk::Allocation allocation = get_allocation();

  Glib::RefPtr<Gdk::Window> window = get_window();

  Glib::RefPtr<Gdk::Colormap> colormap = get_colormap();
  for (int i = 0; i < COLOR_ID_SIZEOF; i++)
    {
      colormap->alloc_color(bar_colors[i]);
    }
  
  // Physical width/height
  int win_w = allocation.get_width();
  int win_h = allocation.get_height();

  // Logical width/height
  // width = direction of bar
  int win_lw, win_lh;
  if (rotation == 0 || rotation == 180)
    {
      win_lw = win_w;
      win_lh = win_h;
    }
  else
    {
      win_lw = win_h;
      win_lh = win_w;
    }

  // Border
  Gdk::Rectangle area(&e->area);
  Glib::RefPtr<Gtk::Style> style = get_style();

  Gdk::Color bg = style->get_bg(Gtk::STATE_NORMAL);
  bar_colors[COLOR_ID_BG] = bg;
  
  // Draw background
  window_gc->set_foreground(bar_colors[COLOR_ID_BG]);
  style->paint_shadow(window, Gtk::STATE_NORMAL, Gtk::SHADOW_IN, area,
                      *this, "", 0, 0, win_w - 1, win_h -1);
  window->draw_rectangle(window_gc, true,
                         e->area.x+border_size,
                         e->area.y+border_size,
                         e->area.width -2*border_size,
                         e->area.height -2*border_size);

  // Bar
  int bar_width = 0;
  if (bar_max_value > 0)
    {
      bar_width = (bar_value * (win_lw - 2 * border_size)) / bar_max_value;
    }

  // Secondary bar
  int sbar_width = 0;
  if (secondary_bar_max_value >  0)
    {
      sbar_width = (secondary_bar_value * (win_lw - 2 * border_size)) / secondary_bar_max_value;
    }

  int bar_height = win_lh - 2 * border_size;

  if (sbar_width > 0)
    {
      // Overlap

      // We should assert() because this is not supported
      // but there are some weird boundary cases
      // in which this still happens.. need to check
      // this out some time.
      // assert(secondary_bar_color == COLOR_ID_INACTIVE);
      ColorId overlap_color;
      switch (bar_color)
        {
        case COLOR_ID_ACTIVE:
          overlap_color = COLOR_ID_INACTIVE_OVER_ACTIVE;
          break;
        case COLOR_ID_OVERDUE:
          overlap_color = COLOR_ID_INACTIVE_OVER_OVERDUE;
          break;
        default:
          // We could abort() because this is not supported
          // but there are some weird boundary cases
          // in which this still happens.. need to check
          // this out some time.
          overlap_color = COLOR_ID_INACTIVE_OVER_ACTIVE;
        }

      if (sbar_width >= bar_width)
        {
          if (bar_width)
            {
              window_gc->set_foreground(bar_colors[overlap_color]);
              draw_bar(window, window_gc, true,
                       border_size, border_size,
                       bar_width, bar_height,
                       win_lw, win_lh);
            }
          if (sbar_width > bar_width)
            {
              window_gc->set_foreground(bar_colors[secondary_bar_color]);
              draw_bar(window, window_gc, true,
                       border_size + bar_width, border_size,
                       sbar_width - bar_width, bar_height,
                       win_lw, win_lh);
            }
        }
      else
        {
          if (sbar_width)
            {
              window_gc->set_foreground(bar_colors[overlap_color]);
              draw_bar(window, window_gc, true,
                       border_size, border_size,
                       sbar_width, bar_height,
                       win_lw, win_lh);
            }
          window_gc->set_foreground(bar_colors[bar_color]);
          draw_bar(window, window_gc, true,
                   border_size + sbar_width, border_size,
                   bar_width - sbar_width, bar_height,
                   win_lw, win_lh);
        }
    }
  else
    {
      // No overlap
      window_gc->set_foreground(bar_colors[bar_color]);
      draw_bar(window, window_gc, true,
               border_size, border_size,
               bar_width, bar_height, win_lw, win_lh);
    }


  // Text
  window_gc->set_foreground(bar_text_color);
  Glib::RefPtr<Pango::Layout> pl1 = create_pango_layout(bar_text);
  Glib::RefPtr<Pango::Context> pc1 = pl1->get_context();

  Pango::Matrix matrix = PANGO_MATRIX_INIT;
  
  pango_matrix_rotate(&matrix, 360 - rotation);
  pc1->set_matrix(matrix);

  int text_width, text_height;
  pl1->get_pixel_size(text_width, text_height);

  int text_x, text_y;

  Gdk::Rectangle rect1, rect2;

  if (rotation == 0 || rotation == 180)
    {
      if (win_w - text_width - MARGINX > 0)
        {
          if (bar_text_align > 0)
            text_x = (win_w - text_width - MARGINX);
          else if (bar_text_align < 0)
            text_x = MARGINX;
          else
            text_x = (win_w - text_width) / 2;
        }
      else
        {
          text_x = MARGINX;
        }
      text_y = (win_h - text_height) / 2;

      int left_width = (bar_width > sbar_width) ? bar_width : sbar_width;
      left_width += border_size;

      Gdk::Rectangle left_rect(0, 0, left_width, win_h);
      Gdk::Rectangle right_rect(left_width, 0, win_w - left_width, win_h);

      rect1 = left_rect;
      rect2 = right_rect;
    }
  else
    {
      if (win_h - text_width - MARGINY > 0)
        {
          int a = bar_text_align;
          if (rotation == 270)
            {
              a *= -1;
            }
          if (a > 0)
            text_y = (win_h - text_width - MARGINY);
          else if (a < 0)
            text_y = MARGINY;
          else
            text_y = (win_h - text_width) / 2;
        }
      else
        {
          text_y = MARGINY;
        }

      text_x = (win_w - text_height) / 2;

      int left_width = (bar_width > sbar_width) ? bar_width : sbar_width;
      left_width += border_size;

      Gdk::Rectangle up_rect(0, 0, win_w, left_width);
      Gdk::Rectangle down_rect(0, left_width, win_w, win_h - left_width);

      rect1 = up_rect;
      rect2 = down_rect;
    }

  Gdk::Color textcolor = style->get_fg(Gtk::STATE_NORMAL);

  TRACE_MSG(textcolor.get_red() << " " <<
            textcolor.get_green() << " " <<
            textcolor.get_blue());
  
  Glib::RefPtr<Gdk::GC> window_gc1 = Gdk::GC::create(window);

  window_gc1->set_clip_origin(0,0);
  window_gc1->set_clip_rectangle(rect1);
  window_gc1->set_foreground(bar_text_color);
  window->draw_layout(window_gc1, text_x, text_y, pl1);

  window_gc1->set_foreground(textcolor);
  window_gc1->set_clip_rectangle(rect2);
  window->draw_layout(window_gc1, text_x, text_y, pl1);
  TRACE_EXIT();
  return true;
}


void
TimeBar::draw_bar(Glib::RefPtr<Gdk::Window> &window,
                  const Glib::RefPtr<Gdk::GC> &gc,
                  bool filled, int x, int y, int width, int height,
                  int winw, int winh)
{
  (void) winh;

  if (rotation == 0 || rotation == 180)
    {
      window->draw_rectangle(gc, filled, x, y, width, height);
    }
  else
    {
      window->draw_rectangle(gc, filled, y, winw - x - width, height, width);
    }
}

#else

Gtk::SizeRequestMode
TimeBar::get_request_mode_vfunc() const
{
  return Gtk::Widget::get_request_mode_vfunc();
}

void
TimeBar::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
  int width, height;
  get_preferred_size(width, height);

  if (rotation == 0 || rotation == 180)
    {
      minimum_width = natural_width = width;
    }
  else
    {
      minimum_width = natural_width = height;
    }
}

void
TimeBar::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
  int width, height;
  get_preferred_size(width, height);

  if (rotation == 0 || rotation == 180)
    {
      minimum_height = natural_height = height;
    }
  else
    {
      minimum_height = natural_height = width;
    }
}

void
TimeBar::get_preferred_width_for_height_vfunc(int /* height */, int &minimum_width, int &natural_width) const
{
  get_preferred_width_vfunc(minimum_width, natural_width);
}

void
TimeBar::get_preferred_height_for_width_vfunc(int /* width */, int &minimum_height, int &natural_height) const
{
  get_preferred_height_vfunc(minimum_height, natural_height);
}

bool
TimeBar::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
  TRACE_ENTER("TimeBar::on_draw");
  const int border_size = 2;

  Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();
  Gtk::Allocation allocation = get_allocation();

  style_context->context_save();
  style_context->add_class(GTK_STYLE_CLASS_FRAME);
 
  // Physical width/height
  int win_w = allocation.get_width();
  int win_h = allocation.get_height();

  // Logical width/height
  // width = direction of bar
  int win_lw, win_lh;
  if (rotation == 0 || rotation == 180)
    {
      win_lw = win_w;
      win_lh = win_h;
    }
  else
    {
      win_lw = win_h;
      win_lh = win_w;
    }

  // Draw background
  style_context->set_state(Gtk::STATE_FLAG_ACTIVE);
  Gdk::RGBA back_color = style_context->get_background_color();
  set_color(cr, back_color);

  // clip to the area indicated by the expose event so that we only redraw
  // the portion of the window that needs to be redrawn
  cr->rectangle(0, 0, win_w, win_h);
  cr->clip();

  style_context->context_save();
  style_context->set_state((Gtk::StateFlags)Gtk::STATE_FLAG_ACTIVE);
  style_context->render_frame(cr, 0, 0, win_w - 1, win_h -1);
  style_context->context_restore();
  
  set_color(cr, back_color);
  cr->rectangle(border_size, border_size, win_w - 2*border_size, win_h - 2*border_size);
  cr->fill();
  
  // Bar
  int bar_width = 0;
  if (bar_max_value > 0)
    {
      bar_width = (bar_value * (win_lw - 2 * border_size)) / bar_max_value;
    }

  // Secondary bar
  int sbar_width = 0;
  if (secondary_bar_max_value >  0)
    {
      sbar_width = (secondary_bar_value * (win_lw - 2 * border_size)) / secondary_bar_max_value;
    }

  int bar_height = win_lh - 2 * border_size;

  if (sbar_width > 0)
    {
      // Overlap

      // We should assert() because this is not supported
      // but there are some weird boundary cases
      // in which this still happens.. need to check
      // this out some time.
      // assert(secondary_bar_color == COLOR_ID_INACTIVE);
      ColorId overlap_color;
      switch (bar_color)
        {
        case COLOR_ID_ACTIVE:
          overlap_color = COLOR_ID_INACTIVE_OVER_ACTIVE;
          break;
        case COLOR_ID_OVERDUE:
          overlap_color = COLOR_ID_INACTIVE_OVER_OVERDUE;
          break;
        default:
          // We could abort() because this is not supported
          // but there are some weird boundary cases
          // in which this still happens.. need to check
          // this out some time.
          overlap_color = COLOR_ID_INACTIVE_OVER_ACTIVE;
        }

      if (sbar_width >= bar_width)
        {
          if (bar_width)
            {
              set_color(cr, bar_colors[overlap_color]);
              draw_bar(cr,
                       border_size, border_size,
                       bar_width, bar_height,
                       win_lw, win_lh);
            }
          if (sbar_width > bar_width)
            {
              set_color(cr, bar_colors[secondary_bar_color]);
              draw_bar(cr,
                       border_size + bar_width, border_size,
                       sbar_width - bar_width, bar_height,
                       win_lw, win_lh);
            }
        }
      else
        {
          if (sbar_width)
            {
              set_color(cr, bar_colors[overlap_color]);
              draw_bar(cr,
                       border_size, border_size,
                       sbar_width, bar_height,
                       win_lw, win_lh);
            }
          set_color(cr, bar_colors[bar_color]);
          draw_bar(cr,
                   border_size + sbar_width, border_size,
                   bar_width - sbar_width, bar_height,
                   win_lw, win_lh);
        }
    }
  else
    {
      // No overlap
      set_color(cr, bar_colors[bar_color]);
      draw_bar(cr,
               border_size, border_size,
               bar_width, bar_height, win_lw, win_lh);
    }


  // Text
  Pango::Matrix matrix = PANGO_MATRIX_INIT;
  pango_matrix_rotate(&matrix, 360 - rotation);

  Glib::RefPtr<Pango::Layout> pl1 = create_pango_layout(bar_text);
  Glib::RefPtr<Pango::Context> pc1 = pl1->get_context();
  
  pc1->set_matrix(matrix);

  int text_width, text_height;
  pl1->get_pixel_size(text_width, text_height);

  int text_x, text_y;

  Gdk::Rectangle rect1, rect2;

  if (rotation == 0 || rotation == 180)
    {
      if (win_w - text_width - MARGINX > 0)
        {
          if (bar_text_align > 0)
            text_x = (win_w - text_width - MARGINX);
          else if (bar_text_align < 0)
            text_x = MARGINX;
          else
            text_x = (win_w - text_width) / 2;
        }
      else
        {
          text_x = MARGINX;
        }
      text_y = (win_h - text_height) / 2;

      int left_width = (bar_width > sbar_width) ? bar_width : sbar_width;
      left_width += border_size;

      Gdk::Rectangle left_rect(0, 0, left_width, win_h);
      Gdk::Rectangle right_rect(left_width, 0, win_w - left_width, win_h);

      rect1 = left_rect;
      rect2 = right_rect;
    }
  else
    {
      if (win_h - text_width - MARGINY > 0)
        {
          int a = bar_text_align;
          if (rotation == 270)
            {
              a *= -1;
            }
          if (a > 0)
            text_y = (win_h - text_width - MARGINY);
          else if (a < 0)
            text_y = MARGINY;
          else
            text_y = (win_h - text_width) / 2;
        }
      else
        {
          text_y = MARGINY;
        }

      text_x = (win_w - text_height) / 2;

      int left_width = (bar_width > sbar_width) ? bar_width : sbar_width;
      left_width += border_size;

      Gdk::Rectangle up_rect(0, 0, win_w, left_width);
      Gdk::Rectangle down_rect(0, left_width, win_w, win_h - left_width);

      rect1 = up_rect;
      rect2 = down_rect;
    }

  cr->reset_clip();
  cr->rectangle(rect1.get_x(), rect1.get_y(), rect1.get_width(), rect1.get_height());
  cr->clip();

  cr->move_to(text_x, text_y);
  set_color(cr, bar_text_color);
  pl1->show_in_cairo_context(cr);
 
  Gdk::RGBA front_color = style_context->get_color();
  cr->reset_clip();
  cr->rectangle(rect2.get_x(), rect2.get_y(), rect2.get_width(), rect2.get_height());
  cr->clip();
  cr->move_to(text_x, text_y);
  set_color(cr, front_color);
  pl1->show_in_cairo_context(cr);

  style_context->context_restore();
  
  TRACE_EXIT();
  return Gtk::Widget::on_draw(cr);;
}

void
TimeBar::set_color(const Cairo::RefPtr<Cairo::Context>& cr, const Gdk::Color &color)
{
  cr->set_source_rgb(color.get_red_p(), color.get_green_p(), color.get_blue_p());
}


void
TimeBar::set_color(const Cairo::RefPtr<Cairo::Context>& cr, const Gdk::RGBA &color)
{
  cr->set_source_rgb(color.get_red(), color.get_green(), color.get_blue());
}

void
TimeBar::draw_bar(const Cairo::RefPtr<Cairo::Context>& cr,
                  int x, int y, int width, int height,
                  int winw, int winh)
{
  (void) winh;

  if (rotation == 0 || rotation == 180)
    {
      cr->rectangle(x, y, width, height);
      cr->fill();
    }
  else
    {
      cr->rectangle(y, winw - x- width, height, width);
      cr->fill();
    }
}

#endif
