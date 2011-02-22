// TimerPreferencesPanel.cc --- Preferences widgets for a timer
//
// Copyright (C) 2002 - 2011 Raymond Penners <raymond@dotsphinx.com>
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>

#include "nls.h"
#include "debug.hh"

#include "CoreFactory.hh"
#include "ICore.hh"
#include "IConfigurator.hh"
#include "IBreak.hh"

#include "TimeEntry.hh"
#include "TimerPreferencesPanel.hh"
#include "GtkUtil.hh"
#include "Hig.hh"

#include "GUIConfig.hh"
#include "CoreConfig.hh"
#include "DataConnector.hh"

using namespace std;

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
  connector = new DataConnector();
  break_id = t;

  Gtk::HBox *box = Gtk::manage(new Gtk::HBox(false, 6));

  // Enabled/Disabled checkbox
  Gtk::Label *enabled_lab = Gtk::manage(GtkUtil::create_label(_("Enable timer"), true));
  enabled_cb = Gtk::manage(new Gtk::CheckButton());
  enabled_cb->add(*enabled_lab);
  enabled_cb->signal_toggled().connect(sigc::mem_fun(*this, &TimerPreferencesPanel::on_enabled_toggled));


  HigCategoriesPanel *categories = Gtk::manage(new HigCategoriesPanel());;

  Gtk::Widget *prelude_frame = Gtk::manage(create_prelude_panel());
  Gtk::Widget *timers_frame = Gtk::manage(create_timers_panel
                                     (hsize_group, vsize_group));
  Gtk::Widget *opts_frame = Gtk::manage(create_options_panel());

  categories->add(*timers_frame);
  categories->add(*opts_frame);

  enable_buttons();

  // Overall box
  box->pack_start(*categories, false, false, 0);
  box->pack_start(*prelude_frame, false, false, 0);

  pack_start(*enabled_cb, false, false, 0);
  pack_start(*box, false, false, 0);

  connector->connect(CoreConfig::CFG_KEY_BREAK_ENABLED % break_id, dc::wrap(enabled_cb));
  
  set_border_width(12);
}


TimerPreferencesPanel::~TimerPreferencesPanel()
{
  delete connector;
}


Gtk::Widget *
TimerPreferencesPanel::create_prelude_panel()
{
  // Prelude frame
  HigCategoryPanel *hig = Gtk::manage(new HigCategoryPanel(_("Break prompting")));

  prelude_cb = Gtk::manage(new Gtk::CheckButton(_("Prompt before breaking")));
  hig->add(*prelude_cb);

  Gtk::HBox *max_box = Gtk::manage(new Gtk::HBox());
  has_max_prelude_cb = Gtk::manage(new Gtk::CheckButton(_("Maximum number of prompts:")));
  max_prelude_spin = Gtk::manage(new Gtk::SpinButton(max_prelude_adjustment));
  max_box->pack_start(*has_max_prelude_cb, false, false, 0);
  max_box->pack_start(*max_prelude_spin, false, false, 0);
  hig->add(*max_box);

  connector->connect(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % break_id,
                     dc::wrap(prelude_cb),
                     sigc::mem_fun(*this, &TimerPreferencesPanel::on_preludes_changed));
  
  connector->connect(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % break_id,
                     dc::wrap(has_max_prelude_cb),
                     sigc::mem_fun(*this, &TimerPreferencesPanel::on_preludes_changed),
                     dc::NO_CONFIG);

  connector->connect(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % break_id,
                     dc::wrap(max_prelude_spin),
                     sigc::mem_fun(*this, &TimerPreferencesPanel::on_preludes_changed),
                     dc::NO_CONFIG);
  return hig;
}

Gtk::Widget *
TimerPreferencesPanel::create_options_panel()
{
  HigCategoryPanel *hig = Gtk::manage(new HigCategoryPanel(_("Options")));

  // Ignorable
  ignorable_cb = Gtk::manage(new Gtk::CheckButton
                        (_("Show 'Postpone' and 'Skip' button")));
  hig->add(*ignorable_cb);

  // Sensitive for activity
  activity_sensitive_cb = Gtk::manage(new Gtk::CheckButton
                        (_("Suspend timer when inactive")));
#if REMOVED_IN_FAVOR_OF_READING_MODE__REMOVE_LATER
  hig->add(*activity_sensitive_cb);
#endif
  
  // Break specific options
#ifdef HAVE_EXERCISES
  exercises_spin = NULL;
#endif

#ifdef HAVE_MICRO_BREAK_ACTIVITY
  monitor_cb = NULL;
  if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      monitor_cb
        = Gtk::manage(new Gtk::CheckButton(_("Regard micro-breaks as activity")));
      hig->add(*monitor_cb);
    }
#endif

#ifdef HAVE_EXERCISES
  if (break_id == BREAK_ID_REST_BREAK)
    {
      exercises_spin = Gtk::manage(new Gtk::SpinButton(exercises_adjustment));
      hig->add(_("Number of exercises:"), *exercises_spin);
    }
