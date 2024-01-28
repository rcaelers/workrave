// Copyright (C) 2002 - 2012 Rob Caelers & Raymond Penners
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

#include <sstream>

#include "core/IStatistics.hh"
#include "Hig.hh"
#include "ui/IApplicationContext.hh"

namespace Gtk
{
  class Label;
  class Button;
  class Calendar;
  class Notebook;
  class Widget;
} // namespace Gtk

class StatisticsDialog : public HigDialog
{
public:
  explicit StatisticsDialog(std::shared_ptr<IApplicationContext> app);
  ~StatisticsDialog() override = default;

  static const int BREAK_STATS = 7;

  int run();

private:
  std::shared_ptr<IApplicationContext> app;

  /** Stats */
  workrave::IStatistics *statistics{nullptr};

  /** Labels for break stats. */
  Gtk::Label *break_labels[workrave::BREAK_ID_SIZEOF][9]{};

  /** Labels for break stats. */
  Gtk::Label *activity_labels[5]{};

  /** Usage label */
  Gtk::Label *usage_label{nullptr};

  /** Daily time. */
  Gtk::Label *daily_usage_time_label{nullptr};

  /** Weekly time. */
  Gtk::Label *weekly_usage_time_label{nullptr};

  /** Monthly time */
  Gtk::Label *monthly_usage_time_label{nullptr};

  /** Labels indicating the start time of the visible data. */
  Gtk::Label *date_label{nullptr};

  /** Calendar */
  Gtk::Calendar *calendar{nullptr};

  /** Forward button */
  Gtk::Button *forward_btn{nullptr};

  /** Back button */
  Gtk::Button *back_btn{nullptr};

  /** Last button */
  Gtk::Button *last_btn{nullptr};

  /** First button */
  Gtk::Button *first_btn{nullptr};

  /** Delete button */
  Gtk::Button *delete_btn{nullptr};

  bool update_usage_real_time{false};

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
  void display_statistics(workrave::IStatistics::DailyStats *stats);
  void clear_display_statistics();
  void display_week_statistics();
  void display_month_statistics();
  bool on_timer();
};

#endif // STATISTICSWINDOW_HH
