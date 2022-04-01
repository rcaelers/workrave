// Copyright (C) 2002 - 2013 Rob Caelers & Raymond Penners
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

#include "debug.hh"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <sstream>

#include "TimeBar.hh"
#include "commonui/Text.hh"
#include "GtkUtil.hh"

const int MARGINX = 4;
const int MARGINY = 2;
const int MIN_HORIZONTAL_BAR_HEIGHT = 20; // stolen from gtk's progress bar

using namespace std;

std::map<TimerColorId, Gdk::Color> TimeBar::bar_colors{
  {TimerColorId::Active, Gdk::Color("lightblue")},
  {TimerColorId::Inactive, Gdk::Color("lightgreen")},
  {TimerColorId::Overdue, Gdk::Color("orange")},
  {TimerColorId::ActiveDuringBreak1, Gdk::Color("red")},
  {TimerColorId::ActiveDuringBreak2, Gdk::Color("#e00000")},
  {TimerColorId::InactiveOverActive, Gdk::Color("#00d4b2")},
  {TimerColorId::InactiveOverOverdue, Gdk::Color("lightgreen")},
  {TimerColorId::Bg, Gdk::Color("#777777")},
};

TimeBar::TimeBar()
{
  add_events(Gdk::EXPOSURE_MASK);
  add_events(Gdk::BUTTON_PRESS_MASK);

  set_bar_color(TimerColorId::Inactive);
  set_secondary_bar_color(TimerColorId::Inactive);
  set_text_color(Gdk::Color("black"));

  GtkUtil::set_theme_fg_color(this);
}

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

void
TimeBar::set_text(string text)
{
  bar_text = text;
}

void
TimeBar::set_text_alignment(int align)
{
  bar_text_align = align;
}

void
TimeBar::set_bar_color(TimerColorId color)
{
  bar_color = color;
}

void
TimeBar::set_secondary_bar_color(TimerColorId color)
{
  secondary_bar_color = color;
}

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

void
TimeBar::update()
{
  queue_draw();
}

void
TimeBar::on_size_allocate(Gtk::Allocation &allocation)
{
  // Use the offered allocation for this container:
  set_allocation(allocation);

  if (get_realized())
    {
      get_window()->move_resize(allocation.get_x(), allocation.get_y(), allocation.get_width(), allocation.get_height());
    }
}

void
TimeBar::get_preferred_size(int &width, int &height) const
{
  // Not sure why create_pango_layout is not const...
  Glib::RefPtr<Pango::Layout> pl = const_cast<TimeBar *>(this)->create_pango_layout(bar_text);

  string min_string = Text::time_to_string(-(59 + 59 * 60 + 9 * 60 * 60));
  Glib::RefPtr<Pango::Layout> plmin = const_cast<TimeBar *>(this)->create_pango_layout(min_string);

  Glib::RefPtr<Pango::Context> pcl = pl->get_context();
  Glib::RefPtr<Pango::Context> pcmin = plmin->get_context();
  Pango::Matrix matrix = PANGO_MATRIX_INIT;

  pango_matrix_rotate(&matrix, 360 - rotation);

  pcl->set_matrix(matrix);
  pcmin->set_matrix(matrix);

  pl->get_pixel_size(width, height);

  int mwidth = 0;
  int mheight = 0;
  plmin->get_pixel_size(mwidth, mheight);
  if (mwidth > width)
    {
      width = mwidth;
    }
  if (mheight > height)
    {
      height = mheight;
    }

  width = width + 2 * MARGINX;
  height = max(height + 2 * MARGINY, MIN_HORIZONTAL_BAR_HEIGHT);
}

Gtk::SizeRequestMode
TimeBar::get_request_mode_vfunc() const
{
  return Gtk::Widget::get_request_mode_vfunc();
}

