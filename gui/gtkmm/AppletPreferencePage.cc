// AppletPreferencePage.cc --- Preferences widgets for a timer
//
// Copyright (C) 2002 Rob Caelers & Raymond Penners
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <sstream>

#include <unistd.h>
#include "GUIControl.hh"
#include "AppletPreferencePage.hh"
#include "Configurator.hh"
#include "AppletWindow.hh"

AppletPreferencePage::AppletPreferencePage()
  : Gtk::HBox(false, 6)
{
  TRACE_ENTER("AppletPreferencePage::AppletPreferencePage");

  Gtk::Notebook *tnotebook = manage(new Gtk::Notebook());
  tnotebook->set_tab_pos(Gtk::POS_TOP);  

  create_page();


  TRACE_EXIT();
}


AppletPreferencePage::~AppletPreferencePage()
{
  TRACE_ENTER("AppletPreferencePage::~AppletPreferencePage");
  TRACE_EXIT();
}


void
AppletPreferencePage::create_page()
{
  // Frame
  Gtk::Frame *frame = manage(new Gtk::Frame(_("layout")));

  // Slot/Position of the break timer.
  Gtk::Label *mp_label = manage(new Gtk::Label(_("Micro-pause")));
  Gtk::Label *rb_label = manage(new Gtk::Label(_("Restbreak")));
  Gtk::Label *dl_label = manage(new Gtk::Label(_("Daily limit")));

  Gtk::Label *slot_label = manage(new Gtk::Label(_("Break position")));
  Gtk::Label *first_label = manage(new Gtk::Label(_("Show only when first")));
  Gtk::Label *imminent_label = manage(new Gtk::Label(_("Show only when imminent")));
  Gtk::Label *time_label = manage(new Gtk::Label(_("Break is imminent when due in")));
  
  Gtk::Table *table = manage(new Gtk::Table(5, 4, false));
  table->set_row_spacings(2);
  table->set_col_spacings(6);
  table->set_border_width(6);
  
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      slot_entry[i] = manage(new Gtk::SpinButton());
      slot_entry[i]->set_range(0, 3);
      slot_entry[i]->set_increments(1, 1);
      slot_entry[i]->set_numeric(true);
      slot_entry[i]->set_width_chars(2);

      time_entry[i] = manage(new Gtk::SpinButton());
      time_entry[i]->set_range(0, 3600);
      time_entry[i]->set_increments(1, 10);
      time_entry[i]->set_numeric(true);
      time_entry[i]->set_width_chars(4);
      
      first_cb[i] = manage(new  Gtk::CheckButton());
      imminent_cb[i] = manage(new  Gtk::CheckButton());

      table->attach(*slot_entry[i], i + 1, i + 2, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
      table->attach(*first_cb[i], i + 1, i + 2, 2, 3, Gtk::SHRINK, Gtk::SHRINK);
      table->attach(*imminent_cb[i], i + 1, i + 2, 3, 4, Gtk::SHRINK, Gtk::SHRINK);
      table->attach(*time_entry[i], i + 1, i + 2, 4, 5, Gtk::SHRINK, Gtk::SHRINK);
    }
  
  int y = 0;
  table->attach(*mp_label, 1, 2, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  table->attach(*rb_label, 2, 3, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  table->attach(*dl_label, 3, 4, y, y+1, Gtk::SHRINK, Gtk::SHRINK);

  table->attach(*slot_label, 0, 1, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
  table->attach(*first_label, 0, 1, 2, 3, Gtk::SHRINK, Gtk::SHRINK);
  table->attach(*imminent_label, 0, 1, 3, 4, Gtk::SHRINK, Gtk::SHRINK);
  table->attach(*time_label, 0, 1, 4, 5, Gtk::SHRINK, Gtk::SHRINK);
  
  frame->add(*table);
  pack_end(*frame, true, true, 0);
}

void
AppletPreferencePage::init_page_values()
{
  Configurator *c = GUIControl::get_instance()->get_configurator();

  bool enabled = false;
  if (!c->get_value(AppletWindow::CFG_KEY_APPLET_ENABLED, &enabled))
    {
      enabled = false;
    }

  int value = 10;
  if (!c->get_value(AppletWindow::CFG_KEY_APPLET_CYCLE_TIME, &value))
    {
      value = 10;
    }
  
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      GUIControl::TimerData &data = GUIControl::get_instance()->timers[i];
      
      int value = 0;
      if (!c->get_value(AppletWindow::CFG_KEY_APPLET + "/" + data.break_name + AppletWindow::CFG_KEY_APPLET_POSITION, &value))
        {
          value = i;
        }

      if (!c->get_value(AppletWindow::CFG_KEY_APPLET + "/" + data.break_name + AppletWindow::CFG_KEY_APPLET_FLAGS, &value))
        {
          value = AppletWindow::BREAK_EXCLUSIVE;
        }
      
      if (!c->get_value(AppletWindow::CFG_KEY_APPLET + "/" + data.break_name + AppletWindow::CFG_KEY_APPLET_IMMINENT, &value))
        {
          value = 30;
        }
    }
}
