// TimerPreferencesPanel.cc --- Preferences widgets for a timer
//
// Copyright (C) 2002 Raymond Penners <raymond@dotsphinx.com>
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

// TODO: only when needed.
#define NOMINMAX

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include <unistd.h>
#include "Timer.hh"
#include "DayTimePred.hh"
#include "TimeEntry.hh"
#include "TimerPreferencesPanel.hh"
#include "ControlInterface.hh"
#include "Configurator.hh"

using std::cout;
using SigC::slot;


TimerPreferencesPanel::TimerPreferencesPanel(GUIControl::TimerId t)
  : Gtk::Frame()
{
  TRACE_ENTER("TimerPreferencesPanel::TimerPreferencesPanel");
  timer_id = t;
  timer = &GUIControl::get_instance()->timers[t];
  TimerInterface *itimer = timer->timer;
  
  // Frame label
  Gtk::HBox *box = manage(new Gtk::HBox(false, 3));
  Gtk::Label *lab = manage(new Gtk::Label(timer->name));
  Gtk::Image *img = manage(new Gtk::Image(timer->icon));
  box->pack_start(*img, false, false, 0);
  box->pack_start(*lab, false, false, 0);
  set_label_widget(*box);

  // Snooze time
  Gtk::Label *snooze_lab = manage(new Gtk::Label("Post-pone time"));
  snooze_tim = manage(new TimeEntry());
  snooze_tim->set_value (itimer->get_snooze());
  snooze_tim->signal_value_changed().connect(SigC::slot(*this, &TimerPreferencesPanel::on_snooze_changed));

  // Auto-reset time
  const char *auto_reset_txt;
  time_t auto_reset_value;
  if (timer_id == GUIControl::TIMER_ID_DAILY_LIMIT)
    {
      auto_reset_txt = "Resets at";
      DayTimePred *pred = (DayTimePred *) itimer->get_auto_reset_predicate();
      assert(pred);
      auto_reset_value = pred->get_time_offset();
    }
  else
    {
      auto_reset_txt = "Pause duration";
      auto_reset_value = itimer->get_auto_reset();
    }
  
  Gtk::Label *auto_reset_lab = manage(new Gtk::Label(auto_reset_txt));
  auto_reset_tim = manage(new TimeEntry());
  auto_reset_tim->set_value (auto_reset_value);
  auto_reset_tim->signal_value_changed().connect(SigC::slot(*this, &TimerPreferencesPanel::on_auto_reset_changed));

  // Limit time
  Gtk::Label *limit_lab = manage(new Gtk::Label("Time before break"));
  limit_tim = manage(new TimeEntry());
  limit_tim->set_value (itimer->get_limit());
  limit_tim->signal_value_changed().connect(SigC::slot(*this, &TimerPreferencesPanel::on_limit_changed));

  // Timers table
  Gtk::Table *timers_table = manage(new Gtk::Table(3, 2, false));
  timers_table->set_row_spacings(2);
  timers_table->set_col_spacings(6);
  int y = 0;
  timers_table->attach(*limit_lab, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  timers_table->attach(*limit_tim, 1, 2, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  y++;
  timers_table->attach(*auto_reset_lab, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  timers_table->attach(*auto_reset_tim, 1, 2, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  y++;
  timers_table->attach(*snooze_lab, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  timers_table->attach(*snooze_tim, 1, 2, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  
  // Separator
  Gtk::VSeparator *separator = manage(new Gtk::VSeparator());

  string pfx = GUIControl::CFG_KEY_BREAK + itimer->get_id();
  string tpfx = ControlInterface::CFG_KEY_TIMER + itimer->get_id();
  bool insists, ignorable;
  GUIControl::get_instance()->get_configurator()->get_value(pfx + GUIControl::CFG_KEY_BREAK_INSISTING, &insists);
  GUIControl::get_instance()->get_configurator()->get_value(pfx + GUIControl::CFG_KEY_BREAK_IGNORABLE, &ignorable);
  
  // Insists option
  insists_cb = manage(new Gtk::CheckButton("Block user input"));
  insists_cb->signal_toggled().connect(SigC::slot(*this, &TimerPreferencesPanel::on_insists_toggled));
  insists_cb->set_active(insists);

  // Monitor
  if (timer_id == GUIControl::TIMER_ID_DAILY_LIMIT)
    {
      monitor_cb = manage(new Gtk::CheckButton("Regard micro-pauses as activity"));
      monitor_cb->signal_toggled().connect(SigC::slot(*this, &TimerPreferencesPanel::on_monitor_toggled));
      string monitor_name;
      bool ret = GUIControl::get_instance()->get_configurator()
        ->get_value(tpfx + ControlInterface::CFG_KEY_TIMER_MONITOR, &monitor_name);
      monitor_cb->set_active(ret && monitor_name != "");
    }
  else
    monitor_cb = NULL;

  // Ignorable
  ignorable_cb = manage(new Gtk::CheckButton("Show 'Postpone' and 'Skip' button"));
  ignorable_cb->signal_toggled().connect(SigC::slot(*this, &TimerPreferencesPanel::on_ignorable_toggled));
  ignorable_cb->set_active(ignorable);
  
  // Options table
  Gtk::Table *opts_table = manage(new Gtk::Table(2,2, false));
  y = 0;
  opts_table->attach(*insists_cb, 1, 2, y, y+1,
                     Gtk::FILL, Gtk::SHRINK);
  y++;
  opts_table->attach(*ignorable_cb, 1, 2, y, y+1,
                     Gtk::FILL, Gtk::SHRINK);
  y++;
  if (monitor_cb)
    {
      opts_table->attach(*monitor_cb, 1, 2, y, y+1,
                         Gtk::FILL, Gtk::SHRINK);
      y++;
    }

  // Overall box
  Gtk::HBox *hbox = manage(new Gtk::HBox(false, 6));
  hbox->pack_start(*timers_table, Gtk::EXPAND | Gtk::SHRINK, 0);
  hbox->pack_start(*separator, Gtk::SHRINK, 0);
  hbox->pack_start(*opts_table, Gtk::EXPAND | Gtk::SHRINK, 0);

  hbox->set_border_width(6);
  add(*hbox);
  TRACE_EXIT();
}



TimerPreferencesPanel::~TimerPreferencesPanel()
{
  TRACE_ENTER("TimerPreferencesPanel::~TimerPreferencesPanel");
  // FIXME: disconnect signals?
  TRACE_EXIT();
}

void
TimerPreferencesPanel::config_changed_notify(string key)
{
}


void
TimerPreferencesPanel::on_auto_reset_changed()
{
  TRACE_ENTER("TimerPreferencesPanel::on_auto_reset_changed");
  string key;
  key = ControlInterface::CFG_KEY_TIMER + timer->timer->get_id();
  time_t val = auto_reset_tim->get_value();
  if (timer_id == GUIControl::TIMER_ID_DAILY_LIMIT)
    {
      key +=  ControlInterface::CFG_KEY_TIMER_RESET_PRED;
      DayTimePred pred;
      pred.init(val/(60*60), (val/60)%60);
      GUIControl::get_instance()->get_configurator()->set_value(key, pred.to_string());
    }
  else
    {
      key +=  ControlInterface::CFG_KEY_TIMER_AUTO_RESET;
      GUIControl::get_instance()->get_configurator()
        ->set_value(key, val);
    }
  TRACE_EXIT();
}

void
TimerPreferencesPanel::on_snooze_changed()
{
  TRACE_ENTER("TimerPreferencesPanel::on_snooze_changed");
  string key;
  key = ControlInterface::CFG_KEY_TIMER + timer->timer->get_id()
    + ControlInterface::CFG_KEY_TIMER_SNOOZE;
  GUIControl::get_instance()->get_configurator()
    ->set_value(key, snooze_tim->get_value());

  TRACE_EXIT();
}

void
TimerPreferencesPanel::on_limit_changed()
{
  TRACE_ENTER("TimerPreferencesPanel::on_limit_changed");
  string key;
  key = ControlInterface::CFG_KEY_TIMER + timer->timer->get_id()
    + ControlInterface::CFG_KEY_TIMER_LIMIT;
  GUIControl::get_instance()->get_configurator()
    ->set_value(key, limit_tim->get_value());

  TRACE_EXIT();
}

void
TimerPreferencesPanel::on_insists_toggled()
{
  TRACE_ENTER("TimerPreferencesPanel::on_insists_toggled");

  string key;
  key = GUIControl::CFG_KEY_BREAK + timer->timer->get_id()
    + GUIControl::CFG_KEY_BREAK_INSISTING;
  GUIControl::get_instance()->get_configurator()
    ->set_value(key, insists_cb->get_active());

  TRACE_EXIT();
}

void
TimerPreferencesPanel::on_monitor_toggled()
{
  string key = ControlInterface::CFG_KEY_TIMER + timer->timer->get_id()
    + ControlInterface::CFG_KEY_TIMER_MONITOR;
  string val;

  if (monitor_cb->get_active())
    {
      val = GUIControl::get_instance()
        ->timers[GUIControl::TIMER_ID_MICRO_PAUSE]
        .timer->get_id();
    }

  GUIControl::get_instance()->get_configurator()->set_value(key, val);
}

void
TimerPreferencesPanel::on_ignorable_toggled()
{
  TRACE_ENTER("TimerPreferencesPanel::on_ignorable_toggled");

  string key;
  key = GUIControl::CFG_KEY_BREAK + timer->timer->get_id()
    + GUIControl::CFG_KEY_BREAK_IGNORABLE;
  GUIControl::get_instance()->get_configurator()
    ->set_value(key, ignorable_cb->get_active());

  TRACE_EXIT();
}
