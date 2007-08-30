// TimerPreferencesPanel.cc --- Preferences widgets for a timer
//
// Copyright (C) 2002, 2003, 2004, 2006 Raymond Penners <raymond@dotsphinx.com>
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

#include <unistd.h>

#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>

#include "nls.h"
#include "debug.hh"

#include "CoreFactory.hh"
#include "ICore.hh"
#include "IBreak.hh"
#include "ITimer.hh"

#include "TimeEntry.hh"
#include "TimerPreferencesPanel.hh"
#include "GtkUtil.hh"
#include "Hig.hh"


TimerPreferencesPanel::TimerPreferencesPanel
(BreakId t,
 Glib::RefPtr<Gtk::SizeGroup> hsize_group,
 Glib::RefPtr<Gtk::SizeGroup> vsize_group)
  : Gtk::VBox(false, 6),
         max_prelude_adjustment(0, 1, 100)
#ifdef HAVE_EXERCISES
  ,exercises_adjustment(0, 0, 10)
#endif
{
  break_id = t;

  ICore *core = CoreFactory::get_core();
  break_data = core->get_break(BreakId(break_id));

  Gtk::HBox *box = manage(new Gtk::HBox(false, 6));

  // Enabled/Disabled checkbox
  Gtk::Label *enabled_lab = manage(GtkUtil::create_label(_("Enable timer"), true));
  enabled_cb = manage(new Gtk::CheckButton());
  enabled_cb->add(*enabled_lab);
  enabled_cb->set_active(break_data->get_break_enabled());
  enabled_cb->signal_toggled().connect(MEMBER_SLOT(*this, &TimerPreferencesPanel::on_enabled_toggled));

  HigCategoriesPanel *categories = manage(new HigCategoriesPanel());;

  Gtk::Widget *prelude_frame = manage(create_prelude_panel());
  Gtk::Widget *timers_frame = manage(create_timers_panel
                                     (hsize_group, vsize_group));
  Gtk::Widget *opts_frame = manage(create_options_panel());

  categories->add(*timers_frame);
  categories->add(*opts_frame);

  enable_buttons();
  set_prelude_sensitivity();

  // Overall box
  box->pack_start(*categories, false, false, 0);
  box->pack_start(*prelude_frame, false, false, 0);

  pack_start(*enabled_cb, false, false, 0);
  pack_start(*box, false, false, 0);

  set_border_width(12);
}


TimerPreferencesPanel::~TimerPreferencesPanel()
{
  // FIXME: disconnect signals?
}



Gtk::Widget *
TimerPreferencesPanel::create_prelude_panel()
{
  // Prelude frame
  HigCategoryPanel *hig = manage(new HigCategoryPanel(_("Break prompting")));

  prelude_cb = manage(new Gtk::CheckButton(_("Prompt before breaking")));
  int max_preludes = break_data->get_break_max_preludes();
  prelude_cb->set_active(max_preludes != 0);

  has_max_prelude_cb = manage(new Gtk::CheckButton
                              (_("Maximum number of prompts:")));
  has_max_prelude_cb->set_active(max_preludes > 0);

  max_prelude_adjustment.set_value(max_preludes > 0 ? max_preludes : 1);
  max_prelude_spin = manage(new Gtk::SpinButton(max_prelude_adjustment));

  set_prelude_sensitivity();

  prelude_cb->signal_toggled()
    .connect(MEMBER_SLOT(*this,
                        &TimerPreferencesPanel::on_preludes_active_toggled));
  has_max_prelude_cb->signal_toggled()
    .connect(MEMBER_SLOT(*this,
                        &TimerPreferencesPanel::on_preludes_maximum_toggled));
  max_prelude_adjustment.signal_value_changed()
    .connect(MEMBER_SLOT(*this,
                        &TimerPreferencesPanel::on_preludes_maximum_changed));

  hig->add(*prelude_cb);
  Gtk::HBox *max_box = manage(new Gtk::HBox());
  max_box->pack_start(*has_max_prelude_cb, false, false, 0);
  max_box->pack_start(*max_prelude_spin, false, false, 0);
  hig->add(*max_box);

  return hig;
}

