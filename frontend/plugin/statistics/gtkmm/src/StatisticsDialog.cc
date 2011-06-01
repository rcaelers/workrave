// StatisticsDialog.cc --- Statistics dialog
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011 Rob Caelers <robc@krandor.nl>
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

#include "preinclude.h"

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
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

#include <gtkmm/notebook.h>
#include <gtkmm/calendar.h>
#include <gtkmm/box.h>
#include <gtkmm/stock.h>
#include <gtkmm/label.h>
#include <gtkmm/separator.h>

#include "debug.hh"
#include "nls.h"

#include "ICore.hh"
#include "CoreFactory.hh"

#include "StatisticsDialog.hh"
#include "Text.hh"
#include "Util.hh"
#include "GtkUtil.hh"

StatisticsDialog::StatisticsDialog()
  : HigDialog(_("Statistics"), false, false),
    statistics(NULL),
    daily_usage_label(NULL),
    date_label(NULL)
{
  ICore *core = CoreFactory::get_core();
  statistics = core->get_statistics();

  for (int i = 0; i < 5; i++)
    {
      activity_labels[i] = NULL;
    }
  
  init_gui();
  display_calendar_date();
}


//! Destructor.
StatisticsDialog::~StatisticsDialog()
{
}


int
StatisticsDialog::run()
{
  // Periodic timer.
  Glib::signal_timeout().connect(sigc::mem_fun(*this, &StatisticsDialog::on_timer), 1000);

  show_all();
  return 0;
}


void
StatisticsDialog::init_gui()
{
#if !defined(PLATFORM_OS_OSX)
  Gtk::Notebook *tnotebook = Gtk::manage(new Gtk::Notebook());
  tnotebook->set_tab_pos(Gtk::POS_TOP);
#else
  Gtk::HBox *tnotebook= Gtk::manage(new Gtk::HBox(false, 6));
#endif
  
  // Calendar
  calendar = Gtk::manage(new Gtk::Calendar());
  calendar->signal_month_changed().connect(sigc::mem_fun(*this, &StatisticsDialog::on_calendar_month_changed));
  calendar->signal_day_selected().connect(sigc::mem_fun(*this, &StatisticsDialog::on_calendar_day_selected));
#ifdef HAVE_GTK3
  calendar->set_display_options(Gtk::CALENDAR_SHOW_WEEK_NUMBERS
                                |Gtk::CALENDAR_SHOW_DAY_NAMES
                                |Gtk::CALENDAR_SHOW_HEADING);
#else
  calendar->display_options(Gtk::CALENDAR_SHOW_WEEK_NUMBERS
                            |Gtk::CALENDAR_SHOW_DAY_NAMES
                            |Gtk::CALENDAR_SHOW_HEADING);
#endif
  
  // Button box.
  Gtk::HBox *btnbox= Gtk::manage(new Gtk::HBox(false, 6));
  first_btn
    = Gtk::manage(GtkUtil::create_custom_stock_button(NULL,Gtk::Stock::GOTO_FIRST));
  first_btn->signal_clicked()
    .connect(sigc::mem_fun(*this, &StatisticsDialog::on_history_goto_first));
  last_btn
    = Gtk::manage(GtkUtil::create_custom_stock_button(NULL,Gtk::Stock::GOTO_LAST));
  last_btn->signal_clicked()
    .connect(sigc::mem_fun(*this, &StatisticsDialog::on_history_goto_last));
  back_btn
    = Gtk::manage(GtkUtil::create_custom_stock_button(NULL,Gtk::Stock::GO_BACK));
  back_btn->signal_clicked()
    .connect(sigc::mem_fun(*this, &StatisticsDialog::on_history_go_back));
  forward_btn
    = Gtk::manage(GtkUtil::create_custom_stock_button(NULL,Gtk::Stock::GO_FORWARD));
  forward_btn->signal_clicked()
    .connect(sigc::mem_fun(*this, &StatisticsDialog::on_history_go_forward));

  btnbox->pack_start(*first_btn, true, true, 0);
  btnbox->pack_start(*back_btn, true, true, 0);
  btnbox->pack_start(*forward_btn, true, true, 0);
  btnbox->pack_start(*last_btn, true, true, 0);

  // Info box
  date_label = Gtk::manage(new Gtk::Label);

  // Navigation box
  HigCategoryPanel *browsebox
    = Gtk::manage(new HigCategoryPanel(_("Browse history")));
  browsebox->add(*btnbox);
  browsebox->add(*calendar);

  // Stats box
  HigCategoriesPanel *navbox = Gtk::manage(new HigCategoriesPanel());
  HigCategoryPanel *statbox = Gtk::manage(new HigCategoryPanel(_("Statistics")));
  statbox->add(_("Date:"), *date_label);
  statbox->add(*tnotebook);
  navbox->add(*statbox);

  Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox(false, 12));
  hbox->pack_start(*browsebox, false, false, 0);
  hbox->pack_start(*navbox, true, true, 0);

  create_break_page(tnotebook);
