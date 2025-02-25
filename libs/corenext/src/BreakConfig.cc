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
#  include "config.h"
#endif

#include "debug.hh"

#include <boost/algorithm/string/predicate.hpp>
#include <utility>

#include "BreakConfig.hh"
#include "core/CoreConfig.hh"
#include "DayTimePred.hh"

using namespace std;
using namespace workrave;
using namespace workrave::config;

BreakConfig::BreakConfig(BreakId break_id, BreakStateModel::Ptr break_state_model, Timer::Ptr timer)
  : break_id(break_id)
  , break_state_model(std::move(break_state_model))
  , timer(std::move(timer))
  , enabled(true)
  , use_microbreak_activity(false)
{
  load_timer_config();
  load_break_config();

  CoreConfig::key_timer(break_id).connect(this, [this] { load_timer_config(); });
  CoreConfig::key_break(break_id).connect(this, [this] { load_break_config(); });
}

void
BreakConfig::load_timer_config()
{
  TRACE_ENTRY();
  // Read break limit.
  int limit = CoreConfig::timer_limit(break_id)();
  timer->set_limit(limit);
  timer->set_limit_enabled(limit > 0);

  // Read autoreset interval
  int autoreset = CoreConfig::timer_auto_reset(break_id)();
  timer->set_auto_reset(autoreset);
  timer->set_auto_reset_enabled(autoreset > 0);

  // Read reset predicate
  string reset_pred = CoreConfig::timer_reset_pred(break_id)();
  if (!reset_pred.empty())
    {
      DayTimePred *pred = create_time_pred(reset_pred);
      timer->set_daily_reset(pred);
    }

  // Read the snooze time.
  int snooze = CoreConfig::timer_snooze(break_id)();
  timer->set_snooze(snooze);

  // Read the monitor setting for the timer.
  if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      use_microbreak_activity = (CoreConfig::timer_daily_limit_use_micro_break_activity()() != 0);
    }
}

void
BreakConfig::load_break_config()
{
  // Maximum number of prelude windows.
  int max_preludes = CoreConfig::break_max_preludes(break_id)();
  break_state_model->set_max_number_of_preludes(max_preludes);

  // Break enabled?
  enabled = CoreConfig::break_enabled(break_id)();

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

DayTimePred *
BreakConfig::create_time_pred(string spec)
{
  DayTimePred *pred = nullptr;
  bool ok = false;

  std::string type;
  std::string::size_type pos = spec.find('/');

  if (pos != std::string::npos)
    {
      type = spec.substr(0, pos);
      spec = spec.substr(pos + 1);

      if (type == "day")
        {
          auto *dayPred = new DayTimePred();
          ok = dayPred->init(spec);
          pred = dayPred;
        }
    }

  if ((pred != nullptr) && !ok)
    {
      delete pred;
      pred = nullptr;
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
