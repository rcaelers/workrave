// TimerBoxtGtkView.hh --- All timers
//
// Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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

#include "commonui/ITimerBoxView.hh"
#include "TimeBar.hh"
#include "Menus.hh"

#include "utils/ScopedConnections.hh"

class EventImage;

namespace Gtk
{
  class Image;
  class Bin;
  class Image;
  class EventBox;
}

class TimerBoxGtkView : public Gtk::Table, public ITimerBoxView
{
public:
  TimerBoxGtkView(Menus::MenuKind menu, bool transparent = false);
  virtual ~TimerBoxGtkView();

  void set_geometry(Orientation orientation, int size);
  int get_visible_count() const;
  void set_slot(workrave::BreakId  id, int slot);
  void set_time_bar(workrave::BreakId id,
                    int value,
                    TimerColorId primary_color,
                    int primary_value, int primary_max,
                    TimerColorId secondary_color,
                    int secondary_value, int secondary_max);
  void set_tip(std::string tip);
  void set_icon(StatusIconType icon);
  void update_view();
  void set_enabled(bool enabled);

  void set_sheep_only(bool sheep_only);
  bool is_sheep_only() const;

  virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

private:
  void init_widgets();
  void init_table();
  void init();
  void update_widgets();

  bool on_restbreak_button_press_event(int button);
  int get_number_of_timers() const;

  //! What menu to active on click
  Menus::MenuKind menu;

  //! Use tranparentcy.
  bool transparent;

  //! Reconfigure the panel.
  bool reconfigure;

  //! Array of time labels
  Gtk::Widget *labels[BREAK_ID_SIZEOF];

  //! Array of time bar widgets.
  TimeBar *bars[BREAK_ID_SIZEOF];

  //! Break images
  Gtk::Image *images[BREAK_ID_SIZEOF];

  //! Sheep
  Gtk::Image *sheep;

  //! Sheep
  Gtk::EventBox *sheep_eventbox;

  //! orientation.
  Orientation orientation;

  //! Size
  int size;

  //! Rows
  int table_rows;

  //! Columns
  int table_columns;

  //! Reverse
  int table_reverse;

  //! Current slot content.
  int current_content[workrave::BREAK_ID_SIZEOF];

  //! New slot content.
  int new_content[workrave::BREAK_ID_SIZEOF];

  //! Number of visible breaks.
  int visible_count;

  //! Rotation (clockwise in degress)
  int rotation;

  //! Only show the sheep
  bool sheep_only;

  scoped_connections connections;
};


inline int
TimerBoxGtkView::get_visible_count() const
{
  return visible_count;
}


#endif // TIMERBOXGTKVIEW_HH