#if !defined(PLATFORM_OS_OSX)
  // No details activity statistics on OS X..
  create_activity_page(tnotebook);
#endif

  tnotebook->show_all();

#if !defined(PLATFORM_OS_OSX)
  tnotebook->set_current_page(0);
#endif
  
  get_vbox()->pack_start(*hbox, true, true, 0);

  // Dialog
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
  show_all();
}






void
StatisticsDialog::create_break_page(Gtk::Widget *tnotebook)
{
  Gtk::HBox *box = Gtk::manage(new Gtk::HBox(false, 3));
  Gtk::Label *lab = Gtk::manage(new Gtk::Label(_("Breaks")));
  box->pack_start(*lab, false, false, 0);

  Gtk::Table *table = Gtk::manage(new Gtk::Table(10, 5, false));
  table->set_row_spacings(2);
  table->set_col_spacings(6);
  table->set_border_width(6);

  Gtk::Widget *unique_label
    = GtkUtil::create_label_with_tooltip
    (_("Break prompts"),
     _("The number of times you were prompted to break, excluding"
       " repeated prompts for the same break"));

  Gtk::Widget *prompted_label
    = GtkUtil::create_label_with_tooltip
    (_("Repeated prompts"),
     _("The number of times you were repeatedly prompted to break"));

  Gtk::Widget *taken_label
    = GtkUtil::create_label_with_tooltip
    (_("Prompted breaks taken"),
     _("The number of times you took a break when being prompted"));

  Gtk::Widget *natural_label
    = GtkUtil::create_label_with_tooltip
    (_("Natural breaks taken"),
     _("The number of times you took a break without being prompted"));

  Gtk::Widget *skipped_label
    = GtkUtil::create_label_with_tooltip
    (_("Breaks skipped"),
     _("The number of breaks you skipped"));

  Gtk::Widget *postponed_label
    = GtkUtil::create_label_with_tooltip
    (_("Breaks postponed"),
     _("The number of breaks you postponed"));

  Gtk::Widget *overdue_label
    = GtkUtil::create_label_with_tooltip
    (_("Overdue time"),
     _("The total time this break was overdue"));

  Gtk::Widget *usage_label
    = GtkUtil::create_label_with_tooltip
    (_("Daily usage"),
     _("The total computer usage"));

  Gtk::HSeparator *hrule = Gtk::manage(new Gtk::HSeparator());
  Gtk::VSeparator *vrule = Gtk::manage(new Gtk::VSeparator());

  // Add labels to table.
  int y = 0;

  Gtk::Widget *mp_label = Gtk::manage(GtkUtil::create_label_for_break
                                 (BREAK_ID_MICRO_BREAK));
  Gtk::Widget *rb_label = Gtk::manage(GtkUtil::create_label_for_break
                                 (BREAK_ID_REST_BREAK));
  Gtk::Widget *dl_label = Gtk::manage(GtkUtil::create_label_for_break
                                 (BREAK_ID_DAILY_LIMIT));

  y = 0;
  GtkUtil::table_attach_left_aligned(*table, *mp_label, 2, y);
  GtkUtil::table_attach_left_aligned(*table, *rb_label, 3, y);
  GtkUtil::table_attach_left_aligned(*table, *dl_label, 4, y);

  y = 1;
  table->attach(*hrule, 0, 5, y, y + 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
  table->attach(*vrule, 1, 2, 0, 9, Gtk::SHRINK, Gtk::EXPAND | Gtk::FILL);

  y = 2;
  GtkUtil::table_attach_left_aligned(*table, *unique_label, 0, y++);
  GtkUtil::table_attach_left_aligned(*table, *prompted_label, 0, y++);
  GtkUtil::table_attach_left_aligned(*table, *taken_label, 0, y++);
  GtkUtil::table_attach_left_aligned(*table, *natural_label, 0, y++);
  GtkUtil::table_attach_left_aligned(*table, *skipped_label, 0, y++);
  GtkUtil::table_attach_left_aligned(*table, *postponed_label, 0, y++);
  GtkUtil::table_attach_left_aligned(*table, *overdue_label, 0, y++);

  hrule = Gtk::manage(new Gtk::HSeparator());
  vrule = Gtk::manage(new Gtk::VSeparator());
  table->attach(*hrule, 0, 5, y, y + 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
  table->attach(*vrule, 1, 2, 10, 12, Gtk::SHRINK, Gtk::EXPAND | Gtk::FILL);
  y++;

  daily_usage_label = Gtk::manage(new Gtk::Label());

  GtkUtil::table_attach_left_aligned(*table, *usage_label, 0, y);
  GtkUtil::table_attach_right_aligned(*table, *daily_usage_label, 2, y++);

  // Put the breaks in table.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      for (int j = 0; j < BREAK_STATS; j++)
        {
          break_labels[i][j] = Gtk::manage(new Gtk::Label());
          GtkUtil::table_attach_right_aligned(*table, *break_labels[i][j], i + 2, j + 2);
        }
    }

  box->show_all();
  
#if !defined(PLATFORM_OS_OSX)
  #ifdef HAVE_GTK3  
  ((Gtk::Notebook *)tnotebook)->append_page(*table, *box);
  #else
  ((Gtk::Notebook *)tnotebook)->pages().push_back(Gtk::Notebook_Helpers::TabElem(*table, *box));
  #endif  
#else
  ((Gtk::HBox *)tnotebook)->pack_start(*table, true, true, 0);
#endif
}


void
StatisticsDialog::create_activity_page(Gtk::Widget *tnotebook)
{
  Gtk::HBox *box = Gtk::manage(new Gtk::HBox(false, 3));
  Gtk::Label *lab = Gtk::manage(new Gtk::Label(_("Activity")));
  box->pack_start(*lab, false, false, 0);

  Gtk::Table *table = Gtk::manage(new Gtk::Table(8, 5, false));
  table->set_row_spacings(2);
  table->set_col_spacings(6);
  table->set_border_width(6);

  Gtk::Widget *mouse_time_label
    = GtkUtil::create_label_with_tooltip
    (_("Mouse usage:"),
     _("The total time you were using the mouse"));
  Gtk::Widget *mouse_movement_label
    = GtkUtil::create_label_with_tooltip
    (_("Mouse movement:"),
     _("The total on-screen mouse movement"));
  Gtk::Widget *mouse_click_movement_label
    = GtkUtil::create_label_with_tooltip
    (_("Effective mouse movement:"),
     _("The total mouse movement you would have had if you moved your "
       "mouse in straight lines between clicks"));
  Gtk::Widget *mouse_clicks_label
    = GtkUtil::create_label_with_tooltip
    (_("Mouse button clicks:"),
     _("The total number of mouse button clicks"));
  Gtk::Widget *keystrokes_label
    = GtkUtil::create_label_with_tooltip
    (_("Keystrokes:"),
     _("The total number of keys pressed"));


  int y = 0;
  GtkUtil::table_attach_left_aligned(*table, *mouse_time_label, 0, y++);
  GtkUtil::table_attach_left_aligned(*table, *mouse_movement_label, 0, y++);
  GtkUtil::table_attach_left_aligned(*table, *mouse_click_movement_label, 0, y++);
  GtkUtil::table_attach_left_aligned(*table, *mouse_clicks_label, 0, y++);
  GtkUtil::table_attach_left_aligned(*table, *keystrokes_label, 0, y++);

  for (int i = 0; i < 5; i++)
    {
      activity_labels[i] = Gtk::manage(new Gtk::Label());
      GtkUtil::table_attach_right_aligned(*table, *activity_labels[i], 1, i);
    }

  box->show_all();
#ifdef HAVE_GTK3  
  ((Gtk::Notebook *)tnotebook)->append_page(*table, *box);
#else
  ((Gtk::Notebook *)tnotebook)->pages().push_back(Gtk::Notebook_Helpers::TabElem(*table, *box));
#endif  

}






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
      date_label->set_text("-");
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
      date_label->set_text(buf);
    }


  int64_t value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_ACTIVE_TIME];
  daily_usage_label->set_text(Text::time_to_string(value));

  // Put the breaks in table.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      stringstream ss;

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_UNIQUE_BREAKS];
      ss.str("");
      ss << value;
      break_labels[i][0]->set_text(ss.str());

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_PROMPTED]
        - value;
      ss.str("");
      ss << value;
      break_labels[i][1]->set_text(ss.str());

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_TAKEN];
      ss.str("");
      ss << value;
      break_labels[i][2]->set_text(ss.str());

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_NATURAL_TAKEN];
      ss.str("");
      ss << value;
      break_labels[i][3]->set_text(ss.str());

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_SKIPPED];
      ss.str("");
      ss << value;
      break_labels[i][4]->set_text(ss.str());

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_POSTPONED];
      ss.str("");
      ss << value;
      break_labels[i][5]->set_text(ss.str());

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_TOTAL_OVERDUE];

      break_labels[i][6]->set_text(Text::time_to_string(value));
    }

  stringstream ss;

  if (activity_labels[0] != NULL)
    {
      // Label not available is OS X
      
      value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_MOVEMENT_TIME];
      if (value > 24 * 60 * 60) {
        value = 0;
      }
      activity_labels[0]->set_text(Text::time_to_string(value));

      value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_MOUSE_MOVEMENT];
      ss.str("");
      stream_distance(ss, value);
      activity_labels[1]->set_text(ss.str());

      value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_CLICK_MOVEMENT];
      ss.str("");
      stream_distance(ss, value);
      activity_labels[2]->set_text(ss.str());

      value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_CLICKS];
      ss.str("");
      ss << value;
      activity_labels[3]->set_text(ss.str());

      value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_KEYSTROKES];
      ss.str("");
      ss << value;
      activity_labels[4]->set_text(ss.str());
    }
}


