// StatisticsDialog.cc --- Statistics dialog
//
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
#include "config.h"
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <assert.h>
#include <sstream>
#include <stdio.h>

#include <ctime>
#include <cstring>

#include "debug.hh"
#include "nls.h"

#include "core/ICore.hh"
#include "commonui/Backend.hh"

#include "StatisticsDialog.hh"
#include "commonui/Text.hh"
#include "utils/Locale.hh"

#include "UiUtil.hh"

using namespace std;

StatisticsDialog::StatisticsDialog()
  : QDialog(),
    daily_usage_time_label(NULL),
    weekly_usage_time_label(NULL),
    monthly_usage_time_label(NULL),
    date_label(NULL),
    update_usage_real_time(false)
{
  ICore::Ptr core = Backend::get_core();
  statistics = core->get_statistics();

  for (int i = 0; i < 5; i++)
    {
      activity_labels[i] = NULL;
    }

  init_gui();
  display_calendar_date();
}

StatisticsDialog::~StatisticsDialog()
{
}

int
StatisticsDialog::run()
{
  // Periodic timer.
  QTimer *timer = new QTimer(this);
  timer->setInterval(1000);
  connect(timer, SIGNAL(timeout()), this, SLOT(on_timer()));
  timer->start();  
  
  return 0;
}


void
StatisticsDialog::init_gui()
{
  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins(1, 1, 1, 1);
  setLayout(layout);

  QHBoxLayout *main_layout = new QHBoxLayout();
  main_layout->setContentsMargins(1, 1, 1, 1);
  layout->addLayout(main_layout);

  create_navigation_box(main_layout);
  create_statistics_box(main_layout);

  QDialogButtonBox *buttonBox =  new QDialogButtonBox(QDialogButtonBox::Close);
  layout->addWidget(buttonBox);
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(accept()));

  QTabWidget *notebook = new QTabWidget();
  notebook->setTabPosition(QTabWidget::West);
  notebook->setIconSize(QSize(100, 100));
  layout->addWidget(notebook);
}

void
StatisticsDialog::create_navigation_box(QLayout *parent)
{
  QGroupBox *box = new QGroupBox(_("Browse history"));
  QVBoxLayout *layout = new QVBoxLayout;
  box->setLayout(layout);
  parent->addWidget(box);
    
  QHBoxLayout *button_box = new QHBoxLayout();
  
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
  
  last_button  = new QPushButton;
  last_button->setIcon(QIcon::fromTheme("go-last", UiUtil::create_icon("go-last-symbolic.svg")));
  connect(last_button, &QPushButton::clicked, this, &StatisticsDialog::on_history_goto_last);
  button_box->addWidget(last_button, QDialogButtonBox::ActionRole);
  
  layout->addLayout(button_box);

  calendar = new QCalendarWidget();
	connect(calendar, &QCalendarWidget::clicked, this, &StatisticsDialog::on_calendar_day_selected);
	connect(calendar, &QCalendarWidget::currentPageChanged, this, &StatisticsDialog::on_calendar_month_changed);
  layout->addWidget(calendar);

  delete_button = new QPushButton(_("Delete all statistics history"));
  delete_button->setIcon(QIcon::fromTheme("edit-delete"));
  connect(delete_button, &QPushButton::clicked, this, &StatisticsDialog::on_history_delete_all);

  layout->addWidget(calendar);
}

void
StatisticsDialog::create_statistics_box(QLayout *parent)
{
  QGroupBox *box = new QGroupBox(_("Statistics"));
  QVBoxLayout *layout = new QVBoxLayout;
  box->setLayout(layout);
  parent->addWidget(box);

  date_label = UiUtil::add_label(layout, _("Date:"));
  
  create_break_page(layout);
}

