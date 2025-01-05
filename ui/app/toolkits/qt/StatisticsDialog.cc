// Copyright (C) 2002 - 2013 Rob Caelers <robc@krandor.nl>
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

#include <sstream>
#include <ctime>
#include <cstring>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "core/ICore.hh"

#include "StatisticsDialog.hh"

#include "UiUtil.hh"
#include "qformat.hh"

using namespace workrave;

StatisticsDialog::StatisticsDialog(std::shared_ptr<IApplicationContext> app)
  : app(app)
{
  auto core = app->get_core();
  statistics = core->get_statistics();

  for (auto &activity_label: activity_labels)
    {
      activity_label = nullptr;
    }

  init_gui();
  display_calendar_date();
}

auto
StatisticsDialog::run() -> int
{
  // Periodic timer.
  auto *timer = new QTimer(this);
  timer->setInterval(1000);
  connect(timer, SIGNAL(timeout()), this, SLOT(on_timer()));
  timer->start();

  return 0;
}

void
StatisticsDialog::init_gui()
{
  auto *layout = new QVBoxLayout();
  layout->setContentsMargins(1, 1, 1, 1);
  setLayout(layout);

  auto *main_layout = new QHBoxLayout();
  main_layout->setContentsMargins(1, 1, 1, 1);
  layout->addLayout(main_layout);

  create_navigation_box(main_layout);
  create_statistics_box(main_layout);

  auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
  layout->addWidget(buttonBox);
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(accept()));

  auto *notebook = new QTabWidget();
  notebook->setTabPosition(QTabWidget::West);
  notebook->setIconSize(QSize(100, 100));
  layout->addWidget(notebook);
}

void
StatisticsDialog::create_navigation_box(QLayout *parent)
{
  auto *box = new QGroupBox(tr("Browse history"));
  auto *layout = new QVBoxLayout;
  box->setLayout(layout);
  parent->addWidget(box);

  auto *button_box = new QHBoxLayout();

  first_button = new QPushButton;
  first_button->setIcon(QIcon::fromTheme("go-first", UiUtil::create_icon("go-first-symbolic.svg")));
  connect(first_button, &QPushButton::clicked, this, &StatisticsDialog::on_history_goto_first);
  button_box->addWidget(first_button, QDialogButtonBox::ActionRole);

  back_button = new QPushButton;
  back_button->setIcon(QIcon::fromTheme("go-previous", UiUtil::create_icon("go-previous-symbolic.svg")));
  connect(back_button, &QPushButton::clicked, this, &StatisticsDialog::on_history_go_back);
  button_box->addWidget(back_button, QDialogButtonBox::ActionRole);

  forward_button = new QPushButton;
  forward_button->setIcon(QIcon::fromTheme("go-next", UiUtil::create_icon("go-next-symbolic.svg")));
  connect(forward_button, &QPushButton::clicked, this, &StatisticsDialog::on_history_go_forward);
  button_box->addWidget(forward_button, QDialogButtonBox::ActionRole);

  last_button = new QPushButton;
  last_button->setIcon(QIcon::fromTheme("go-last", UiUtil::create_icon("go-last-symbolic.svg")));
  connect(last_button, &QPushButton::clicked, this, &StatisticsDialog::on_history_goto_last);
  button_box->addWidget(last_button, QDialogButtonBox::ActionRole);

  layout->addLayout(button_box);

  calendar = new QCalendarWidget();
  connect(calendar, &QCalendarWidget::clicked, this, &StatisticsDialog::on_calendar_day_selected);
  connect(calendar, &QCalendarWidget::currentPageChanged, this, &StatisticsDialog::on_calendar_month_changed);
  layout->addWidget(calendar);

  delete_button = new QPushButton(tr("Delete all statistics history"));
  delete_button->setIcon(QIcon::fromTheme("edit-delete"));
  connect(delete_button, &QPushButton::clicked, this, &StatisticsDialog::on_history_delete_all);

  layout->addWidget(calendar);
}

void
StatisticsDialog::create_statistics_box(QLayout *parent)
{
  auto *box = new QGroupBox(tr("Statistics"));
  auto *layout = new QVBoxLayout;
  box->setLayout(layout);
  parent->addWidget(box);

  date_label = UiUtil::add_label(layout, tr("Date:"));

  create_break_page(layout);
}

