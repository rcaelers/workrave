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

#include <algorithm>
#include <cassert>
#include <cstdlib>

#include "TimeBar.hh"
#include "commonui/Text.hh"
#include "ui/UiTypes.hh"
#include "GtkUtil.hh"

const int MARGINX = 4;
const int MARGINY = 2;
const int MIN_HORIZONTAL_BAR_HEIGHT = 20; // stolen from gtk's progress bar

using namespace std;

TimeBar::TimeBar(const std::string &name)
  : bar_colors{
      {TimerColorId::Active, Gdk::RGBA("lightblue")},
      {TimerColorId::Inactive, Gdk::RGBA("lightgreen")},
      {TimerColorId::Overdue, Gdk::RGBA("orange")},
      {TimerColorId::InactiveOverActive, Gdk::RGBA("#00d4b2")},
      {TimerColorId::InactiveOverOverdue, Gdk::RGBA("lightgreen")},
      {TimerColorId::Bg, Gdk::RGBA("#777777")},
    },
    bar_text_colors{
      {TimerColorId::Active, Gdk::RGBA("black")},
      {TimerColorId::Inactive, Gdk::RGBA("black")},
      {TimerColorId::Overdue, Gdk::RGBA("black")},
      {TimerColorId::InactiveOverActive, Gdk::RGBA("black")},
      {TimerColorId::InactiveOverOverdue, Gdk::RGBA("black")},
      {TimerColorId::Bg, Gdk::RGBA("black")},
    }
{
  add_events(Gdk::EXPOSURE_MASK);
  add_events(Gdk::BUTTON_PRESS_MASK);

  set_bar_color(TimerColorId::Inactive);
  set_secondary_bar_color(TimerColorId::Inactive);

  GtkUtil::set_theme_fg_color(this);

  GtkUtil::override_bg_color("workrave-timebar", name, bar_colors[TimerColorId::Bg]);
  GtkUtil::override_bg_color("workrave-timebar-active", name, bar_colors[TimerColorId::Active]);
  GtkUtil::override_bg_color("workrave-timebar-inactive", name, bar_colors[TimerColorId::Inactive]);
  GtkUtil::override_bg_color("workrave-timebar-overdue", name, bar_colors[TimerColorId::Overdue]);
  GtkUtil::override_bg_color("workrave-timebar-inactive-over-active", name, bar_colors[TimerColorId::InactiveOverActive]);
  GtkUtil::override_bg_color("workrave-timebar-inactive-over-overdue", name, bar_colors[TimerColorId::InactiveOverOverdue]);

  GtkUtil::override_color("workrave-timebar", name, bar_text_colors[TimerColorId::Bg]);
  GtkUtil::override_color("workrave-timebar-active", name, bar_text_colors[TimerColorId::Active]);
  GtkUtil::override_color("workrave-timebar-inactive", name, bar_text_colors[TimerColorId::Inactive]);
  GtkUtil::override_color("workrave-timebar-overdue", name, bar_text_colors[TimerColorId::Overdue]);
  GtkUtil::override_color("workrave-timebar-inactive-over-active", name, bar_text_colors[TimerColorId::InactiveOverActive]);
  GtkUtil::override_color("workrave-timebar-inactive-over-overdue", name, bar_text_colors[TimerColorId::InactiveOverOverdue]);
}

void
TimeBar::set_progress(int value, int max_value)
{
  value = std::min(value, max_value);

  bar_value = value;
  bar_max_value = max_value;
}

void
TimeBar::set_secondary_progress(int value, int max_value)
{
  value = std::min(value, max_value);

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

  string min_string = Text::time_to_string(-(59 + (59 * 60) + (9 * 60 * 60)));
  Glib::RefPtr<Pango::Layout> plmin = const_cast<TimeBar *>(this)->create_pango_layout(min_string);

  Glib::RefPtr<Pango::Context> pcl = pl->get_context();
  Glib::RefPtr<Pango::Context> pcmin = plmin->get_context();

  pl->get_pixel_size(width, height);

  int mwidth = 0;
  int mheight = 0;
  plmin->get_pixel_size(mwidth, mheight);
  width = std::max(mwidth, width);
  height = std::max(mheight, height);

  width = width + 2 * MARGINX;
  height = max(height + (2 * MARGINY), MIN_HORIZONTAL_BAR_HEIGHT);
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
  minimum_width = natural_width = width;
}

void
TimeBar::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
  int width = 0;
  int height = 0;
  get_preferred_size(width, height);
  minimum_height = natural_height = height;
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

void
TimeBar::draw_frame(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height)
{
  // Draw background
  Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();
  style_context->set_state(Gtk::STATE_FLAG_ACTIVE);
  Gdk::RGBA back_color = style_context->get_background_color();
  set_color(cr, back_color);

  // clip to the area indicated by the expose event so that we only redraw
  // the portion of the window that needs to be redrawn
  cr->rectangle(0, 0, width, height);
  cr->clip();
  style_context->render_background(cr, 0, 0, width - 1, height - 1);
  style_context->render_frame(cr, 0, 0, width - 1, height - 1);
}