#endif
  if (break_id == BREAK_ID_REST_BREAK)
    {
      auto_natural_cb = Gtk::manage(new Gtk::CheckButton(_("Start natural restbreak when screen is locked")));
      hig->add(*auto_natural_cb);
      
      connector->connect(GUIConfig::CFG_KEY_BREAK_AUTO_NATURAL % break_id,
                         dc::wrap(auto_natural_cb));
    }

  connector->connect(CoreConfig::CFG_KEY_TIMER_ACTIVITY_SENSITIVE % break_id,
                     dc::wrap(activity_sensitive_cb));

  connector->connect(GUIConfig::CFG_KEY_BREAK_IGNORABLE % break_id,
                     dc::wrap(ignorable_cb));

  
#ifdef HAVE_EXERCISES
  if (break_id == BREAK_ID_REST_BREAK)
    {
      connector->connect(GUIConfig::CFG_KEY_BREAK_EXERCISES % break_id,
                         dc::wrap(exercises_spin));
    }
#endif

  connector->connect(CoreConfig::CFG_KEY_TIMER_MONITOR % break_id,
                     dc::wrap(monitor_cb),
                     sigc::mem_fun(*this, &TimerPreferencesPanel::on_monitor_changed));
  return hig;
}

Gtk::Widget *
TimerPreferencesPanel::create_timers_panel
(Glib::RefPtr<Gtk::SizeGroup> hsize_group,
 Glib::RefPtr<Gtk::SizeGroup> vsize_group)
{
  HigCategoryPanel *hig = Gtk::manage(new HigCategoryPanel(_("Timers")));

  // Limit time
  limit_tim = Gtk::manage(new TimeEntry());
  Gtk::Label *limit_lab = hig->add(break_id == BREAK_ID_DAILY_LIMIT
           ? _("Time before end:")
           : _("Time between breaks:"), *limit_tim);
  hsize_group->add_widget(*limit_lab);


  // Auto-reset time
  if (break_id != BREAK_ID_DAILY_LIMIT)
    {
      const char *auto_reset_txt = _("Break duration:");

      auto_reset_tim = Gtk::manage(new TimeEntry());

      Gtk::Label *auto_reset_lab = Gtk::manage(new Gtk::Label(auto_reset_txt));
      hsize_group->add_widget(*auto_reset_lab);
      hig->add(*auto_reset_lab, *auto_reset_tim);
    }
  else
    {
      auto_reset_tim = NULL;
    }

  // Snooze time
  snooze_tim = Gtk::manage(new TimeEntry());
  Gtk::Label *snooze_lab = hig->add(_("Postpone time:"), *snooze_tim);
  hsize_group->add_widget(*snooze_lab);

  vsize_group->add_widget(*hig);

  connector->connect(CoreConfig::CFG_KEY_TIMER_LIMIT % break_id, dc::wrap(limit_tim));
  connector->connect(CoreConfig::CFG_KEY_TIMER_AUTO_RESET % break_id, dc::wrap(auto_reset_tim));
  connector->connect(CoreConfig::CFG_KEY_TIMER_SNOOZE % break_id, dc::wrap(snooze_tim));

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

bool
TimerPreferencesPanel::on_preludes_changed(const std::string &key, bool write)
{
  static bool inside = false;

  if (inside)
    return true;

  inside = true;
  
  IConfigurator *config = CoreFactory::get_configurator();
  if (write)
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
      config->set_value(key, mp);
      set_prelude_sensitivity();
    }
  else
    {
      int value;
      bool ok = config->get_value(key, value);
      if (ok)
        {
          if (value == -1)
            {
              prelude_cb->set_active(true);
              has_max_prelude_cb->set_active(false);
            }
          else if (value == 0)
            {
              prelude_cb->set_active(false);
              has_max_prelude_cb->set_active(false);
            }
          else
            {
              prelude_cb->set_active(true);
              has_max_prelude_cb->set_active(true);
              max_prelude_adjustment.set_value(value);
            }

          set_prelude_sensitivity();
        }
    }

  inside = false;
  
  return true;
}


#ifdef HAVE_MICRO_BREAK_ACTIVITY
bool
TimerPreferencesPanel::on_monitor_changed(const string &key, bool write)
{
  IConfigurator *config = CoreFactory::get_configurator();

  if (write)
    {
      string val;

      if (monitor_cb->get_active())
        {
          ICore *core = CoreFactory::get_core();
          IBreak *mp_break = core->get_break(BREAK_ID_MICRO_BREAK);
          val = mp_break->get_name();
        }

      config->set_value(key, val);
    }
  else
    {
      string monitor_name;
      bool ok = config->get_value(key, monitor_name);
      if (ok)
        {
          bool s = monitor_cb->is_sensitive();
          monitor_cb->set_active(monitor_name != "");
          monitor_cb->set_sensitive(s);
        }
    }

  return true;
}
#endif


void
TimerPreferencesPanel::on_enabled_toggled()
{
  enable_buttons();
  set_prelude_sensitivity();
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
  if (break_id == BREAK_ID_REST_BREAK)
    {
        auto_reset_tim->set_sensitive(true);
    }
  else
    {
      if (auto_reset_tim != NULL)
        {
          auto_reset_tim->set_sensitive(on);
        }
    }
  snooze_tim->set_sensitive(on);
  // max_prelude_spin->set_sensitive(on);
}
