// StatisticsDialog.cc --- Statistics dialog
//
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"

#include "debug.hh"

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

#include <unistd.h>
#include <assert.h>
#include <sstream>

#include "StatisticsDialog.hh"
#include "Text.hh"
#include "Util.hh"
#include "GUIControl.hh"
#include "GtkUtil.hh"

#include "nls.h"

StatisticsDialog::StatisticsDialog()
  : Gtk::Dialog(_("Statistics"), false, true),
    statistics(NULL),
    daily_usage_label(NULL),
    start_time_label(NULL),
    end_time_label(NULL),
    tips(NULL)
{
  TRACE_ENTER("StatisticsDialog::StatisticsDialog");

  statistics = Statistics::get_instance();
  
  init_gui();
  display_calendar_date();
  
  TRACE_EXIT();
}


//! Destructor.
StatisticsDialog::~StatisticsDialog()
{
  TRACE_ENTER("StatisticsDialog::~StatisticsDialog");
  TRACE_EXIT();
}


int
StatisticsDialog::run()
{
  show_all();
  return 0;
}


void
StatisticsDialog::init_gui()
{
  Gtk::Notebook *tnotebook = manage(new Gtk::Notebook());
  tnotebook->set_tab_pos(Gtk::POS_TOP);  

  // Init tooltips.
  tips = manage(new Gtk::Tooltips());
  tips->enable();
  
  Gtk::HBox *hbox = manage(new Gtk::HBox(false, 3));
  Gtk::VBox *vbox = manage(new Gtk::VBox(false, 3));

  // Calendar
  calendar = manage(new Gtk::Calendar());
  calendar->signal_month_changed().connect(SigC::slot(*this, &StatisticsDialog::on_calendar_month_changed));
  calendar->signal_day_selected().connect(SigC::slot(*this, &StatisticsDialog::on_calendar_day_selected));
  calendar->display_options(Gtk::CALENDAR_SHOW_WEEK_NUMBERS
                            |Gtk::CALENDAR_SHOW_DAY_NAMES
                            |Gtk::CALENDAR_SHOW_HEADING);

  // Button box.
  Gtk::HBox *btnbox= manage(new Gtk::HBox(false, 6));
  first_btn  
    = manage(GtkUtil::create_stock_button_without_text(Gtk::Stock::GOTO_FIRST));
  first_btn->signal_clicked()
    .connect(SigC::slot(*this, &StatisticsDialog::on_history_goto_first));
  last_btn
    = manage(GtkUtil::create_stock_button_without_text(Gtk::Stock::GOTO_LAST));
  last_btn->signal_clicked()
    .connect(SigC::slot(*this, &StatisticsDialog::on_history_goto_last));
  back_btn
    = manage(GtkUtil::create_stock_button_without_text(Gtk::Stock::GO_BACK));
  back_btn->signal_clicked()
    .connect(SigC::slot(*this, &StatisticsDialog::on_history_go_back));
  forward_btn
    = manage(GtkUtil::create_stock_button_without_text(Gtk::Stock::GO_FORWARD));
  forward_btn->signal_clicked()
    .connect(SigC::slot(*this, &StatisticsDialog::on_history_go_forward));
  
  Gtk::Label *nav_label = manage(new Gtk::Label(_("History:")));
  nav_label->set_alignment(1.0, 0);
  btnbox->pack_start(*nav_label, true, true, 0);
  btnbox->pack_start(*first_btn, false, false, 0);
  btnbox->pack_start(*back_btn, false, false, 0);
  btnbox->pack_start(*forward_btn, false, false, 0);
  btnbox->pack_start(*last_btn, false, false, 0);

  // Navigation box
  Gtk::VBox *navbox= manage(new Gtk::VBox(false, 6));
  navbox->pack_start(*calendar, false, false, 0);
  navbox->pack_start(*btnbox, false, false, 0);

  // Info box
  Gtk::Widget *infobox = manage(create_info_box());
  
  hbox->pack_start(*navbox, false, false, 0);
  hbox->pack_start(*vbox, true, true, 0);

  vbox->pack_start(*infobox, false, false, 0);
  vbox->pack_start(*tnotebook, true, true, 0);
  
  create_break_page(tnotebook);
  create_activity_page(tnotebook);

  
  get_vbox()->pack_start(*hbox, true, true, 0);

  tnotebook->show_all();
  tnotebook->set_current_page(0);

  // Dialog
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
  show_all();

}


