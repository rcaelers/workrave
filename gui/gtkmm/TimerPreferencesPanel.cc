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

#include "nls.h"
#include "debug.hh"

#include <unistd.h>
#include "Timer.hh"
#include "DayTimePred.hh"
#include "TimeEntry.hh"
#include "TimerPreferencesPanel.hh"
#include "ControlInterface.hh"
#include "Configurator.hh"

TimerPreferencesPanel::TimerPreferencesPanel(GUIControl::TimerId t)
  : Gtk::HBox(false, 6),
    max_prelude_adjustment(0, 1, 100)
{
  TRACE_ENTER("TimerPreferencesPanel::TimerPreferencesPanel");
  timer_id = t;
  timer = &GUIControl::get_instance()->timers[t];

  // Frames
  Gtk::Frame *prelude_frame = manage(create_prelude_frame());
  Gtk::Frame *timers_frame = manage(create_timers_frame());
  Gtk::Frame *opts_frame = manage(create_options_frame());

  // VBox
  Gtk::VBox *vbox = manage(new Gtk::VBox(false, 6));
  vbox->pack_start(*timers_frame, false, false, 0);
  vbox->pack_start(*opts_frame, false, false, 0);
  

  // Overall box
  pack_start(*vbox, false, false, 0);
  pack_start(*prelude_frame, false, false, 0);

  set_border_width(6);

  TRACE_EXIT();
}


TimerPreferencesPanel::~TimerPreferencesPanel()
{
  TRACE_ENTER("TimerPreferencesPanel::~TimerPreferencesPanel");
  // FIXME: disconnect signals?
  TRACE_EXIT();
}


Gtk::Frame *
TimerPreferencesPanel::create_prelude_frame()
{
  // Prelude frame
  Gtk::Frame *prelude_frame = manage(new Gtk::Frame(_("Break prompting")));
  prelude_cb = manage(new Gtk::CheckButton(_("Prompt before breaking")));
  int max_preludes = timer->get_break_max_preludes();
  prelude_cb->set_active(max_preludes != 0);

  has_max_prelude_cb = manage(new Gtk::CheckButton
                              (_("Maximum number of prompts:")));
  has_max_prelude_cb->set_active(max_preludes > 0);
  
  max_prelude_adjustment.set_value(max_preludes > 0 ? max_preludes : 1);
  max_prelude_spin = manage(new Gtk::SpinButton(max_prelude_adjustment));
  
  force_after_prelude_cb = manage(new Gtk::CheckButton
                                  (_("Force break after maximum exceeded")));
  force_after_prelude_cb->set_active(timer->get_break_force_after_preludes());

  set_prelude_sensitivity();
  
  prelude_cb->signal_toggled()
    .connect(SigC::slot(*this,
                        &TimerPreferencesPanel::on_preludes_active_toggled));
  has_max_prelude_cb->signal_toggled()
    .connect(SigC::slot(*this,
                        &TimerPreferencesPanel::on_preludes_maximum_toggled));
  force_after_prelude_cb->signal_toggled()
    .connect(SigC::slot(*this,
                        &TimerPreferencesPanel::on_preludes_force_toggled));
  max_prelude_adjustment.signal_value_changed()
    .connect(SigC::slot(*this,
                        &TimerPreferencesPanel::on_preludes_maximum_changed));

  Gtk::VBox *prelude_vbox = manage(new Gtk::VBox(false, 0));
  prelude_vbox->set_border_width(6);
  prelude_vbox->pack_start(*prelude_cb, false, false, 0);
  prelude_vbox->pack_start(*has_max_prelude_cb, false, false, 0);
  prelude_vbox->pack_start(*max_prelude_spin, false, false, 0);
  prelude_vbox->pack_start(*force_after_prelude_cb, false, false, 0);
  prelude_frame->add(*prelude_vbox);

  return prelude_frame;
}

