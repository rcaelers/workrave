// TimerBoxtGtkView.hh --- All timers
//
// Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef TIMERBOXGTKVIEW_HH
#define TIMERBOXGTKVIEW_HH

#include "preinclude.h"

#include <string>
#include <gtkmm/table.h>
#include <gtkmm/tooltips.h>

#include "ITimerBoxView.hh"
#include "TimeBar.hh"

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
  TimerBoxGtkView();
  ~TimerBoxGtkView();

  void set_geometry(Orientation orientation, int size);
  int get_visible_count() const;
  void set_slot(BreakId  id, int slot);
  void set_time_bar(BreakId id,
                            std::string text, TimeBar::ColorId primary_color,
                            int primary_value, int primary_max,
                            TimeBar::ColorId secondary_color,
                            int secondary_value, int secondary_max);
  void set_tip(std::string tip);
  void set_icon(IconType icon);
  void update_view();
  void set_enabled(bool enabled);


private:
  void init_widgets();
  void init_table();
  void init();

  //! Reconfigure the panel.
  bool reconfigure;

  //! Parent container
  Gtk::Bin *parent;

  //! Tooltips
  Gtk::Tooltips *tooltips;

  //! Array of time labels
  Gtk::Widget **labels;

  //! Array of time bar widgets.
  TimeBar **bars;

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
  int current_content[BREAK_ID_SIZEOF];

  //! New slot content.
  int new_content[BREAK_ID_SIZEOF];

  //! Number of visible breaks.
  int visible_count;

  //! Rotation (clockwise in degress)
  int rotation;
};


inline int
TimerBoxGtkView::get_visible_count() const
{
  return visible_count;
}


#endif // TIMERBOXGTKVIEW_HH
