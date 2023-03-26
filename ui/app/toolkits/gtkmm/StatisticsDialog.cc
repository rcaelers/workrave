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

#include <cstdio>
#include <sstream>

#include <ctime>
#include <cstring>

#include <fmt/chrono.h>
#include <gtkmm.h>

#include "debug.hh"
#include "commonui/nls.h"

#include "core/ICore.hh"

#include "StatisticsDialog.hh"
#include "commonui/Text.hh"
#include "GtkUtil.hh"
#include "commonui/Locale.hh"

using namespace std;
using namespace workrave;

StatisticsDialog::StatisticsDialog(std::shared_ptr<IApplicationContext> app)
  : HigDialog(_("Statistics"), false, false)
  , app(app)
{
  auto core = app->get_core();
  statistics = core->get_statistics();
  statistics->update();

  for (int i = 0; i < 5; i++)
    {
      activity_labels[i] = nullptr;
    }

  init_gui();
  display_calendar_date();
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
#if !defined(PLATFORM_OS_MACOS)
  Gtk::Notebook *tnotebook = Gtk::manage(new Gtk::Notebook());
  tnotebook->set_tab_pos(Gtk::POS_TOP);
#else
  Gtk::HBox *tnotebook = Gtk::manage(new Gtk::HBox(false, 6));
#endif

  // Calendar
  calendar = Gtk::manage(new Gtk::Calendar());
  calendar->signal_month_changed().connect(sigc::mem_fun(*this, &StatisticsDialog::on_calendar_month_changed));
  calendar->signal_day_selected().connect(sigc::mem_fun(*this, &StatisticsDialog::on_calendar_day_selected));
  calendar->set_display_options(Gtk::CALENDAR_SHOW_WEEK_NUMBERS | Gtk::CALENDAR_SHOW_DAY_NAMES | Gtk::CALENDAR_SHOW_HEADING);

  // Button box.
  Gtk::HBox *btnbox = Gtk::manage(new Gtk::HBox(false, 6));
  first_btn = Gtk::manage(GtkUtil::create_custom_stock_button(nullptr, "go-first"));
  first_btn->signal_clicked().connect(sigc::mem_fun(*this, &StatisticsDialog::on_history_goto_first));
  last_btn = Gtk::manage(GtkUtil::create_custom_stock_button(nullptr, "go-last"));
  last_btn->signal_clicked().connect(sigc::mem_fun(*this, &StatisticsDialog::on_history_goto_last));
  back_btn = Gtk::manage(GtkUtil::create_custom_stock_button(nullptr, "go-previous"));
  back_btn->signal_clicked().connect(sigc::mem_fun(*this, &StatisticsDialog::on_history_go_back));
  forward_btn = Gtk::manage(GtkUtil::create_custom_stock_button(nullptr, "go-next"));
  forward_btn->signal_clicked().connect(sigc::mem_fun(*this, &StatisticsDialog::on_history_go_forward));

  btnbox->pack_start(*first_btn, true, true, 0);
  btnbox->pack_start(*back_btn, true, true, 0);
  btnbox->pack_start(*forward_btn, true, true, 0);
  btnbox->pack_start(*last_btn, true, true, 0);

  // Info box
  date_label = Gtk::manage(new Gtk::Label);

  // Navigation box
  HigCategoryPanel *browsebox = Gtk::manage(new HigCategoryPanel(_("Browse history")));
  browsebox->add_widget(*btnbox);
  browsebox->add_widget(*calendar);

  // Delete button
  delete_btn = Gtk::manage(GtkUtil::create_custom_stock_button(_("Delete all statistics history"), "edit-delete"));
  delete_btn->signal_clicked().connect(sigc::mem_fun(*this, &StatisticsDialog::on_history_delete_all));
  browsebox->add_widget(*delete_btn);

  // Stats box
  HigCategoriesPanel *navbox = Gtk::manage(new HigCategoriesPanel());
  HigCategoryPanel *statbox = Gtk::manage(new HigCategoryPanel(_("Statistics")));
  statbox->add_label(_("Date:"), *date_label);
  statbox->add_widget(*tnotebook);
  navbox->add(*statbox);

  Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox(false, 12));
  hbox->pack_start(*browsebox, false, false, 0);
  hbox->pack_start(*navbox, true, true, 0);

  create_break_page(tnotebook);
#if !defined(PLATFORM_OS_MACOS)
  // No details activity statistics on macOS
  create_activity_page(tnotebook);
#endif

  tnotebook->show_all();

#if !defined(PLATFORM_OS_MACOS)
  tnotebook->set_current_page(0);
