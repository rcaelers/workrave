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

#include "BreakStatistics.hh"

using namespace std;
using namespace workrave;
using namespace workrave::stats;

BreakStatistics::BreakStatistics(BreakId break_id,
                                 BreakStateModel::Ptr break_state_model,
                                 Timer::Ptr timer,
                                 workrave::stats::IStatistics::Ptr statistics)
  : break_id(break_id)
  , break_state_model(break_state_model)
  , timer(timer)
  , statistics(statistics)
{
  connect(break_state_model->signal_break_event(), this, [this](auto &&event) {
    on_break_event(std::forward<decltype(event)>(event));
  });
}

void
BreakStatistics::on_break_event(BreakEvent event)
{
  switch (event)
    {
    case BreakEvent::ShowPrelude:
      statistics->break_counter(break_id, IStatistics::STATS_BREAKVALUE_PROMPTED).add(1);
      break;

    case BreakEvent::BreakStart:
      statistics->break_counter(break_id, IStatistics::STATS_BREAKVALUE_UNIQUE_BREAKS).add(1);
      break;

    case BreakEvent::BreakPostponed:
      statistics->break_counter(break_id, IStatistics::STATS_BREAKVALUE_POSTPONED).add(1);
      break;

    case BreakEvent::BreakSkipped:
      statistics->break_counter(break_id, IStatistics::STATS_BREAKVALUE_SKIPPED).add(1);
      break;

    case BreakEvent::BreakTaken:
      statistics->break_counter(break_id, IStatistics::STATS_BREAKVALUE_TAKEN).add(1);
      break;

    case BreakEvent::ShowBreak:
    case BreakEvent::ShowBreakForced:
    case BreakEvent::BreakIgnored:
    case BreakEvent::BreakIdle:
    case BreakEvent::BreakStop:
      break;
    }
}

void
BreakStatistics::daily_reset()
{
  update();
  statistics->start_new_day();
}

void
BreakStatistics::update()
{
  if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      statistics->total_active_time().set(std::chrono::seconds{timer->get_elapsed_time()});
    }

  statistics->total_overdue(break_id).set(std::chrono::seconds{timer->get_total_overdue_time()});
}
