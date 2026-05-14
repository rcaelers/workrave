// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#ifndef TIMERBOXGTKVIEW_HH
#define TIMERBOXGTKVIEW_HH

#include <array>
#include <gtkmm.h>

#include "core/ICore.hh"
#include "utils/Signals.hh"
#include "ui/ITimerBoxView.hh"

#include "TimeBar.hh"

class EventImage;

namespace Gtk
{
  class Image;
  class Bin;
  class Image;
  class EventBox;
} // namespace Gtk

class TimerBoxGtkView
  : public Gtk::Table
  , public ITimerBoxView
  , public workrave::utils::Trackable
{
public:
  explicit TimerBoxGtkView(std::shared_ptr<workrave::ICore> core, bool transparent = false);
  ~TimerBoxGtkView() override;

  int get_visible_count() const;
  void set_slot(workrave::BreakId id, int slot) override;
  void set_time_bar(workrave::BreakId id,
                    int value,
                    TimerColorId primary_color,
                    int primary_value,
                    int primary_max,
                    TimerColorId secondary_color,
                    int secondary_value,
                    int secondary_max) override;
  void set_icon(OperationModeIcon icon) override;
  void update_view() override;
  void set_enabled(bool enabled);

  void set_geometry(Orientation orientation, int size);
  void set_sheep_only(bool sheep_only);
  bool is_sheep_only() const;

  bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;

private:
  void init_widgets();
  void init_table();
  void init();
  void update_widgets();

  int get_number_of_timers() const;

  std::shared_ptr<workrave::ICore> core;

  //! Use tranparentcy.
  bool transparent;

  //! Reconfigure the panel.
  bool reconfigure{true};

  //! Array of time labels
  std::array<Gtk::Widget *, workrave::BREAK_ID_SIZEOF> labels{};

  //! Array of time bar widgets.
  std::array<TimeBar *, workrave::BREAK_ID_SIZEOF> bars{};

  //! Break images
  std::array<Gtk::Image *, workrave::BREAK_ID_SIZEOF> images{};

  //! Sheep
  Gtk::Image *sheep{nullptr};

  //! Sheep
  Gtk::EventBox *sheep_eventbox{nullptr};

  //! orientation.
  Orientation orientation{ORIENTATION_VERTICAL};

  //! Size
  int size{0};

  //! Rows
  int table_rows{-1};

  //! Columns
  int table_columns{-1};

  //! Current slot content.
  std::array<int, workrave::BREAK_ID_SIZEOF> current_content{};

  //! New slot content.
  std::array<int, workrave::BREAK_ID_SIZEOF> new_content{};

  //! Number of visible breaks.
  int visible_count{-1};

  //! Only show the sheep
  bool sheep_only{false};
};

inline int
TimerBoxGtkView::get_visible_count() const
{
  return visible_count;
}

#endif // TIMERBOXGTKVIEW_HH