Gtk::Widget *
TimerPreferencesPanel::create_options_panel()
{
  HigCategoryPanel *hig = manage(new HigCategoryPanel(_("Options")));

  // Ignorable
  bool ignorable = break_data->get_break_ignorable();
  ignorable_cb = manage(new Gtk::CheckButton
                        (_("Show 'Postpone' and 'Skip' button")));
  ignorable_cb->set_active(ignorable);
  ignorable_cb->signal_toggled()
    .connect(MEMBER_SLOT(*this, &TimerPreferencesPanel::on_ignorable_toggled));
  hig->add(*ignorable_cb);

  // Sensitive for activity
  bool sensitive = break_data->get_timer_activity_sensitive();
  activity_sensitive_cb = manage(new Gtk::CheckButton
                        (_("Suspend timer when inactive")));
  activity_sensitive_cb->set_active(sensitive);
  activity_sensitive_cb->signal_toggled()
    .connect(MEMBER_SLOT(*this, &TimerPreferencesPanel::on_activity_sensitive_toggled));
  hig->add(*activity_sensitive_cb);

  // Break specific options
#ifdef HAVE_EXERCISES
  exercises_spin = NULL;
#endif

#ifdef HAVE_MICRO_BREAK_ACTIVITY
  monitor_cb = NULL;
  if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      monitor_cb
        = manage(new Gtk::CheckButton(_("Regard micro-breaks as activity")));
      string monitor_name;
      monitor_name = break_data->get_timer_monitor();
      monitor_cb->set_active(monitor_name != "");
      monitor_cb->signal_toggled()
        .connect(MEMBER_SLOT(*this, &TimerPreferencesPanel::on_monitor_toggled));
      hig->add(*monitor_cb);
    }
#endif

#ifdef HAVE_EXERCISES
  if (break_id == BREAK_ID_REST_BREAK)
    {
      exercises_spin = manage(new Gtk::SpinButton(exercises_adjustment));
      hig->add(_("Number of exercises:"), *exercises_spin);
      exercises_adjustment.set_value(break_data->get_break_exercises());
      exercises_adjustment.signal_value_changed()
        .connect(MEMBER_SLOT(*this,
                            &TimerPreferencesPanel::on_exercises_changed));
    }
#endif


  return hig;
}

Gtk::Widget *
TimerPreferencesPanel::create_timers_panel
(Glib::RefPtr<Gtk::SizeGroup> hsize_group,
 Glib::RefPtr<Gtk::SizeGroup> vsize_group)
{
  HigCategoryPanel *hig = manage(new HigCategoryPanel(_("Timers")));

  // Limit time
  limit_tim = manage(new TimeEntry());
  limit_tim->set_value(break_data->get_timer_limit());
  limit_tim->signal_value_changed()
    .connect(MEMBER_SLOT(*this, &TimerPreferencesPanel::on_limit_changed));
  Gtk::Label *limit_lab = hig->add(break_id == BREAK_ID_DAILY_LIMIT
           ? _("Time before end:")
           : _("Time between breaks:"), *limit_tim);
  hsize_group->add_widget(*limit_lab);

  // Auto-reset time
  if (break_id != BREAK_ID_DAILY_LIMIT)
    {
      const char *auto_reset_txt;
      time_t auto_reset_value;

      auto_reset_txt = _("Break duration:");
      auto_reset_value = break_data->get_timer_auto_reset();

      auto_reset_tim = manage(new TimeEntry());
      auto_reset_tim->set_value (auto_reset_value);
      auto_reset_tim->signal_value_changed()
        .connect(MEMBER_SLOT(*this,
                            &TimerPreferencesPanel::on_auto_reset_changed));
      Gtk::Label *auto_reset_lab = manage(new Gtk::Label(auto_reset_txt));
      hsize_group->add_widget(*auto_reset_lab);
      hig->add(*auto_reset_lab, *auto_reset_tim);
    }
  else
    {
      auto_reset_tim = NULL;
    }

  // Snooze time
  snooze_tim = manage(new TimeEntry());
  snooze_tim->set_value (break_data->get_timer_snooze());
  snooze_tim->signal_value_changed()
    .connect(MEMBER_SLOT(*this, &TimerPreferencesPanel::on_snooze_changed));
  Gtk::Label *snooze_lab = hig->add(_("Postpone time:"), *snooze_tim);
  hsize_group->add_widget(*snooze_lab);

  vsize_group->add_widget(*hig);
  return hig;
}



