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

#include <string>
#include <gtkmm.h>

#include "utils/Signals.hh"
#include "ui/ITimerBoxView.hh"
#include "ui/IApplication.hh"

#include "TimeBar.hh"
//#include "Menus.hh"

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
  TimerBoxGtkView(std::shared_ptr<IApplication> app, bool transparent = false);
  ~TimerBoxGtkView() override;

  void set_geometry(Orientation orientation, int size) override;
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
  void set_tip(std::string tip) override;
  void set_icon(OperationModeIcon icon) override;
  void update_view() override;
  void set_enabled(bool enabled);

  void set_sheep_only(bool sheep_only);
  bool is_sheep_only() const;

  bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;

private:
  void init_widgets();
  void init_table();
  void init();
  void update_widgets();

  int get_number_of_timers() const;

  std::shared_ptr<IApplication> app;

  //! Use tranparentcy.
  bool transparent;

  //! Reconfigure the panel.
  bool reconfigure{true};

  //! Array of time labels
  Gtk::Widget *labels[workrave::BREAK_ID_SIZEOF]{};

  //! Array of time bar widgets.
  TimeBar *bars[workrave::BREAK_ID_SIZEOF]{};

  //! Break images
  Gtk::Image *images[workrave::BREAK_ID_SIZEOF]{};

  //! Sheep
  Gtk::Image *sheep{nullptr};

  //! Sheep
  Gtk::EventBox *sheep_eventbox{nullptr};

  //! orientation.
  Orientation orientation{ORIENTATION_UP};

  //! Size
  int size{0};

  //! Rows
  int table_rows{-1};

  //! Columns
  int table_columns{-1};

  //! Reverse
  bool table_reverse{false};

  //! Current slot content.
  int current_content[workrave::BREAK_ID_SIZEOF]{};

  //! New slot content.
  int new_content[workrave::BREAK_ID_SIZEOF]{};

  //! Number of visible breaks.
  int visible_count{-1};

  //! Rotation (clockwise in degrees)
  int rotation{0};

  //! Only show the sheep
  bool sheep_only{false};
};

inline int
TimerBoxGtkView::get_visible_count() const
{
  return visible_count;
}

#endif // TIMERBOXGTKVIEW_HH
