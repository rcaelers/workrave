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

#include "nls.h"

StatisticsDialog::StatisticsDialog()
  : Gtk::Dialog("Statistics", true, false)
{
  TRACE_ENTER("StatisticsDialog::StatisticsDialog");

  statistics = Statistics::get_instance();
  
  init_gui();
  init_page_values();
  select_day(0);
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
  int id = Gtk::Dialog::run();
  return id;
}


void
StatisticsDialog::init_gui()
{
  Gtk::Notebook *tnotebook = manage(new Gtk::Notebook());
  tnotebook->set_tab_pos(Gtk::POS_TOP);  


  Gtk::HBox *hbox = manage(new Gtk::HBox(false, 3));
  Gtk::VBox *vbox = manage(new Gtk::VBox(false, 3));

  // Scrollbar
  day_adjust = manage(new Gtk::Adjustment(1, 1, 100));
  Gtk::VScrollbar *sb = manage(new Gtk::VScrollbar(*day_adjust));

  day_adjust->signal_value_changed().connect(SigC::slot(*this, &StatisticsDialog::on_scrollbar));
  
  // Info box
  Gtk::HBox *infobox = manage(new Gtk::HBox(false, 3));
  create_info_box(infobox);
  
  hbox->pack_start(*sb, false, false, 0);
  hbox->pack_start(*vbox, true, true, 0);

  vbox->pack_start(*infobox, false, false, 0);
  vbox->pack_start(*tnotebook, true, true, 0);
  
  create_break_page(tnotebook);
  create_activity_page(tnotebook);

  // init_page_values();

  get_vbox()->pack_start(*hbox, true, true, 0);

  tnotebook->show_all();
  tnotebook->set_current_page(0);

  // Dialog
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
  show_all();

}


void
StatisticsDialog::create_info_box(Gtk::Box *box)
{
  Gtk::Table *table = manage(new Gtk::Table(1, 4, false));
  table->set_row_spacings(2);
  table->set_col_spacings(6);
  table->set_border_width(6);

  Gtk::Label *start_label = manage(new Gtk::Label(_("Start time:")));
  Gtk::Label *end_label = manage(new Gtk::Label(_("End time:")));

  start_time_label = manage(new Gtk::Label);
  end_time_label = manage(new Gtk::Label);
  
  attach_right(*table, *start_label, 0, 0);
  attach_right(*table, *start_time_label, 1, 0);
  attach_right(*table, *end_label, 2, 0);
  attach_right(*table, *end_time_label, 3, 0);

  box->pack_start(*table, true, true, 0);
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

  Gtk::Label *unique_label = manage(new Gtk::Label(_("Total breaks")));
  Gtk::Label *prompted_label = manage(new Gtk::Label(_("Prompted")));
  Gtk::Label *taken_label = manage(new Gtk::Label(_("Breaks Taken")));
  Gtk::Label *natural_label = manage(new Gtk::Label(_("Natural breaks")));
  Gtk::Label *skipped_label = manage(new Gtk::Label(_("Breaks skipped")));
  Gtk::Label *postponed_label = manage(new Gtk::Label(_("Breaks postponed")));
  Gtk::Label *usage_label = manage(new Gtk::Label(_("Daily usage")));

  
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
  table->attach(*vrule, 1, 2, 0, 8, Gtk::SHRINK, Gtk::EXPAND | Gtk::FILL);

  y = 2;
  attach_right(*table, *unique_label, 0, y++);
  attach_right(*table, *prompted_label, 0, y++);
  attach_right(*table, *taken_label, 0, y++);
  attach_right(*table, *natural_label, 0, y++);
  attach_right(*table, *skipped_label, 0, y++);
  attach_right(*table, *postponed_label, 0, y++);
  
  hrule = manage(new Gtk::HSeparator());
  vrule = manage(new Gtk::VSeparator());
  table->attach(*hrule, 0, 5, y, y + 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
  table->attach(*vrule, 1, 2, 9, 11, Gtk::SHRINK, Gtk::EXPAND | Gtk::FILL);
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
StatisticsDialog::init_page_values()
{
  TRACE_ENTER("StatisticsDialog::init_page_values");
  int size = statistics->get_history_size();
  TRACE_MSG(size);
  
  day_adjust->set_lower(0);
  day_adjust->set_upper(size - 1);
  day_adjust->set_value(size - 1);
  day_adjust->set_step_increment(1);
  day_adjust->set_page_increment(10);
  TRACE_EXIT();
}

void
StatisticsDialog::select_day(int day)
{
  Statistics::DailyStats *stats = statistics->get_day(day);

  char s[200];
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


  int value = stats->misc_stats[Statistics::STATS_VALUE_TOTAL_ACTIVE_TIME];
  daily_usage_label->set_text(Text::time_to_string(value));
  
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
    }
}

void
StatisticsDialog::on_scrollbar()
{
  int size = statistics->get_history_size();
  int day = (int)day_adjust->get_value();

  select_day(size - day - 1);
}