void
TimerPreferencesPanel::set_prelude_sensitivity()
{
  bool on = enabled_cb->get_active();
  bool has_preludes = prelude_cb->get_active();
  bool has_max = has_max_prelude_cb->get_active();
  has_max_prelude_cb->set_sensitive(has_preludes && on);
  max_prelude_spin->set_sensitive(has_preludes && has_max && on);
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
  break_data->set_break_max_preludes(mp);
  set_prelude_sensitivity();
}

void
TimerPreferencesPanel::on_preludes_maximum_changed()
{
  int mp = (int) max_prelude_adjustment.get_value();
  break_data->set_break_max_preludes(mp);
}

#ifdef HAVE_EXERCISES
void
TimerPreferencesPanel::on_exercises_changed()
{
  int ex = (int) exercises_adjustment.get_value();
  break_data->set_break_exercises(ex);
}
#endif

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
  break_data->set_break_max_preludes(mp);
  set_prelude_sensitivity();
}




void
TimerPreferencesPanel::on_auto_reset_changed()
{
  TRACE_ENTER("TimerPreferencesPanel::on_auto_reset_changed");
  if (auto_reset_tim != NULL)
    {
      time_t val = auto_reset_tim->get_value();
      break_data->set_timer_auto_reset(val);
    }
  TRACE_EXIT();
}

void
TimerPreferencesPanel::on_snooze_changed()
{
  TRACE_ENTER("TimerPreferencesPanel::on_snooze_changed");
  break_data->set_timer_snooze(snooze_tim->get_value());
  TRACE_EXIT();
}

void
TimerPreferencesPanel::on_limit_changed()
{
  TRACE_ENTER("TimerPreferencesPanel::on_limit_changed");
  break_data->set_timer_limit(limit_tim->get_value());
  TRACE_EXIT();
}

#ifdef HAVE_MICRO_BREAK_ACTIVITY
void
TimerPreferencesPanel::on_monitor_toggled()
{
  string val;

  if (monitor_cb->get_active())
    {
      ICore *core = CoreFactory::get_core();
      IBreak *mp_break = core->get_break(BREAK_ID_MICRO_BREAK);
      val = mp_break->get_name();
    }

  break_data->set_timer_monitor(val);
}
#endif

void
TimerPreferencesPanel::on_ignorable_toggled()
{
  break_data->set_break_ignorable(ignorable_cb->get_active());
}

void
TimerPreferencesPanel::on_activity_sensitive_toggled()
{
  break_data->set_timer_activity_sensitive(activity_sensitive_cb->get_active());
}

void
TimerPreferencesPanel::on_enabled_toggled()
{
  break_data->set_break_enabled(enabled_cb->get_active());
  enable_buttons();
}

//! Enable widgets
void
TimerPreferencesPanel::enable_buttons()
{
  bool on = enabled_cb->get_active();

  ignorable_cb->set_sensitive(on);
  activity_sensitive_cb->set_sensitive(on);

#ifdef HAVE_MICRO_BREAK_ACTIVITY
  if (monitor_cb != NULL)
    {
      monitor_cb->set_sensitive(on);
    }
#endif

  prelude_cb->set_sensitive(on);
  has_max_prelude_cb->set_sensitive(on);
  limit_tim->set_sensitive(on);
  if (auto_reset_tim != NULL)
    auto_reset_tim->set_sensitive(on);
  snooze_tim->set_sensitive(on);
  max_prelude_spin->set_sensitive(on);
}