Gtk::Widget *
StatisticsDialog::create_info_box()
{
  Gtk::Table *table = new Gtk::Table(2, 2, false);
  table->set_row_spacings(2);
  table->set_col_spacings(6);
  table->set_border_width(6);

  Gtk::Label *start_label = manage(new Gtk::Label());
  start_label->set_markup(_("<b>Start time:</b>"));
  Gtk::Label *end_label = manage(new Gtk::Label());
  end_label->set_markup(_("<b>End time:</b>"));
  start_time_label = manage(new Gtk::Label);
  end_time_label = manage(new Gtk::Label);
  
  start_label->set_alignment(1.0, 0);
  end_label->set_alignment(1.0, 0);
  start_time_label->set_alignment(0.0, 0);
  end_time_label->set_alignment(0.0, 0);

  table->attach(*start_label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
  table->attach(*start_time_label, 1, 2, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
  table->attach(*end_label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
  table->attach(*end_time_label, 1, 2, 1, 2, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
  return table;
}


Gtk::Widget *
StatisticsDialog::create_label(char *text, char *tooltip)
{
  Gtk::Label *label = manage(new Gtk::Label(text));
  Gtk::EventBox *eventbox = manage(new Gtk::EventBox());

  eventbox->add(*label);

  tips->set_tip(*eventbox, tooltip);
  return eventbox;
}


void
StatisticsDialog::create_break_page(Gtk::Notebook *tnotebook)
{
  Gtk::HBox *box = manage(new Gtk::HBox(false, 3));
  Gtk::Label *lab = manage(new Gtk::Label(_("Breaks")));
  box->pack_start(*lab, false, false, 0);

  Gtk::Table *table = manage(new Gtk::Table(10, 5, false));
  table->set_row_spacings(2);
  table->set_col_spacings(6);
  table->set_border_width(6);

  Gtk::Widget *unique_label = create_label(_("Total breaks"),
                                          _("Total number of unique breaks prompted. This equals "
                                            "the number of times the timer reached zero."));
  
  Gtk::Widget *prompted_label = create_label(_("Prompted"),
                                            _("Number of times workrave prompted you to take a break"));
  
  Gtk::Widget *taken_label = create_label(_("Breaks taken"),
                                         _("Number of times you took a break when prompted"));
  
  Gtk::Widget *natural_label = create_label(_("Natural breaks taken"),
                                         _("Number of times you took a break without being prompted"));
  
  Gtk::Widget *skipped_label = create_label(_("Breaks skipped"),
                                           _("Number of times you skipped a break"));
 
  Gtk::Widget *postponed_label = create_label(_("Breaks postponed"),
                                             _("Number of times you postponed a break"));
  
  Gtk::Widget *overdue_label = create_label(_("Total overdue time"),
                                           _("The total time this break was overdue"));

  Gtk::Widget *usage_label = create_label(_("Daily usage"),
                                        _("Total computer usage"));
  
  Gtk::HSeparator *hrule = manage(new Gtk::HSeparator());
  Gtk::VSeparator *vrule = manage(new Gtk::VSeparator());
  
  // Add labels to table.
  int y = 0;

  Gtk::Label *mp_label = manage(new Gtk::Label(_("Micro-pause")));
  Gtk::Label *rb_label = manage(new Gtk::Label(_("Restbreak")));
  Gtk::Label *dl_label = manage(new Gtk::Label(_("Daily limit")));
  

  y = 0;
  attach_left(*table, *mp_label, 2, y);
  attach_left(*table, *rb_label, 3, y);
  attach_left(*table, *dl_label, 4, y);

  y = 1;
  table->attach(*hrule, 0, 5, y, y + 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
  table->attach(*vrule, 1, 2, 0, 9, Gtk::SHRINK, Gtk::EXPAND | Gtk::FILL);

  y = 2;
  attach_right(*table, *unique_label, 0, y++);
  attach_right(*table, *prompted_label, 0, y++);
  attach_right(*table, *taken_label, 0, y++);
  attach_right(*table, *natural_label, 0, y++);
  attach_right(*table, *skipped_label, 0, y++);
  attach_right(*table, *postponed_label, 0, y++);
  attach_right(*table, *overdue_label, 0, y++);
  
  hrule = manage(new Gtk::HSeparator());
  vrule = manage(new Gtk::VSeparator());
  table->attach(*hrule, 0, 5, y, y + 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
  table->attach(*vrule, 1, 2, 10, 12, Gtk::SHRINK, Gtk::EXPAND | Gtk::FILL);
  y++;

  daily_usage_label = new Gtk::Label();
  
  attach_right(*table, *usage_label, 0, y);
  attach_left(*table, *daily_usage_label, 2, y++);
  
  // Put the breaks in table.
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      for (int j = 0; j < BREAK_STATS; j++)
        {
          break_labels[i][j] = new Gtk::Label();
          attach_left(*table, *break_labels[i][j], i + 2, j + 2);
        }
    }
  
  box->show_all();
  tnotebook->pages().push_back(Gtk::Notebook_Helpers::TabElem(*table, *box));
}


void
StatisticsDialog::create_activity_page(Gtk::Notebook *tnotebook)
{
  Gtk::HBox *box = manage(new Gtk::HBox(false, 3));
  Gtk::Label *lab = manage(new Gtk::Label(_("Activity")));
  box->pack_start(*lab, false, false, 0);

  Gtk::Table *table = manage(new Gtk::Table(8, 5, false));
  table->set_row_spacings(2);
  table->set_col_spacings(6);
  table->set_border_width(6);

  Gtk::Widget *mouse_time_label = create_label(_("Total mouse time:"),
                                              _("Total time you were using the mouse."));
  Gtk::Widget *mouse_movement_label = create_label(_("Total mouse movement:"),
                                              _("Total mouse movement in number of pixels."));
  Gtk::Widget *mouse_click_movement_label = create_label(_("Total click-to-click movement:"),
                                              _("Total mouse movement you would have had if you moved your "
                                                "mouse in straight lines between clicks."));
  Gtk::Widget *mouse_clicks_label = create_label(_("Total mouse button clicks:"),
                                              _("Total number of mouse button clicks."));
  Gtk::Widget *keystrokes_label = create_label(_("Total keystrokes:"),
                                              _("Total number of keys pressed."));
  

  int y = 0;
  attach_right(*table, *mouse_time_label, 0, y++);
  attach_right(*table, *mouse_movement_label, 0, y++);
  attach_right(*table, *mouse_click_movement_label, 0, y++);
  attach_right(*table, *mouse_clicks_label, 0, y++);
  attach_right(*table, *keystrokes_label, 0, y++);

  for (int i = 0; i < 5; i++)
    {
      activity_labels[i] = new Gtk::Label();
      attach_left(*table, *activity_labels[i], 1, i);
    }
  
  box->show_all();
  tnotebook->pages().push_back(Gtk::Notebook_Helpers::TabElem(*table, *box));
}


void
StatisticsDialog::attach_left(Gtk::Table &table, Widget &child, guint left_attach, guint top_attach)
{
  Gtk::Alignment *a = manage(new Gtk::Alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_BOTTOM, 0.0, 0.0));
  a->add(child);
  
  table.attach(*a, left_attach, left_attach+1, top_attach, top_attach + 1,
               Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
}


void
StatisticsDialog::attach_right(Gtk::Table &table, Widget &child, guint left_attach, guint top_attach)
{
  Gtk::Alignment *a = manage(new Gtk::Alignment(Gtk::ALIGN_RIGHT, Gtk::ALIGN_BOTTOM, 0.0, 0.0));
  a->add(child);
  
  table.attach(*a, left_attach, left_attach+1, top_attach, top_attach + 1,
               Gtk::FILL, Gtk::SHRINK);
}





void
StatisticsDialog::display_statistics(Statistics::DailyStats *stats)
{
  Statistics::DailyStats empty;
  bool is_empty;

  is_empty = stats == NULL;
  if (is_empty)
    {
      stats = &empty;
    }
  
  char s[200];
  if (stats->is_empty())
    {
      start_time_label->set_text("-");
      end_time_label->set_text("-");
    }
  else
    {
      size_t size = strftime(s, 200, "%c", &stats->start);
      if (size != 0)
        {
          start_time_label->set_text(s);
        }
      else
        {
          start_time_label->set_text("???");
        }
  
      size = strftime(s, 200, "%c", &stats->stop);
      if (size != 0)
        {
          end_time_label->set_text(s);
        }
      else
        {
          end_time_label->set_text("???");
        }
    }


  int value = stats->misc_stats[Statistics::STATS_VALUE_TOTAL_ACTIVE_TIME];
  daily_usage_label->set_text(Text::time_to_string(value));
  
  GUIControl *gui_control = GUIControl::get_instance();
  assert(gui_control != NULL);
  
  // Put the breaks in table.
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      stringstream ss;

      value = stats->break_stats[i][Statistics::STATS_BREAKVALUE_UNIQUE_BREAKS];
      ss.str("");
      ss << value;
      break_labels[i][0]->set_text(ss.str());
      
      value = stats->break_stats[i][Statistics::STATS_BREAKVALUE_PROMPTED];
      ss.str("");
      ss << value;
      break_labels[i][1]->set_text(ss.str());

      value = stats->break_stats[i][Statistics::STATS_BREAKVALUE_TAKEN];
      ss.str("");
      ss << value;
      break_labels[i][2]->set_text(ss.str());

      value = stats->break_stats[i][Statistics::STATS_BREAKVALUE_NATURAL_TAKEN];
      ss.str("");
      ss << value;
      break_labels[i][3]->set_text(ss.str());

      value = stats->break_stats[i][Statistics::STATS_BREAKVALUE_SKIPPED];
      ss.str("");
      ss << value;
      break_labels[i][4]->set_text(ss.str());

      value = stats->break_stats[i][Statistics::STATS_BREAKVALUE_POSTPONED];
      ss.str("");
      ss << value;
      break_labels[i][5]->set_text(ss.str());

      value = stats->break_stats[i][Statistics::STATS_BREAKVALUE_TOTAL_OVERDUE];

      break_labels[i][6]->set_text(Text::time_to_string(value));

      value = stats->misc_stats[Statistics::STATS_VALUE_TOTAL_MOVEMENT_TIME];
      activity_labels[0]->set_text(Text::time_to_string(value));

      value = stats->misc_stats[Statistics::STATS_VALUE_TOTAL_MOUSE_MOVEMENT];
      ss.str("");
      ss << value;
      activity_labels[1]->set_text(ss.str());
      
      value = stats->misc_stats[Statistics::STATS_VALUE_TOTAL_CLICK_MOVEMENT];
      ss.str("");
      ss << value;
      activity_labels[2]->set_text(ss.str());

      value = stats->misc_stats[Statistics::STATS_VALUE_TOTAL_CLICKS];
      ss.str("");
      ss << value;
      activity_labels[3]->set_text(ss.str());

      value = stats->misc_stats[Statistics::STATS_VALUE_TOTAL_KEYSTROKES];
      ss.str("");
      ss << value;
      activity_labels[4]->set_text(ss.str());
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
  calendar->freeze();
  Statistics::DailyStats *stats = statistics->get_day(idx);
  calendar->select_month(stats->start.tm_mon, stats->start.tm_year+1900);
  calendar->select_day(stats->start.tm_mday);
  calendar->thaw();
  display_calendar_date();
}

void
StatisticsDialog::display_calendar_date()
{
  int idx, next, prev;
  get_calendar_day_index(idx, next, prev);
  Statistics::DailyStats *stats = NULL;
  if (idx >= 0)
    {
      stats = statistics->get_day(idx);
    }
  forward_btn->set_sensitive(next >= 0);
  back_btn->set_sensitive(prev >= 0);
  last_btn->set_sensitive(idx != 0);
  first_btn->set_sensitive(idx != statistics->get_history_size());
  display_statistics(stats);
}


bool
StatisticsDialog::on_focus_in_event(GdkEventFocus *event)
{ 
  TRACE_ENTER("StatisticsDialog::focus_in");
  TRACE_EXIT();
}


bool
StatisticsDialog::on_focus_out_event(GdkEventFocus *event)
{ 
  TRACE_ENTER("StatisticsDialog::focus_out");
  TRACE_EXIT();
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