std::array<TimeBar::Bar, 2>
TimeBar::calc_bars(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height)
{
  const int border_size = 1;
  std::array<Bar, 2> bars;

  // Bar
  int bar_width = 0;
  if (bar_max_value > 0)
    {
      bar_width = (bar_value * (width - 2 * border_size - 1)) / bar_max_value;
    }

  // Secondary bar
  int sbar_width = 0;
  if (secondary_bar_max_value > 0)
    {
      sbar_width = (secondary_bar_value * (width - 2 * border_size - 1)) / secondary_bar_max_value;
    }

  int bar_height = height - (2 * border_size) - 1;

  if (sbar_width > 0)
    {
      // Overlap

      // We should assert() because this is not supported
      // but there are some weird boundary cases
      // in which this still happens.. need to check
      // this out some time.
      // assert(secondary_bar_color == TimerColorId::Inactive);
      TimerColorId overlap_color{TimerColorId::Bg};
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
          if (bar_width != 0)
            {
              bars[0] = Bar(border_size, border_size, bar_width, bar_height, overlap_color);
            }
          if (sbar_width > bar_width)
            {
              bars[1] = Bar(border_size + bar_width, border_size, sbar_width - bar_width, bar_height, secondary_bar_color);
            }
        }
      else
        {
          if (sbar_width != 0)
            {
              bars[0] = Bar(border_size, border_size, sbar_width, bar_height, overlap_color);
            }
          if (bar_width > sbar_width)
            {
              bars[1] = Bar(border_size + sbar_width, border_size, bar_width - sbar_width, bar_height, bar_color);
            }
        }
    }
  else
    {
      bars[0] = Bar(border_size, border_size, bar_width, bar_height, bar_color);
    }
  return bars;
}

void
TimeBar::draw_bars(const Cairo::RefPtr<Cairo::Context> &cr, const std::array<Bar, 2> &bars)
{
  for (const auto &bar: bars)
    {
      if (bar.width > 0 && bar.height > 0)
        {
          draw_bar(cr, bar);
        }
    }
}

void
TimeBar::draw_text(const Cairo::RefPtr<Cairo::Context> &cr, const std::array<Bar, 2> &bars, int width, int height)
{
  Glib::RefPtr<Pango::Layout> pl1 = create_pango_layout(bar_text);
  Glib::RefPtr<Pango::Context> pc1 = pl1->get_context();

  int text_width = 0;
  int text_height = 0;
  pl1->get_pixel_size(text_width, text_height);

  int text_x = 0;
  int text_y = 0;

  if (width - text_width - MARGINX > 0)
    {
      if (bar_text_align > 0)
        {
          text_x = (width - text_width - MARGINX);
        }
      else if (bar_text_align < 0)
        {
          text_x = MARGINX;
        }
      else
        {
          text_x = (width - text_width) / 2;
        }
    }
  else
    {
      text_x = MARGINX;
    }
  text_y = (height - text_height) / 2;

  cr->reset_clip();

  Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();
  set_color(cr, style_context->get_color(Gtk::STATE_FLAG_ACTIVE));
  cr->move_to(text_x, text_y);
  pl1->show_in_cairo_context(cr);

  for (const auto &bar: bars)
    {
      if (bar.height > 0 && bar.width > 0)
        {
          Gdk::RGBA color = bar_text_colors[bar.color];
          cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 1);

          cr->reset_clip();
          cr->rectangle(bar.x, bar.y, bar.width, bar.height);
          cr->clip();

          cr->move_to(text_x, text_y);
          pl1->show_in_cairo_context(cr);
        }
    }
}

bool
TimeBar::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
  Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();
  Gtk::Allocation allocation = get_allocation();

  style_context->context_save();
  style_context->add_class(GTK_STYLE_CLASS_FRAME);

  // Physical width/height
  int win_w = allocation.get_width() - 2;
  int win_h = allocation.get_height();

  auto bars = calc_bars(cr, win_w, win_h);
  draw_frame(cr, win_w, win_h);
  draw_bars(cr, bars);
  draw_text(cr, bars, win_w, win_h);

  style_context->context_restore();

  return Gtk::Widget::on_draw(cr);
}

void
TimeBar::set_color(const Cairo::RefPtr<Cairo::Context> &cr, const Gdk::RGBA &color) const
{
  cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 1);
}

void
TimeBar::draw_bar(const Cairo::RefPtr<Cairo::Context> &cr, const Bar &bar) const
{
  Gdk::RGBA color = bar_colors.at(bar.color);
  cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 1);
  cr->rectangle(bar.x, bar.y, bar.width, bar.height);
  cr->fill();
}