void
StatisticsDialog::clear_display_statistics()
{
  date_label->set_text("");
  daily_usage_label->set_text("");

  // Put the breaks in table.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      for (int j = 0; j <= 6; j++)
        {
          break_labels[i][j]->set_text("");
        }
    }
  for (int i = 0; i <= 4; i++)
    {
      if (activity_labels[i] != NULL)
        {
          activity_labels[i]->set_text("");
        }
    }
}

void
StatisticsDialog::on_calendar_month_changed()
{
  display_calendar_date();
}

void
StatisticsDialog::on_calendar_day_selected()
{
  display_calendar_date();
}

void
StatisticsDialog::get_calendar_day_index(int &idx, int &next, int &prev)
{
  guint y, m, d;
  calendar->get_date(y, m, d);
  statistics->get_day_index_by_date(y, m+1, d, idx, next, prev);
}

void
StatisticsDialog::set_calendar_day_index(int idx)
{
  IStatistics::DailyStats *stats = statistics->get_day(idx);
  calendar->select_month(stats->start.tm_mon, stats->start.tm_year+1900);
  calendar->select_day(stats->start.tm_mday);
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
  forward_btn->set_sensitive(next >= 0);
  back_btn->set_sensitive(prev >= 0);
  last_btn->set_sensitive(idx != 0);
  first_btn->set_sensitive(idx != statistics->get_history_size());
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


//! Periodic heartbeat.
bool
StatisticsDialog::on_timer()
{
  int idx, next, prev;
  get_calendar_day_index(idx, next, prev);

  if (idx == 0)
    {
      statistics->update();
      display_calendar_date();
    }
  return true;
}


void
StatisticsDialog::stream_distance(stringstream &stream, int64_t pixels)
{
  char buf[64];

  double mm = (double) pixels * gdk_screen_width_mm() / gdk_screen_width();
  sprintf(buf, "%.02f m", mm/1000);
  stream << buf;
}
