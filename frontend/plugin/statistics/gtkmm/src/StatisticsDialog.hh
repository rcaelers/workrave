// PreferencesDialog.hh --- Statistics Dialog
//
// Copyright (C) 2002, 2003, 2006, 2007, 2008 Rob Caelers & Raymond Penners
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

#ifndef STATISTICSDIALOG_HH
#define STATISTICSDIALOG_HH

#include "preinclude.h"
#include <sstream>

#include "IStatistics.hh"
#include "Hig.hh"

namespace Gtk
{
  class Label;
  class Button;
  class Calendar;
  class Notebook;
  class Widget;
}

using namespace workrave;

class StatisticsDialog : public HigDialog
{
public:
  StatisticsDialog();
  ~StatisticsDialog();

  static const int BREAK_STATS = 7;

  int run();

private:
  /** Stats */
  IStatistics *statistics;

  /** Labels for break stats. */
  Gtk::Label *break_labels[BREAK_ID_SIZEOF][9];

  /** Labels for break stats. */
  Gtk::Label *activity_labels[5];

  /** Labels for break stats. */
  Gtk::Label *daily_usage_label;

  /** Labels indicating the start time of the visible data. */
  Gtk::Label *date_label;

  /** Calendar */
  Gtk::Calendar *calendar;

  /** Forward button */
  Gtk::Button *forward_btn;

  /** Back button */
  Gtk::Button *back_btn;

  /** Last button */
  Gtk::Button *last_btn;

  /** First button */
  Gtk::Button *first_btn;

  /** Delete button */
  Gtk::Button *delete_btn;
  void on_history_delete_all();

  void init_gui();
  void select_day(int day);

  void create_break_page(Gtk::Widget *tnotebook);
  void create_activity_page(Gtk::Widget *tnotebook);

  void stream_distance(std::stringstream &stream, int64_t pixels);
  void get_calendar_day_index(int &idx, int &next, int &prev);
  void set_calendar_day_index(int idx);
  void on_calendar_month_changed();
  void on_calendar_day_selected();
  void on_history_go_back();
  void on_history_go_forward();
  void on_history_goto_last();
  void on_history_goto_first();
  void display_calendar_date();
  void display_statistics(IStatistics::DailyStats *stats);
  void clear_display_statistics();

  bool on_timer();
};

#endif // STATISTICSWINDOW_HH