void
StatisticsDialog::create_break_page(QBoxLayout *parent)
{
  auto *table = new QGridLayout();
  parent->addLayout(table);

  QWidget *unique_label = UiUtil::create_label_with_tooltip(tr("Break prompts"),
                                                            tr("The number of times you were prompted to break, excluding"
                                                               " repeated prompts for the same break"));

  QWidget *prompted_label = UiUtil::create_label_with_tooltip(tr("Repeated prompts"),
                                                              tr("The number of times you were repeatedly prompted to break"));

  QWidget *taken_label = UiUtil::create_label_with_tooltip(tr("Prompted breaks taken"),
                                                           tr("The number of times you took a break when being prompted"));

  QWidget *natural_label = UiUtil::create_label_with_tooltip(tr("Natural breaks taken"),
                                                             tr("The number of times you took a break without being prompted"));

  QWidget *skipped_label = UiUtil::create_label_with_tooltip(tr("Breaks skipped"), tr("The number of breaks you skipped"));

  QWidget *postponed_label = UiUtil::create_label_with_tooltip(tr("Breaks postponed"), tr("The number of breaks you postponed"));

  QWidget *overdue_label = UiUtil::create_label_with_tooltip(tr("Overdue time"), tr("The total time this break was overdue"));

  QWidget *usage_label = UiUtil::create_label_with_tooltip(tr("Usage"), ("Active computer usage"));

  QWidget *daily_usage_label = UiUtil::create_label_with_tooltip(tr("Daily"),
                                                                 tr("The total computer usage for the selected day"));

  QWidget *weekly_usage_label = UiUtil::create_label_with_tooltip(
    tr("Weekly"),
    tr("The total computer usage for the whole week of the selected day"));

  QWidget *monthly_usage_label = UiUtil::create_label_with_tooltip(
    tr("Monthly"),
    tr("The total computer usage for the whole month of the selected day"));

  auto *hrule = new QFrame();
  auto *vrule = new QFrame();

  hrule->setFrameShape(QFrame::HLine);
  hrule->setFrameShadow(QFrame::Sunken);
  vrule->setFrameShape(QFrame::VLine);
  vrule->setFrameShadow(QFrame::Sunken);

  int y = 0;

  QWidget *mp_label = UiUtil::create_label_for_break(BREAK_ID_MICRO_BREAK);
  QWidget *rb_label = UiUtil::create_label_for_break(BREAK_ID_REST_BREAK);
  QWidget *dl_label = UiUtil::create_label_for_break(BREAK_ID_DAILY_LIMIT);

  y = 0;
  table->addWidget(mp_label, y, 2);
  table->addWidget(rb_label, y, 3);
  table->addWidget(dl_label, y, 4);
  table->addWidget(vrule, y, 1, 9, 1);

  y = 1;
  table->addWidget(hrule, y, 0, 1, 5);

  y = 2;
  table->addWidget(unique_label, y++, 0);
  table->addWidget(prompted_label, y++, 0);
  table->addWidget(taken_label, y++, 0);
  table->addWidget(natural_label, y++, 0);
  table->addWidget(skipped_label, y++, 0);
  table->addWidget(postponed_label, y++, 0);
  table->addWidget(overdue_label, y++, 0);

  hrule = new QFrame();
  hrule->setFrameShape(QFrame::HLine);
  hrule->setFrameShadow(QFrame::Raised);

  table->addWidget(hrule, y, 0, 1, 5);
  y += 2;

  daily_usage_time_label = new QLabel();
  weekly_usage_time_label = new QLabel();
  monthly_usage_time_label = new QLabel();

  vrule = new QFrame();
  vrule->setFrameShape(QFrame::VLine);
  vrule->setFrameShadow(QFrame::Sunken);
  table->addWidget(vrule, y, 1, 3, 1);

  table->addWidget(daily_usage_label, y, 2);
  table->addWidget(weekly_usage_label, y, 3);
  table->addWidget(monthly_usage_label, y, 4);
  y++;

  hrule = new QFrame();
  hrule->setFrameShape(QFrame::HLine);
  hrule->setFrameShadow(QFrame::Sunken);
  table->addWidget(hrule, y, 0, 1, 5);
  y++;

  table->addWidget(usage_label, y, 0);
  table->addWidget(daily_usage_time_label, y, 2);
  table->addWidget(weekly_usage_time_label, y, 3);
  table->addWidget(monthly_usage_time_label, y, 4);

  // Put the breaks in table.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      for (int j = 0; j < BREAK_STATS; j++)
        {
          break_labels[i][j] = new QLabel();
          table->addWidget(break_labels[i][j], j + 2, i + 2);
        }
    }
}

