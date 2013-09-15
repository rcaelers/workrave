// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#include "debug.hh"

#include "BreakConfig.hh"
#include "CoreConfig.hh"
#include "DayTimePred.hh"

using namespace std;
using namespace workrave::config;

BreakConfig::Ptr
BreakConfig::create(BreakId break_id, BreakStateModel::Ptr break_state_model, Timer::Ptr timer, IConfigurator::Ptr configurator)
{
  return Ptr(new BreakConfig(break_id, break_state_model, timer, configurator));
}


BreakConfig::BreakConfig(BreakId break_id, BreakStateModel::Ptr break_state_model, Timer::Ptr timer, IConfigurator::Ptr configurator) :
  break_id(break_id),
  break_state_model(break_state_model),
  timer(timer),
  configurator(configurator),
  enabled(true),
  use_microbreak_activity(false)
{
  load_timer_config();
  load_break_config();
  
  configurator->add_listener(CoreConfig::CFG_KEY_TIMER % break_id, this);
  configurator->add_listener(CoreConfig::CFG_KEY_BREAK % break_id, this);
}

//! Destructor.
BreakConfig::~BreakConfig()
{
}

void
BreakConfig::load_timer_config()
{
  TRACE_ENTER("BreakConfig::load_timer_config");

  // Read break limit.
  int limit;
  configurator->get_value(CoreConfig::CFG_KEY_TIMER_LIMIT % break_id, limit);
  timer->set_limit(limit);
  timer->set_limit_enabled(limit > 0);

  // Read autoreset interval
  int autoreset;
  configurator->get_value(CoreConfig::CFG_KEY_TIMER_AUTO_RESET % break_id, autoreset);
  timer->set_auto_reset(autoreset);
  timer->set_auto_reset_enabled(autoreset > 0);

  // Read reset predicate
  string reset_pred;
  configurator->get_value(CoreConfig::CFG_KEY_TIMER_RESET_PRED % break_id, reset_pred);
  if (reset_pred != "")
    {
      DayTimePred *pred = create_time_pred(reset_pred);
      timer->set_daily_reset(pred);
    }

  // Read the snooze time.
  int snooze;
  configurator->get_value(CoreConfig::CFG_KEY_TIMER_SNOOZE % break_id, snooze);
  timer->set_snooze(snooze);

  // Read the monitor setting for the timer.
  if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      use_microbreak_activity = false;
      configurator->get_value(CoreConfig::CFG_KEY_TIMER_DAILY_LIMIT_USE_MICRO_BREAK_ACTIVITY, use_microbreak_activity);
    }
  
  TRACE_EXIT();
}

void
BreakConfig::load_break_config()
{
  // Maximum number of prelude windows.
  int max_preludes;
  configurator->get_value(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % break_id, max_preludes);
  break_state_model->set_max_number_of_preludes(max_preludes);

  // Break enabled?
  configurator->get_value(CoreConfig::CFG_KEY_BREAK_ENABLED % break_id, enabled);

  if (enabled)
    {
      timer->enable();
      if (break_id == BREAK_ID_DAILY_LIMIT)
        {
          timer->set_limit_enabled(timer->get_limit() > 0);
        }
    }
  else
    {
      if (break_id != BREAK_ID_DAILY_LIMIT)
        {
          timer->disable();
        }
      else
        {
          timer->set_limit_enabled(false);
        }
    }
}

void
BreakConfig::config_changed_notify(const string &key)
{
  TRACE_ENTER_MSG("BreakConfig::config_changed_notify", key);
  string name;

  if (CoreConfig::starts_with(key, CoreConfig::CFG_KEY_BREAKS, name))
    {
      TRACE_MSG("break: " << name);
      load_break_config();
    }
  else if (CoreConfig::starts_with(key, CoreConfig::CFG_KEY_TIMERS, name))
    {
      TRACE_MSG("timer: " << name);
      load_timer_config();
    }
  TRACE_EXIT();
}

DayTimePred *
BreakConfig::create_time_pred(string spec)
{
  DayTimePred *pred = 0;
  bool ok = false;

  std::string type;
  std::string::size_type pos = spec.find('/');

  if (pos != std::string::npos)
    {
      type = spec.substr(0, pos);
      spec = spec.substr(pos + 1);

      if (type == "day")
        {
          DayTimePred *dayPred = new DayTimePred();
          ok = dayPred->init(spec);
          pred = dayPred;
        }
    }

  if (pred && !ok)
    {
      delete pred;
      pred = NULL;
    }

  return pred;
}

bool
BreakConfig::is_enabled() const
{
  return enabled;
}

bool
BreakConfig::is_microbreak_used_for_activity() const
{
  return use_microbreak_activity;
}
