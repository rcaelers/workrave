// StatisticsDialog.cc --- Statistics dialog
//
// Copyright (C) 2002, 2003 Raymond Penners <raymond@dotsphinx.com>
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

#include <unistd.h>
#include <assert.h>

#include "StatisticsDialog.hh"
#include "Util.hh"
#include "GUIControl.hh"

#include "nls.h"

StatisticsDialog::StatisticsDialog()
  : Gtk::Dialog("Statistics", true, false)
{
  TRACE_ENTER("StatisticsDialog::StatisticsDialog");
  init_gui();
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
  Gtk::Adjustment *adj = manage(new Gtk::Adjustment(1, 1, 100));
  Gtk::VScrollbar *sb = manage(new Gtk::VScrollbar(*adj));

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

  Gtk::Label *start_value_label = manage(new Gtk::Label(_("Sat Jan 25 2003, 4:00")));
  Gtk::Label *end_value_label = manage(new Gtk::Label(_("Sun Jan 26 2003, 4:00")));
  
  attach_right(*table, *start_label, 0, 0);
  attach_right(*table, *start_value_label, 1, 0);
  attach_right(*table, *end_label, 2, 0);
  attach_right(*table, *end_value_label, 3, 0);

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
  table->attach(*mp_label, 2, 3, y, y + 1, Gtk::SHRINK, Gtk::SHRINK);
  table->attach(*rb_label, 3, 4, y, y + 1, Gtk::SHRINK, Gtk::SHRINK);
  table->attach(*dl_label, 4, 5, y, y + 1, Gtk::SHRINK, Gtk::SHRINK);

  y = 1;
  table->attach(*hrule, 0, 5, y, y + 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
  table->attach(*vrule, 1, 2, 0, 8, Gtk::SHRINK, Gtk::EXPAND | Gtk::FILL);

  y = 2;
  attach_right(*table, *prompted_label, 0, y++);
  attach_right(*table, *unique_label, 0, y++);
  attach_right(*table, *taken_label, 0, y++);
  attach_right(*table, *natural_label, 0, y++);
  attach_right(*table, *skipped_label, 0, y++);
  attach_right(*table, *postponed_label, 0, y++);
  
  hrule = manage(new Gtk::HSeparator());
  vrule = manage(new Gtk::VSeparator());
  table->attach(*hrule, 0, 5, y, y + 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
  table->attach(*vrule, 1, 2, 9, 11, Gtk::SHRINK, Gtk::EXPAND | Gtk::FILL);
  y++;

  daily_usage_label = new Gtk::Label("y");
  
  attach_right(*table, *usage_label, 0, y);
  attach_left(*table, *daily_usage_label, 2, y++);
  
  // Put the breaks in table.
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      for (int j = 0; j < BREAK_STATS; j++)
        {
          break_labels[i][j] = new Gtk::Label("x");
          
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
StatisticsDialog::attach_left(Gtk::Table &table, Widget &child, guint left_attach, uint top_attach)
{
  Gtk::Alignment *a = manage(new Gtk::Alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_BOTTOM, 0.0, 0.0));
  a->add(child);
  
  table.attach(*a, left_attach, left_attach+1, top_attach, top_attach + 1,
               Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
}


void
StatisticsDialog::attach_right(Gtk::Table &table, Widget &child, guint left_attach, uint top_attach)
{
  Gtk::Alignment *a = manage(new Gtk::Alignment(Gtk::ALIGN_RIGHT, Gtk::ALIGN_BOTTOM, 0.0, 0.0));
  a->add(child);
  
  table.attach(*a, left_attach, left_attach+1, top_attach, top_attach + 1,
               Gtk::FILL, Gtk::SHRINK);
}