void
StatisticsDialog::display_statistics(IStatistics::DailyStats *stats)
{
  IStatistics::DailyStats empty{};
  bool is_empty{};

  is_empty = stats == nullptr;
  if (is_empty)
    {
      stats = &empty;
    }

  if (stats->start.tm_year == 0 /*stats->is_empty() */)
    {
      date_label->setText("-");
    }
  else
    {
      std::stringstream ss;
      ss.imbue(std::locale(ss.getloc(), new boost::posix_time::time_facet("%x")));
      boost::posix_time::ptime pt = boost::posix_time::ptime_from_tm(stats->start);
      ss << pt;
      std::string date = ss.str();

      ss.imbue(std::locale(ss.getloc(), new boost::posix_time::time_facet("%X")));
      ss << pt;
      std::string start = ss.str();
      pt = boost::posix_time::ptime_from_tm(stats->stop);
      ss << pt;
      std::string stop = ss.str();

      QString text = qstr(qformat(tr("%s, from %s to %s")) % date % start % stop);

      date_label->setText(text);
    }

  int64_t value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_ACTIVE_TIME];
  daily_usage_time_label->setText(UiUtil::time_to_string(value));

  // Put the breaks in table.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_UNIQUE_BREAKS];
      break_labels[i][0]->setText(QString::number(value));

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_PROMPTED] - value;
      break_labels[i][1]->setText(QString::number(value));

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_TAKEN];
      break_labels[i][2]->setText(QString::number(value));

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_NATURAL_TAKEN];
      break_labels[i][3]->setText(QString::number(value));

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_SKIPPED];
      break_labels[i][4]->setText(QString::number(value));

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_POSTPONED];
      break_labels[i][5]->setText(QString::number(value));

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_TOTAL_OVERDUE];

      break_labels[i][6]->setText(UiUtil::time_to_string(value));
    }
}

void
StatisticsDialog::display_week_statistics()
{
  QDate date = calendar->selectedDate();
  int y = date.year();
  int m = date.month() - 1;
  int d = date.day();

  std::tm timeinfo{};
  std::memset(&timeinfo, 0, sizeof(timeinfo));
  timeinfo.tm_mday = d;
  timeinfo.tm_mon = m;
  timeinfo.tm_year = y - 1900;

  std::time_t t = std::mktime(&timeinfo);
  std::tm const *time_loc = std::localtime(&t);

  QLocale locale;
  int week_start = locale.firstDayOfWeek() % 7;

  int offset = (time_loc->tm_wday - week_start + 7) % 7;
  int64_t total_week = 0;
  for (int i = 0; i < 7; i++)
    {
      std::memset(&timeinfo, 0, sizeof(timeinfo));
      timeinfo.tm_mday = d - offset + i;
      timeinfo.tm_mon = m;
      timeinfo.tm_year = y - 1900;
      t = std::mktime(&timeinfo);
      time_loc = std::localtime(&t);

      int idx = 0;
      int next = 0;
      int prev = 0;
      statistics->get_day_index_by_date(time_loc->tm_year + 1900, time_loc->tm_mon + 1, time_loc->tm_mday, idx, next, prev);

      if (idx >= 0)
        {
          IStatistics::DailyStats *stats = statistics->get_day(idx);
          if (stats != nullptr)
            {
              total_week += stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_ACTIVE_TIME];
            }

          update_usage_real_time |= (idx == 0);
        }
    }

  weekly_usage_time_label->setText(total_week > 0 ? UiUtil::time_to_string(total_week) : "");
}

void
StatisticsDialog::display_month_statistics()
{
  QDate date = calendar->selectedDate();
  int y = date.year();
  int m = date.month() - 1;

  int max_mday = 0;
  if (m == 3 || m == 5 || m == 8 || m == 10)
    {
      max_mday = 30;
    }
  else if (m == 1)
    {
      bool is_leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
      if (is_leap)
        {
          max_mday = 29;
        }
      else
        {
          max_mday = 28;
        }
    }
  else
    {
      max_mday = 31;
    }

  int64_t total_month = 0;
  for (int i = 1; i <= max_mday; i++)
    {
      int idx = 0;
      int next = 0;
      int prev = 0;
      statistics->get_day_index_by_date(y, m + 1, i, idx, next, prev);
      if (idx >= 0)
        {
          IStatistics::DailyStats *stats = statistics->get_day(idx);
          if (stats != nullptr)
            {
              total_month += stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_ACTIVE_TIME];
            }

          update_usage_real_time |= (idx == 0);
        }
    }

  monthly_usage_time_label->setText(total_month > 0 ? UiUtil::time_to_string(total_month) : "");
}

void
StatisticsDialog::clear_display_statistics()
{
  date_label->setText("");
  daily_usage_time_label->setText("");
  weekly_usage_time_label->setText("");
  monthly_usage_time_label->setText("");

  for (auto &break_label: break_labels)
    {
      for (int j = 0; j <= 6; j++)
        {
          break_label[j]->setText("");
        }
    }
  for (int i = 0; i <= 4; i++)
    {
      if (activity_labels[i] != nullptr)
        {
          activity_labels[i]->setText("");
        }
    }
}

void
StatisticsDialog::on_calendar_month_changed(int year, int month)
{
  display_calendar_date();
}

void
StatisticsDialog::on_calendar_day_selected(const QDate &date)
{
  display_calendar_date();
}