void
TimeBar::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
  int width = 0;
  int height = 0;
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
  int width = 0;
  int height = 0;
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
  const int border_size = 1;

  Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();
  Gtk::Allocation allocation = get_allocation();

  style_context->context_save();
  style_context->add_class(GTK_STYLE_CLASS_FRAME);

  // Physical width/height
  int win_w = allocation.get_width() - 2;
  int win_h = allocation.get_height();

  // Logical width/height
  // width = direction of bar
  int win_lw = 0;
  int win_lh = 0;
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

  style_context->render_background(cr, 0, 0, win_w - 1, win_h - 1);
  style_context->render_frame(cr, 0, 0, win_w - 1, win_h - 1);

  // set_color(cr, back_color);
  // cr->rectangle(border_size, border_size, win_w - 2*border_size, win_h - 2*border_size);
  // cr->fill();

  // Bar
  int bar_width = 0;
  if (bar_max_value > 0)
    {
      bar_width = (bar_value * (win_lw - 2 * border_size - 1)) / bar_max_value;
    }

  // Secondary bar
  int sbar_width = 0;
  if (secondary_bar_max_value > 0)
    {
      sbar_width = (secondary_bar_value * (win_lw - 2 * border_size - 1)) / secondary_bar_max_value;
    }

  int bar_height = win_lh - 2 * border_size - 1;

  if (sbar_width > 0)
    {
      // Overlap

      // We should assert() because this is not supported
      // but there are some weird boundary cases
      // in which this still happens.. need to check
      // this out some time.
      // assert(secondary_bar_color == TimerColorId::Inactive);
      TimerColorId overlap_color;
      switch (bar_color)
        {
        case TimerColorId::Active:
          overlap_color = TimerColorId::InactiveOverActive;
          break;
        case TimerColorId::Overdue:
          overlap_color = TimerColorId::InactiveOverOverdue;
          break;
        default:
          // We could abort() because this is not supported
          // but there are some weird boundary cases
          // in which this still happens.. need to check
          // this out some time.
          overlap_color = TimerColorId::InactiveOverActive;
        }

      if (sbar_width >= bar_width)
        {
          if (bar_width)
            {
              set_color(cr, bar_colors[overlap_color]);
              draw_bar(cr, border_size, border_size, bar_width, bar_height, win_lw, win_lh);
            }
          if (sbar_width > bar_width)
            {
              set_color(cr, bar_colors[secondary_bar_color]);
              draw_bar(cr, border_size + bar_width, border_size, sbar_width - bar_width, bar_height, win_lw, win_lh);
            }
        }
      else
        {
          if (sbar_width)
            {
              set_color(cr, bar_colors[overlap_color]);
              draw_bar(cr, border_size, border_size, sbar_width, bar_height, win_lw, win_lh);
            }
          set_color(cr, bar_colors[bar_color]);
          draw_bar(cr, border_size + sbar_width, border_size, bar_width - sbar_width, bar_height, win_lw, win_lh);
        }
    }
  else
    {
      // No overlap
      set_color(cr, bar_colors[bar_color]);
      draw_bar(cr, border_size, border_size, bar_width, bar_height, win_lw, win_lh);
    }

  // Text
  Pango::Matrix matrix = PANGO_MATRIX_INIT;
  pango_matrix_rotate(&matrix, 360 - rotation);

  Glib::RefPtr<Pango::Layout> pl1 = create_pango_layout(bar_text);
  Glib::RefPtr<Pango::Context> pc1 = pl1->get_context();

  pc1->set_matrix(matrix);

  int text_width = 0;
  int text_height = 0;
  pl1->get_pixel_size(text_width, text_height);

  int text_x = 0;
  int text_y = 0;

  Gdk::Rectangle rect1;
  Gdk::Rectangle rect2;

  if (rotation == 0 || rotation == 180)
    {
      if (win_w - text_width - MARGINX > 0)
        {
          if (bar_text_align > 0)
            {
              text_x = (win_w - text_width - MARGINX);
            }
          else if (bar_text_align < 0)
            {
              text_x = MARGINX;
            }
          else
            {
              text_x = (win_w - text_width) / 2;
            }
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
            {
              text_y = (win_h - text_width - MARGINY);
            }
          else if (a < 0)
            {
              text_y = MARGINY;
            }
          else
            {
              text_y = (win_h - text_width) / 2;
            }
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

  cr->reset_clip();
  cr->rectangle(rect2.get_x(), rect2.get_y(), rect2.get_width(), rect2.get_height());
  cr->clip();

  set_color(cr, style_context->get_color(Gtk::STATE_FLAG_ACTIVE));
  cr->move_to(text_x, text_y);
  pl1->show_in_cairo_context(cr);
  style_context->context_restore();

  return Gtk::Widget::on_draw(cr);
}

void
TimeBar::set_color(const Cairo::RefPtr<Cairo::Context> &cr, const Gdk::Color &color)
{
  cr->set_source_rgba(color.get_red_p(), color.get_green_p(), color.get_blue_p(), 1);
}

void
TimeBar::set_color(const Cairo::RefPtr<Cairo::Context> &cr, const Gdk::RGBA &color)
{
  cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 1);
}

void
TimeBar::draw_bar(const Cairo::RefPtr<Cairo::Context> &cr, int x, int y, int width, int height, int winw, int winh)
{
  (void)winh;

  if (rotation == 0 || rotation == 180)
    {
      cr->rectangle(x, y, width, height);
      cr->fill();
    }
  else
    {
      cr->rectangle(y, winw - x - width, height, width);
      cr->fill();
    }
}