void
StatisticsDialog::create_break_page(QBoxLayout *parent)
{
  QGridLayout *table = new QGridLayout();
  parent->addLayout(table);
  
  QWidget *unique_label
    = UiUtil::create_label_with_tooltip
    (_("Break prompts"),
     _("The number of times you were prompted to break, excluding"
       " repeated prompts for the same break"));

  QWidget *prompted_label
    = UiUtil::create_label_with_tooltip
    (_("Repeated prompts"),
     _("The number of times you were repeatedly prompted to break"));

  QWidget *taken_label
    = UiUtil::create_label_with_tooltip
    (_("Prompted breaks taken"),
     _("The number of times you took a break when being prompted"));

  QWidget *natural_label
    = UiUtil::create_label_with_tooltip
    (_("Natural breaks taken"),
     _("The number of times you took a break without being prompted"));

  QWidget *skipped_label
    = UiUtil::create_label_with_tooltip
    (_("Breaks skipped"),
     _("The number of breaks you skipped"));

  QWidget *postponed_label
    = UiUtil::create_label_with_tooltip
    (_("Breaks postponed"),
     _("The number of breaks you postponed"));

  QWidget *overdue_label
    = UiUtil::create_label_with_tooltip
    (_("Overdue time"),
     _("The total time this break was overdue"));

  QWidget *usage_label
    = UiUtil::create_label_with_tooltip
    (_("Usage"),
      ("Active computer usage"));

  QWidget *daily_usage_label
    = UiUtil::create_label_with_tooltip
    (_("Daily"),
     _("The total computer usage for the selected day"));

  QWidget *weekly_usage_label
    = UiUtil::create_label_with_tooltip
    (_("Weekly"),
     _("The total computer usage for the whole week of the selected day"));

  QWidget *monthly_usage_label
    = UiUtil::create_label_with_tooltip
    (_("Monthly"),
     _("The total computer usage for the whole month of the selected day"));

  QFrame *hrule = new QFrame();
  QFrame *vrule = new QFrame();

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
  y+=2;

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


// void
// StatisticsDialog::create_activity_page(QWidget *tnotebook)
// {
//   QHBoxLayout *box = new QHBoxLayout(false, 3);
//   QLabel *lab = new QLabel(_("Activity"));
//   box->pack_start(*lab, false, false, 0);

//   QTable *table = new QTable(8, 5, false);
//   table->set_row_spacings(2);
//   table->set_col_spacings(6);
//   table->set_border_width(6);

//   QWidget *mouse_time_label
//     = UiUtil::create_label_with_tooltip
//     (_("Mouse usage:"),
//      _("The total time you were using the mouse"));
//   QWidget *mouse_movement_label
//     = UiUtil::create_label_with_tooltip
//     (_("Mouse movement:"),
//      _("The total on-screen mouse movement"));
//   QWidget *mouse_click_movement_label
//     = UiUtil::create_label_with_tooltip
//     (_("Effective mouse movement:"),
//      _("The total mouse movement you would have had if you moved your "
//        "mouse in straight lines between clicks"));
//   QWidget *mouse_clicks_label
//     = UiUtil::create_label_with_tooltip
//     (_("Mouse button clicks:"),
//      _("The total number of mouse button clicks"));
//   QWidget *keystrokes_label
//     = UiUtil::create_label_with_tooltip
//     (_("Keystrokes:"),
//      _("The total number of keys pressed"));


//   int y = 0;
//   table->addWidget(mouse_time_label, 0, y++);
//   table->addWidget(mouse_movement_label, 0, y++);
//   table->addWidget(mouse_click_movement_label, 0, y++);
//   table->addWidget(mouse_clicks_label, 0, y++);
//   table->addWidget(keystrokes_label, 0, y++);

//   for (int i = 0; i < 5; i++)
//     {
//       activity_labels[i] = new QLabel();
//       table->addWidget(activity_labels[i], 1, i);
//     }

//   box->show_all();
// #ifdef HAVE_GTK3
//   ((QTabWidget *)tnotebook)->append_page(*tablebox);
// #else
//   ((QTabWidget *)tnotebook)->pages().push_back(QTabWidget_Helpers::TabElem(*tablebox));
// #endif
// }


void
StatisticsDialog::display_statistics(IStatistics::DailyStats *stats)
{
  IStatistics::DailyStats empty;
  bool is_empty;

  is_empty = stats == NULL;
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
      char date[100];
      char start[100];
      char stop[100];
      strftime(date, sizeof(date), "%x", &stats->start);
      strftime(start, sizeof(start), "%X", &stats->start);
      strftime(stop, sizeof(stop), "%X", &stats->stop);
      char buf[200];
      sprintf(buf, _("%s, from %s to %s"), date, start, stop);
      date_label->setText(buf);
    }


  int64_t value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_ACTIVE_TIME];
  daily_usage_time_label->setText(QString::fromStdString(Text::time_to_string(value)));

  // Put the breaks in table.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      stringstream ss;

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_UNIQUE_BREAKS];
      ss.str("");
      ss << value;
      break_labels[i][0]->setText(QString::fromStdString(ss.str()));

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_PROMPTED]
        - value;
      ss.str("");
      ss << value;
      break_labels[i][1]->setText(QString::fromStdString(ss.str()));

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_TAKEN];
      ss.str("");
      ss << value;
      break_labels[i][2]->setText(QString::fromStdString(ss.str()));

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_NATURAL_TAKEN];
      ss.str("");
      ss << value;
      break_labels[i][3]->setText(QString::fromStdString(ss.str()));

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_SKIPPED];
      ss.str("");
      ss << value;
      break_labels[i][4]->setText(QString::fromStdString(ss.str()));

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_POSTPONED];
      ss.str("");
      ss << value;
      break_labels[i][5]->setText(QString::fromStdString(ss.str()));

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_TOTAL_OVERDUE];

      break_labels[i][6]->setText(QString::fromStdString(Text::time_to_string(value)));
    }

  stringstream ss;

  if (activity_labels[0] != NULL)
    {
      // Label not available is OS X

      value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_MOVEMENT_TIME];
      if (value > 24 * 60 * 60) {
        value = 0;
      }
      activity_labels[0]->setText(QString::fromStdString(Text::time_to_string(value)));

      value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_MOUSE_MOVEMENT];
      ss.str("");
      stream_distance(ss, value);
      activity_labels[1]->setText(QString::fromStdString(ss.str()));

      value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_CLICK_MOVEMENT];
      ss.str("");
      stream_distance(ss, value);
      activity_labels[2]->setText(QString::fromStdString(ss.str()));

      value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_CLICKS];
      ss.str("");
      ss << value;
      activity_labels[3]->setText(QString::fromStdString(ss.str()));

      value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_KEYSTROKES];
      ss.str("");
      ss << value;
      activity_labels[4]->setText(QString::fromStdString(ss.str()));
    }

}