#endif

  get_vbox()->pack_start(*hbox, true, true, 0);

  // Dialog
  Gtk::Button *button = add_button(_("Close"), Gtk::RESPONSE_CLOSE);
  button->set_image_from_icon_name("window-close", Gtk::ICON_SIZE_BUTTON);

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

  Gtk::Widget *unique_label = GtkUtil::create_label_with_tooltip(_("Break prompts"),
                                                                 _("The number of times you were prompted to break, excluding"
                                                                   " repeated prompts for the same break"));

  Gtk::Widget *prompted_label = GtkUtil::create_label_with_tooltip(
    _("Repeated prompts"),
    _("The number of times you were repeatedly prompted to break"));

  Gtk::Widget *taken_label = GtkUtil::create_label_with_tooltip(_("Prompted breaks taken"),
                                                                _("The number of times you took a break when being prompted"));

  Gtk::Widget *natural_label = GtkUtil::create_label_with_tooltip(
    _("Natural breaks taken"),
    _("The number of times you took a break without being prompted"));

  Gtk::Widget *skipped_label = GtkUtil::create_label_with_tooltip(_("Breaks skipped"), _("The number of breaks you skipped"));

  Gtk::Widget *postponed_label = GtkUtil::create_label_with_tooltip(_("Breaks postponed"),
                                                                    _("The number of breaks you postponed"));

  Gtk::Widget *overdue_label = GtkUtil::create_label_with_tooltip(_("Overdue time"), _("The total time this break was overdue"));

  Gtk::Widget *usage_label = GtkUtil::create_label_with_tooltip(_("Usage"), _("Active computer usage"));

  Gtk::Widget *daily_usage_label = GtkUtil::create_label_with_tooltip(_("Daily"),
                                                                      _("The total computer usage for the selected day"));

  Gtk::Widget *weekly_usage_label = GtkUtil::create_label_with_tooltip(
    _("Weekly"),
    _("The total computer usage for the whole week of the selected day"));

  Gtk::Widget *monthly_usage_label = GtkUtil::create_label_with_tooltip(
    _("Monthly"),
    _("The total computer usage for the whole month of the selected day"));

  Gtk::HSeparator *hrule = Gtk::manage(new Gtk::HSeparator());
  Gtk::VSeparator *vrule = Gtk::manage(new Gtk::VSeparator());

  // Add labels to table.
  int y = 0;

  Gtk::Widget *mp_label = Gtk::manage(GtkUtil::create_label_for_break(BREAK_ID_MICRO_BREAK));
  Gtk::Widget *rb_label = Gtk::manage(GtkUtil::create_label_for_break(BREAK_ID_REST_BREAK));
  Gtk::Widget *dl_label = Gtk::manage(GtkUtil::create_label_for_break(BREAK_ID_DAILY_LIMIT));

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
  table->attach(*hrule, 0, 5, y, y + 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
  y++;

  daily_usage_time_label = Gtk::manage(new Gtk::Label());
  weekly_usage_time_label = Gtk::manage(new Gtk::Label());
  monthly_usage_time_label = Gtk::manage(new Gtk::Label());

  vrule = Gtk::manage(new Gtk::VSeparator());
  table->attach(*vrule, 1, 2, y, y + 3, Gtk::SHRINK, Gtk::EXPAND | Gtk::FILL);
  GtkUtil::table_attach_right_aligned(*table, *daily_usage_label, 2, y);
  GtkUtil::table_attach_right_aligned(*table, *weekly_usage_label, 3, y);
  GtkUtil::table_attach_right_aligned(*table, *monthly_usage_label, 4, y);
  y++;

  hrule = Gtk::manage(new Gtk::HSeparator());
  table->attach(*hrule, 0, 5, y, y + 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
  y++;

  GtkUtil::table_attach_left_aligned(*table, *usage_label, 0, y);
  GtkUtil::table_attach_right_aligned(*table, *daily_usage_time_label, 2, y);
  GtkUtil::table_attach_right_aligned(*table, *weekly_usage_time_label, 3, y);
  GtkUtil::table_attach_right_aligned(*table, *monthly_usage_time_label, 4, y);

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

#if !defined(PLATFORM_OS_MACOS)
  ((Gtk::Notebook *)tnotebook)->append_page(*table, *box);
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

  Gtk::Widget *mouse_time_label = GtkUtil::create_label_with_tooltip(_("Mouse usage:"),
                                                                     _("The total time you were using the mouse"));
  Gtk::Widget *mouse_movement_label = GtkUtil::create_label_with_tooltip(_("Mouse movement:"),
                                                                         _("The total on-screen mouse movement"));
  Gtk::Widget *mouse_click_movement_label = GtkUtil::create_label_with_tooltip(
    _("Effective mouse movement:"),
    _("The total mouse movement you would have had if you moved your "
      "mouse in straight lines between clicks"));
  Gtk::Widget *mouse_clicks_label = GtkUtil::create_label_with_tooltip(_("Mouse button clicks:"),
                                                                       _("The total number of mouse button clicks"));
  Gtk::Widget *keystrokes_label = GtkUtil::create_label_with_tooltip(_("Keystrokes:"), _("The total number of keys pressed"));

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
  ((Gtk::Notebook *)tnotebook)->append_page(*table, *box);
}

void
StatisticsDialog::display_statistics(IStatistics::DailyStats *stats)
{
  IStatistics::DailyStats empty{};

  bool is_empty = stats == nullptr;
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
      auto data_range = fmt::format(fmt::runtime(_("{:%x}, from {:%X} to {:%X}")), stats->start, stats->start, stats->stop);
      date_label->set_text(data_range);
    }

  int64_t value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_ACTIVE_TIME];
  daily_usage_time_label->set_text(Text::time_to_string(value));

  // Put the breaks in table.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      stringstream ss;

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_UNIQUE_BREAKS];
      ss.str("");
      ss << value;
      break_labels[i][0]->set_text(ss.str());

      value = stats->break_stats[i][IStatistics::STATS_BREAKVALUE_PROMPTED] - value;
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

  if (activity_labels[0] != nullptr)
    {
      // Label not available is OS X

      value = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_MOVEMENT_TIME];
      if (value > 24 * 60 * 60)
        {
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
StatisticsDialog::display_week_statistics()
{
  guint y = 0;
  guint m = 0;
  guint d = 0;
  calendar->get_date(y, m, d);

  std::tm timeinfo{};
  std::memset(&timeinfo, 0, sizeof(timeinfo));
  timeinfo.tm_mday = d;
  timeinfo.tm_mon = m;
  timeinfo.tm_year = y - 1900;

  std::time_t t = std::mktime(&timeinfo);
  std::tm const *time_loc = std::localtime(&t);

  int offset = (time_loc->tm_wday - Locale::get_week_start() + 7) % 7;
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

  weekly_usage_time_label->set_text(total_week > 0 ? Text::time_to_string(total_week) : "");
}

void
StatisticsDialog::display_month_statistics()
{
  guint y = 0;
  guint m = 0;
  guint d = 0;
  calendar->get_date(y, m, d);

  guint max_mday = 0;
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
  for (guint i = 1; i <= max_mday; i++)
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

  monthly_usage_time_label->set_text(total_month > 0 ? Text::time_to_string(total_month) : "");
}

void
StatisticsDialog::clear_display_statistics()
{
  date_label->set_text("");
  daily_usage_time_label->set_text("");
  weekly_usage_time_label->set_text("");
  monthly_usage_time_label->set_text("");

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
      if (activity_labels[i] != nullptr)
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
  guint y = 0;
  guint m = 0;
  guint d = 0;
  calendar->get_date(y, m, d);
  statistics->get_day_index_by_date(y, m + 1, d, idx, next, prev);
}

void
StatisticsDialog::set_calendar_day_index(int idx)
{
  IStatistics::DailyStats *stats = statistics->get_day(idx);
  calendar->select_month(stats->start.tm_mon, stats->start.tm_year + 1900);
  calendar->select_day(stats->start.tm_mday);
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
  forward_btn->set_sensitive(next >= 0);
  back_btn->set_sensitive(prev >= 0);
  last_btn->set_sensitive(idx != 0);
  first_btn->set_sensitive(idx != statistics->get_history_size());
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
  /* Modal dialogs interrupt GUI input. That can be a problem if for example a break is
  triggered while the message boxes are shown. The user would have no way to interact
  with the break window without closing out the dialog which may be hidden behind it.
  Temporarily override operation mode to avoid catastrophe, and remove the
  override before any return.
  */
  const char funcname[] = "StatisticsDialog::on_history_delete_all";
  app->get_core()->set_operation_mode_override(OperationMode::Suspended, funcname);

  // Confirm the user's intention
  string msg = HigUtil::create_alert_text(_("Warning"), _("You have chosen to delete your statistics history. Continue?"));
  Gtk::MessageDialog mb_ask(*this, msg, true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_YES_NO, false);
  mb_ask.set_title(_("Warning"));
  mb_ask.get_widget_for_response(Gtk::RESPONSE_NO)->grab_default();
  if (mb_ask.run() == Gtk::RESPONSE_YES)
    {
      mb_ask.hide();

      // Try to delete statistics history files
      for (;;)
        {
          if (statistics->delete_all_history())
            {
              msg = HigUtil::create_alert_text(_("Files deleted!"),
                                               _("The files containing your statistics history have been deleted."));
              Gtk::MessageDialog mb_info(*this, msg, true, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, false);
              mb_info.set_title(_("Info"));
              mb_info.run();
              break;
            }

          msg = HigUtil::create_alert_text(_("File deletion failed!"),
                                           _("The files containing your statistics history could not be deleted. Try again?"));
          Gtk::MessageDialog mb_error(*this, msg, true, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_YES_NO, false);
          mb_error.set_title(_("Error"));
          mb_error.get_widget_for_response(Gtk::RESPONSE_NO)->grab_default();
          if (mb_error.run() != Gtk::RESPONSE_YES)
            {
              break;
            }
        }
    }

  // Remove this function's operation mode override
  app->get_core()->remove_operation_mode_override(funcname);
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
  GdkDisplay *display = gdk_display_get_default();
  GdkMonitor *monitor = gdk_display_get_primary_monitor(display);

  GdkRectangle geometry;
  gdk_monitor_get_geometry(monitor, &geometry);

  double mm = (double)pixels * gdk_monitor_get_width_mm(monitor) / geometry.width;

  char buf[64];
  sprintf(buf, "%.02f m", mm / 1000);
  stream << buf;
}
