// TimerPreferencesPanel.cc --- Preferences widgets for a timer
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

// TODO: only when needed.
#define NOMINMAX

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"

#include "nls.h"
#include "debug.hh"

#include <unistd.h>
#include "Timer.hh"
#include "DayTimePred.hh"
#include "TimeEntry.hh"
#include "TimerPreferencesPanel.hh"
#include "ControlInterface.hh"
#include "Configurator.hh"
#include "GtkUtil.hh"
#include "Hig.hh"


TimerPreferencesPanel::TimerPreferencesPanel(GUIControl::BreakId t)
  : Gtk::HBox(false, 6),
    max_prelude_adjustment(0, 1, 100)
{
  TRACE_ENTER("TimerPreferencesPanel::TimerPreferencesPanel");
  break_id = t;
  timer = &GUIControl::get_instance()->timers[t];

  HigCategoriesPanel *categories = manage(new HigCategoriesPanel());;
  
  Gtk::Widget *prelude_frame = manage(create_prelude_panel());
  Gtk::Widget *timers_frame = manage(create_timers_panel());
  Gtk::Widget *opts_frame = manage(create_options_panel());

  categories->add(*timers_frame);
  categories->add(*opts_frame);
  
  // Overall box
  pack_start(*categories, false, false, 0);
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



Gtk::Widget *
TimerPreferencesPanel::create_prelude_panel()
{
  // Prelude frame
  HigCategoryPanel *hig = manage(new HigCategoryPanel(_("Break prompting")));
  
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

  hig->add(*prelude_cb);
  Gtk::HBox *max_box = manage(new Gtk::HBox());
  max_box->pack_start(*has_max_prelude_cb, false, false, 0);
  max_box->pack_start(*max_prelude_spin, false, false, 0);
  hig->add(*max_box);
  hig->add(*force_after_prelude_cb);

  return hig;
}

Gtk::Widget *
TimerPreferencesPanel::create_options_panel()
{
  TimerInterface *itimer = timer->timer;

  HigCategoryPanel *hig = manage(new HigCategoryPanel(_("Options")));
  
  // Insists option
  bool insists = timer->get_break_insisting();
  insists_cb = manage(new Gtk::CheckButton(_("Block user input")));
  insists_cb->set_active(insists);
  insists_cb->signal_toggled()
    .connect(SigC::slot(*this, &TimerPreferencesPanel::on_insists_toggled));
  hig->add(*insists_cb);
  
  // Monitor
  Configurator *cfg = GUIControl::get_instance()->get_configurator();
  string tpfx = ControlInterface::CFG_KEY_TIMER + itimer->get_id();
  if (break_id == GUIControl::BREAK_ID_DAILY_LIMIT)
    {
      monitor_cb
        = manage(new Gtk::CheckButton(_("Regard micro-pauses as activity")));
      string monitor_name;
      bool b = cfg->get_value
        (tpfx + ControlInterface::CFG_KEY_TIMER_MONITOR, &monitor_name);
      monitor_cb->set_active(b && monitor_name != "");
      monitor_cb->signal_toggled()
        .connect(SigC::slot(*this, &TimerPreferencesPanel::on_monitor_toggled));
      hig->add(*monitor_cb);
    }
  else
    {
      monitor_cb = NULL;
    }

  // Ignorable
  bool ignorable = timer->get_break_ignorable();
  ignorable_cb = manage(new Gtk::CheckButton
                        (_("Show 'Postpone' and 'Skip' button")));
  ignorable_cb->set_active(ignorable);
  ignorable_cb->signal_toggled()
    .connect(SigC::slot(*this, &TimerPreferencesPanel::on_ignorable_toggled));
  hig->add(*ignorable_cb);
  
  return hig;
}

Gtk::Widget *
TimerPreferencesPanel::create_timers_panel()
{
  HigCategoryPanel *hig = manage(new HigCategoryPanel(_("Timers")));

  // Snooze time
  TimerInterface *itimer = timer->timer;
  snooze_tim = manage(new TimeEntry());
  snooze_tim->set_value (itimer->get_snooze());
  snooze_tim->signal_value_changed()
    .connect(SigC::slot(*this, &TimerPreferencesPanel::on_snooze_changed));
  hig->add(_("Post-pone time:"), *snooze_tim);
  
  // Auto-reset time
  const char *auto_reset_txt;
  time_t auto_reset_value;
  if (break_id == GUIControl::BREAK_ID_DAILY_LIMIT)
    {
      auto_reset_txt = _("Resets at:");
      DayTimePred *pred = (DayTimePred *) itimer->get_auto_reset_predicate();
      assert(pred);
      auto_reset_value = pred->get_time_offset();
    }
  else
    {
      auto_reset_txt = _("Pause duration:");
      auto_reset_value = itimer->get_auto_reset();
    }
  
  auto_reset_tim = manage(new TimeEntry());
  auto_reset_tim->set_value (auto_reset_value);
  auto_reset_tim->signal_value_changed()
    .connect(SigC::slot(*this, &TimerPreferencesPanel::on_auto_reset_changed));
  hig->add(auto_reset_txt, *auto_reset_tim);

  // Limit time
  limit_tim = manage(new TimeEntry());
  limit_tim->set_value (itimer->get_limit());
  limit_tim->signal_value_changed()
    .connect(SigC::slot(*this, &TimerPreferencesPanel::on_limit_changed));
  hig->add(_("Time before break:"), *limit_tim);

  return hig;
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
  if (break_id == GUIControl::BREAK_ID_DAILY_LIMIT)
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
        ->timers[GUIControl::BREAK_ID_MICRO_PAUSE]
        .timer->get_id();
    }

  GUIControl::get_instance()->get_configurator()->set_value(key, val);
}

void
TimerPreferencesPanel::on_ignorable_toggled()
{
  timer->set_break_ignorable(ignorable_cb->get_active());
}