void
StatisticsDialog::display_week_statistics()
{
  QDate date = calendar->selectedDate();
  int y = date.year();
  int m = date.month() - 1;
  int d = date.day();

  std::tm timeinfo;
  std::memset(&timeinfo, 0, sizeof(timeinfo));
  timeinfo.tm_mday = d;
  timeinfo.tm_mon = m;
  timeinfo.tm_year = y - 1900;

  std::time_t t = std::mktime(&timeinfo);
  std::tm const *time_loc = std::localtime(&t);

  int offset = (time_loc->tm_wday - workrave::utils::Locale::get_week_start() + 7) % 7;
  int64_t total_week = 0;
  for (int i = 0; i < 7; i++)
    {
      std::memset(&timeinfo, 0, sizeof(timeinfo));
      timeinfo.tm_mday = d - offset + i;
      timeinfo.tm_mon = m;
      timeinfo.tm_year = y - 1900;
      t = std::mktime(&timeinfo);
      time_loc = std::localtime(&t);

      int idx, next, prev;
      statistics->get_day_index_by_date(time_loc->tm_year + 1900,
                                        time_loc->tm_mon + 1,
                                        time_loc->tm_mday, idx, next, prev);

      if (idx >= 0)
        {
          IStatistics::DailyStats *stats = statistics->get_day(idx);
          if (stats != NULL)
            {
              total_week += stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_ACTIVE_TIME];
            }

          update_usage_real_time |= (idx == 0);
        }
    }

  weekly_usage_time_label->setText(total_week > 0 ? QString::fromStdString(Text::time_to_string(total_week)) : "");
}