Gtk::Frame *
TimerPreferencesPanel::create_options_frame()
{
  TimerInterface *itimer = timer->timer;
  
  // Insists option
  bool insists = timer->get_break_insisting();
  insists_cb = manage(new Gtk::CheckButton(_("Block user input")));
  insists_cb->set_active(insists);
  insists_cb->signal_toggled()
    .connect(SigC::slot(*this, &TimerPreferencesPanel::on_insists_toggled));

  // Monitor
  Configurator *cfg = GUIControl::get_instance()->get_configurator();
  string tpfx = ControlInterface::CFG_KEY_TIMER + itimer->get_id();
  if (timer_id == GUIControl::TIMER_ID_DAILY_LIMIT)
    {
      monitor_cb
        = manage(new Gtk::CheckButton(_("Regard micro-pauses as activity")));
      string monitor_name;
      bool b = cfg->get_value
        (tpfx + ControlInterface::CFG_KEY_TIMER_MONITOR, &monitor_name);
      monitor_cb->set_active(b && monitor_name != "");
      monitor_cb->signal_toggled()
        .connect(SigC::slot(*this, &TimerPreferencesPanel::on_monitor_toggled));
    }
  else
    monitor_cb = NULL;

  // Ignorable
  bool ignorable = timer->get_break_ignorable();
  ignorable_cb = manage(new Gtk::CheckButton
                        (_("Show 'Postpone' and 'Skip' button")));
  ignorable_cb->set_active(ignorable);
  ignorable_cb->signal_toggled()
    .connect(SigC::slot(*this, &TimerPreferencesPanel::on_ignorable_toggled));
  
  // Options table
  Gtk::Table *opts_table = manage(new Gtk::Table(2,2, false));
  int y = 0;
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

  // Options frame
  Gtk::Frame *opts_frame = manage(new Gtk::Frame(_("Options")));
  opts_table->set_border_width(6);
  opts_frame->add(*opts_table);


  return opts_frame;
}

Gtk::Frame *
TimerPreferencesPanel::create_timers_frame()
{
  // Snooze time
  TimerInterface *itimer = timer->timer;
  Gtk::Label *snooze_lab = manage(new Gtk::Label(_("Post-pone time")));
  snooze_tim = manage(new TimeEntry());
  snooze_tim->set_value (itimer->get_snooze());
  snooze_tim->signal_value_changed()
    .connect(SigC::slot(*this, &TimerPreferencesPanel::on_snooze_changed));

  // Auto-reset time
  const char *auto_reset_txt;
  time_t auto_reset_value;
  if (timer_id == GUIControl::TIMER_ID_DAILY_LIMIT)
    {
      auto_reset_txt = _("Resets at");
      DayTimePred *pred = (DayTimePred *) itimer->get_auto_reset_predicate();
      assert(pred);
      auto_reset_value = pred->get_time_offset();
    }
  else
    {
      auto_reset_txt = _("Pause duration");
      auto_reset_value = itimer->get_auto_reset();
    }
  
  Gtk::Label *auto_reset_lab = manage(new Gtk::Label(auto_reset_txt));
  auto_reset_tim = manage(new TimeEntry());
  auto_reset_tim->set_value (auto_reset_value);
  auto_reset_tim->signal_value_changed()
    .connect(SigC::slot(*this, &TimerPreferencesPanel::on_auto_reset_changed));

  // Limit time
  Gtk::Label *limit_lab = manage(new Gtk::Label(_("Time before break")));
  limit_tim = manage(new TimeEntry());
  limit_tim->set_value (itimer->get_limit());
  limit_tim->signal_value_changed()
    .connect(SigC::slot(*this, &TimerPreferencesPanel::on_limit_changed));

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

  // Timers frame
  Gtk::Frame *timers_frame = manage(new Gtk::Frame(_("Timers")));
  timers_table->set_border_width(6);
  timers_frame->add(*timers_table);
  return timers_frame;
}



void
TimerPreferencesPanel::set_prelude_sensitivity()
{
  bool has_preludes = prelude_cb->get_active();
  bool has_max = has_max_prelude_cb->get_active();
  has_max_prelude_cb->set_sensitive(has_preludes);
  max_prelude_spin->set_sensitive(has_preludes && has_max);
  force_after_prelude_cb->set_sensitive(has_preludes && has_max);
}

void
TimerPreferencesPanel::on_preludes_active_toggled()
{
  int mp;
  if (prelude_cb->get_active())
    {
      if (has_max_prelude_cb->get_active())
        {
          mp = (int) max_prelude_adjustment.get_value();
        }
      else
        {
          mp = -1;
        }
    }
  else
    {
      mp = 0;
    }
  timer->set_break_max_preludes(mp);
  set_prelude_sensitivity();
}

void
TimerPreferencesPanel::on_preludes_maximum_changed()
{
  int mp = (int) max_prelude_adjustment.get_value();
  timer->set_break_max_preludes(mp);
}

void
TimerPreferencesPanel::on_preludes_maximum_toggled()
{
  int mp;
  if (has_max_prelude_cb->get_active())
    {
      mp = (int)max_prelude_adjustment.get_value();
    }
  else
    {
      mp = -1;
    }
  timer->set_break_max_preludes(mp);
  set_prelude_sensitivity();
}

void
TimerPreferencesPanel::on_preludes_force_toggled()
{
  timer->set_break_force_after_preludes(force_after_prelude_cb->get_active());
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
  timer->set_break_insisting(insists_cb->get_active());
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
  timer->set_break_ignorable(ignorable_cb->get_active());
}
