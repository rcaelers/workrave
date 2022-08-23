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

#include <gtkmm.h>
#include <gdkmm.h>
#include <gdkmm/types.h>

#include "ui/UiTypes.hh"

class TimeBar : public Gtk::DrawingArea
{
public:
  TimeBar();
  ~TimeBar() override = default;

  void set_progress(int value, int max_value);
  void set_secondary_progress(int value, int max_value);

  void set_text(std::string text);

  void update();
  void set_bar_color(TimerColorId color);
  void set_secondary_bar_color(TimerColorId color);
  void set_text_alignment(int align);

  void set_border_size(int size);
  void set_rotation(int r);

  void get_minimum_size(int &width, int &height) const;
  void get_preferred_size(int &width, int &height) const;

private:
  void draw_bar(const Cairo::RefPtr<Cairo::Context> &cr, int x, int y, int width, int height, int winw, int winh);
  void set_color(const Cairo::RefPtr<Cairo::Context> &cr, const Gdk::RGBA &color);
  void set_color(const Cairo::RefPtr<Cairo::Context> &cr, const Gdk::RGBA &color);
  void set_text_color(Gdk::RGBA color);

protected:
  Gtk::SizeRequestMode get_request_mode_vfunc() const override;
  void get_preferred_width_vfunc(int &minimum_width, int &natural_width) const override;
  void get_preferred_height_vfunc(int &minimum_height, int &natural_height) const override;
  void get_preferred_width_for_height_vfunc(int height, int &minimum_width, int &natural_width) const override;
  void get_preferred_height_for_width_vfunc(int width, int &minimum_height, int &natural_height) const override;
  void on_size_allocate(Gtk::Allocation &allocation) override;
  bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;

private:
  static std::map<TimerColorId, Gdk::RGBA> bar_colors;

  //! Color of the time-bar.
  TimerColorId bar_color;

  //! Color of the time-bar.
  TimerColorId secondary_bar_color;

  //! Color of the text.
  Gdk::RGBA bar_text_color;

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

  //! Bar rotation (clockwise degrees)
  int rotation{0};
};

#endif // TIMEBAR_HH