void
StatisticsDialog::get_calendar_day_index(int &idx, int &next, int &prev)
{
  QDate date = calendar->selectedDate();
  statistics->get_day_index_by_date(date.year(), date.month(), date.day(), idx, next, prev);
}

void
StatisticsDialog::set_calendar_day_index(int idx)
{
  IStatistics::DailyStats *stats = statistics->get_day(idx);
  QDate date = calendar->selectedDate();
  date.setDate(stats->start.tm_year + 1900, stats->start.tm_mon + 1, stats->start.tm_mday);
  calendar->setSelectedDate(date);
  display_calendar_date();
}

void
StatisticsDialog::display_calendar_date()
{
  int idx = 0;
  int next = 0;
  int prev = 0;
  get_calendar_day_index(idx, next, prev);
  IStatistics::DailyStats *stats = nullptr;
  if (idx >= 0)
    {
      stats = statistics->get_day(idx);
      display_statistics(stats);
    }
  else
    {
      clear_display_statistics();
    }
  update_usage_real_time = false;
  display_week_statistics();
  display_month_statistics();
  forward_button->setEnabled(next >= 0);
  back_button->setEnabled(prev >= 0);
  last_button->setEnabled(idx != 0);
  first_button->setEnabled(idx != statistics->get_history_size());
}

void
StatisticsDialog::on_history_go_back()
{
  int idx = 0;
  int next = 0;
  int prev = 0;
  get_calendar_day_index(idx, next, prev);
  if (prev >= 0)
    {
      set_calendar_day_index(prev);
    }
}

void
StatisticsDialog::on_history_go_forward()
{
  int idx = 0;
  int next = 0;
  int prev = 0;
  get_calendar_day_index(idx, next, prev);
  if (next >= 0)
    {
      set_calendar_day_index(next);
    }
}

void
StatisticsDialog::on_history_goto_last()
{
  set_calendar_day_index(0);
}

void
StatisticsDialog::on_history_goto_first()
{
  int size = statistics->get_history_size();
  set_calendar_day_index(size);
}

void
StatisticsDialog::on_history_delete_all()
{
  // /* Modal dialogs interrupt GUI input. That can be a problem if for example a break is
  // triggered while the message boxes are shown. The user would have no way to interact
  // with the break window without closing out the dialog which may be hidden behind it.
  // Temporarily override operation mode to avoid catastrophe, and remove the
  // override before any return.
  // */
  // const char funcname[] = "StatisticsDialog::on_history_delete_all";
  // app->get_core()->set_operation_mode_override( OperationMode::Suspended, funcname );

  // // Confirm the user's intention
  // string msg = UiUtil::create_alert_text(
  //     tr("Warning"),
  //     tr("You have chosen to delete your statistics history. Continue?")
  //     );
  // QMessageDialog mb_ask( *this, msg, true, QMESSAGE_WARNING, QBUTTONS_YES_NO, false );
  // mb_ask.set_title( tr("Warning") );
  // mb_ask.get_widget_for_response( QRESPONSE_NO )->grab_default();
  // if( mb_ask.run() == QRESPONSE_YES )
  // {
  //     mb_ask.hide();

  //     // Try to delete statistics history files
  //     for( ;; )
  //     {
  //         if( statistics->delete_all_history() )
  //         {
  //             msg = UiUtil::create_alert_text(
  //                 tr("Files deleted!"),
  //                 tr("The files containing your statistics history have been deleted.")
  //                 );
  //             QMessageDialog mb_info( *this, msg, true, QMESSAGE_INFO, QBUTTONS_OK, false );
  //             mb_info.set_title( tr("Info") );
  //             mb_info.run();
  //             break;
  //         }

  //         msg = UiUtil::create_alert_text(
  //             tr("File deletion failed!"),
  //             tr("The files containing your statistics history could not be deleted. Try again?")
  //             );
  //         QMessageDialog mb_error( *this, msg, true, QMESSAGE_ERROR, QBUTTONS_YES_NO, false );
  //         mb_error.set_title( tr("Error") );
  //         mb_error.get_widget_for_response( QRESPONSE_NO )->grab_default();
  //         if( mb_error.run() != QRESPONSE_YES )
  //             break;
  //     }
  // }

  // // Remove this function's operation mode override
  // app->get_core()->remove_operation_mode_override( funcname );
}

//! Periodic heartbeat.
auto
StatisticsDialog::on_timer() -> bool
{
  if (update_usage_real_time)
    {
      statistics->update();
      display_calendar_date();
    }
  return true;
}

void
StatisticsDialog::stream_distance(std::stringstream &stream, int64_t pixels)
{
  // char buf[64];

  // double mm = (double) pixels * gdk_screen_width_mm() / gdk_screen_width();
  // sprintf(buf, "%.02f m", mm/1000);
  // stream << buf;
}
