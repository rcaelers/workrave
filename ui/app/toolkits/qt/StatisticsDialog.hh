// Copyright (C) 2002 - 2015 Rob Caelers & Raymond Penners
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

#include <QtGui>
#include <QtWidgets>

#include <sstream>
#include <memory>

#include "core/IStatistics.hh"
#include "ui/IApplicationContext.hh"

class StatisticsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit StatisticsDialog(std::shared_ptr<IApplicationContext> app);

  static const int BREAK_STATS = 7;

  auto run() -> int;

private:
  std::shared_ptr<IApplicationContext> app;
  workrave::IStatistics::Ptr statistics;

  QLabel *break_labels[workrave::BREAK_ID_SIZEOF][9];
  QLabel *activity_labels[5];
  QLabel *usage_label{nullptr};
  QLabel *daily_usage_time_label{nullptr};
  QLabel *weekly_usage_time_label{nullptr};
  QLabel *monthly_usage_time_label{nullptr};
  QLabel *date_label{nullptr};

  QCalendarWidget *calendar{nullptr};
  QPushButton *forward_button{nullptr};
  QPushButton *back_button{nullptr};
  QPushButton *last_button{nullptr};
  QPushButton *first_button{nullptr};
  QPushButton *delete_button{nullptr};

  bool update_usage_real_time{false};

  void on_history_delete_all();

  void init_gui();
  void select_day(int day);

  void create_navigation_box(QLayout *parent);
  void create_statistics_box(QLayout *parent);

  void create_break_page(QBoxLayout *parent);
  //  void create_activity_page(QWidget *tnotebook);

  void stream_distance(std::stringstream &stream, int64_t pixels);
  void get_calendar_day_index(int &idx, int &next, int &prev);
  void set_calendar_day_index(int idx);
  void on_calendar_month_changed(int year, int month);
  void on_calendar_day_selected(const QDate &date);
  void on_history_go_back();
  void on_history_go_forward();
  void on_history_goto_last();
  void on_history_goto_first();
  void display_calendar_date();
  void display_statistics(workrave::IStatistics::DailyStats *stats);
  void clear_display_statistics();
  void display_week_statistics();
  void display_month_statistics();
  auto on_timer() -> bool;
};

#endif // STATISTICSDIALOG_HH
