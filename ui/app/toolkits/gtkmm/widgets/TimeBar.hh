// Copyright (C) 2002 - 2011 Rob Caelers & Raymond Penners
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
#include <utility>

#include <gtkmm.h>
#include <gdkmm.h>
#include <gdkmm/types.h>

#include "ui/UiTypes.hh"

class TimeBar : public Gtk::DrawingArea
{
public:
  explicit TimeBar(const std::string &name = "");
  ~TimeBar() override = default;

  void set_progress(int value, int max_value);
  void set_secondary_progress(int value, int max_value);

  void set_text(std::string text);

  void update();
  void set_bar_color(TimerColorId color);
  void set_secondary_bar_color(TimerColorId color);
  void set_text_alignment(int align);

  void get_minimum_size(int &width, int &height) const;
  void get_preferred_size(int &width, int &height) const;

private:
  struct Bar
  {
    int x;
    int y;
    int width;
    int height;
    TimerColorId color;

    Bar()
      : x(0)
      , y(0)
      , width(0)
      , height(0)
      , color(TimerColorId::Bg)
    {
    }

    Bar(int x, int y, int width, int height, TimerColorId color)
      : x(x)
      , y(y)
      , width(width)
      , height(height)
      , color(color)
    {
    }
  };

  void draw_bar(const Cairo::RefPtr<Cairo::Context> &cr, const Bar &bar) const;
  void draw_frame(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);
  std::array<Bar, 2> calc_bars(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);
  void draw_bars(const Cairo::RefPtr<Cairo::Context> &cr, const std::array<Bar, 2> &bars);
  void draw_text(const Cairo::RefPtr<Cairo::Context> &cr, const std::array<Bar, 2> &bars, int width, int win_h);

  void set_color(const Cairo::RefPtr<Cairo::Context> &cr, const Gdk::RGBA &color) const;

protected:
  Gtk::SizeRequestMode get_request_mode_vfunc() const override;
  void get_preferred_width_vfunc(int &minimum_width, int &natural_width) const override;
  void get_preferred_height_vfunc(int &minimum_height, int &natural_height) const override;
  void get_preferred_width_for_height_vfunc(int height, int &minimum_width, int &natural_width) const override;
  void get_preferred_height_for_width_vfunc(int width, int &minimum_height, int &natural_height) const override;
  void on_size_allocate(Gtk::Allocation &allocation) override;
  bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;

private:
  std::map<TimerColorId, Gdk::RGBA> bar_colors;
  std::map<TimerColorId, Gdk::RGBA> bar_text_colors;

  //! Color of the time-bar.
  TimerColorId bar_color{};

  //! Color of the time-bar.
  TimerColorId secondary_bar_color{};

  //! The current value.
  int bar_value{0};

  //! The maximum value.
  int bar_max_value{0};

  //! The current value.
  int secondary_bar_value{0};

  //! The maximum value.
  int secondary_bar_max_value{0};

  //! Text to show;
  std::string bar_text;

  //! Text alignment
  int bar_text_align{0};
};

#endif // TIMEBAR_HH
