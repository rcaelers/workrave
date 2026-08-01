// Copyright (C) 2026 Rob Caelers
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

#include "StatisticsContext.hh"

#include <cassert>

#include "stats/IStatistics.hh"

#include "Core.hh"
#include "Timer.hh"

using namespace workrave;

bool
StatisticsContext::is_active() const
{
  if (core == nullptr)
    {
      return false;
    }

  return core->get_activity_monitor()->get_current_state() == ACTIVITY_ACTIVE;
}

void
StatisticsContext::update_counters(workrave::stats::IStatistics *statistics)
{
  if (core == nullptr)
    {
      return;
    }

  // The total active time is what the daily limit timer has counted.
  Timer *daily_limit = core->get_break(BREAK_ID_DAILY_LIMIT)->get_timer();
  assert(daily_limit != nullptr);
  statistics->total_active_time().set(std::chrono::seconds{daily_limit->get_elapsed_time()});

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Timer *timer = core->get_break(BreakId(i))->get_timer();
      assert(timer != nullptr);

      statistics->total_overdue(BreakId(i)).set(std::chrono::seconds{timer->get_total_overdue_time()});
    }
}