void
StatisticsDialog::display_month_statistics() {
  QDate date = calendar->selectedDate();
  int y = date.year();
  int m = date.month() - 1;

  int max_mday;
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
      int idx, next, prev;
      statistics->get_day_index_by_date(y, m + 1, i, idx, next, prev);
      if (idx >= 0)
        {
          IStatistics::DailyStats *stats = statistics->get_day(idx);
          if (stats != NULL)
            {
              total_month += stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_ACTIVE_TIME];
            }

          update_usage_real_time |= (idx == 0);
        }
    }

  monthly_usage_time_label->setText(total_month > 0 ? QString::fromStdString(Text::time_to_string(total_month)) : "");
}

void
StatisticsDialog::clear_display_statistics()
{
  date_label->setText("");
  daily_usage_time_label->setText("");
  weekly_usage_time_label->setText("");
  monthly_usage_time_label->setText("");

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      for (int j = 0; j <= 6; j++)
        {
          break_labels[i][j]->setText("");
        }
    }
  for (int i = 0; i <= 4; i++)
    {
      if (activity_labels[i] != NULL)
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
StatisticsDialog::on_calendar_day_selected(const QDate & date)
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
  date.setDate(stats->start.tm_year+1900, stats->start.tm_mon + 1, stats->start.tm_mday);
  calendar->setSelectedDate(date);
  display_calendar_date();
}

void
StatisticsDialog::display_calendar_date()
{
  int idx, next, prev;
  get_calendar_day_index(idx, next, prev);
  IStatistics::DailyStats *stats = NULL;
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
  int idx, next, prev;
  get_calendar_day_index(idx, next, prev);
  if (prev >= 0)
    set_calendar_day_index(prev);
}

void
StatisticsDialog::on_history_go_forward()
{
  int idx, next, prev;
  get_calendar_day_index(idx, next, prev);
  if (next >= 0)
    set_calendar_day_index(next);
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
    // Backend::get_core()->set_operation_mode_override( OperationMode::Suspended, funcname );

    // // Confirm the user's intention
    // string msg = UiUtil::create_alert_text(
    //     _("Warning"),
    //     _("You have chosen to delete your statistics history. Continue?")
    //     );
    // QMessageDialog mb_ask( *this, msg, true, QMESSAGE_WARNING, QBUTTONS_YES_NO, false );
    // mb_ask.set_title( _("Warning") );
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
    //                 _("Files deleted!"),
    //                 _("The files containing your statistics history have been deleted.")
    //                 );
    //             QMessageDialog mb_info( *this, msg, true, QMESSAGE_INFO, QBUTTONS_OK, false );
    //             mb_info.set_title( _("Info") );
    //             mb_info.run();
    //             break;
    //         }

    //         msg = UiUtil::create_alert_text(
    //             _("File deletion failed!"),
    //             _("The files containing your statistics history could not be deleted. Try again?")
    //             );
    //         QMessageDialog mb_error( *this, msg, true, QMESSAGE_ERROR, QBUTTONS_YES_NO, false );
    //         mb_error.set_title( _("Error") );
    //         mb_error.get_widget_for_response( QRESPONSE_NO )->grab_default();
    //         if( mb_error.run() != QRESPONSE_YES )
    //             break;
    //     }
    // }

    // // Remove this function's operation mode override
    // Backend::get_core()->remove_operation_mode_override( funcname );
}


//! Periodic heartbeat.
bool
StatisticsDialog::on_timer()
{
  if (update_usage_real_time)
    {
      statistics->update();
      display_calendar_date();
    }
  return true;
}

void
StatisticsDialog::stream_distance(stringstream &stream, int64_t pixels)
{
  // char buf[64];

  // double mm = (double) pixels * gdk_screen_width_mm() / gdk_screen_width();
  // sprintf(buf, "%.02f m", mm/1000);
  // stream << buf;
}
