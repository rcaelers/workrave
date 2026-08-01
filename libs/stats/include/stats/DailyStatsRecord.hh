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

#ifndef WORKRAVE_LIBS_STATS_DAILYSTATSRECORD_HH
#define WORKRAVE_LIBS_STATS_DAILYSTATSRECORD_HH

#include <chrono>
#include <cstdint>
#include <vector>

#include "stats/LocalTime.hh"

namespace workrave::stats
{
  //! The calendar day a set of statistics belongs to.
  using Date = std::chrono::year_month_day;

  //! The statistics of a single day.
  struct DailyStatsRecord
  {
    //! When this day started.
    LocalTime start;

    //! When the user was last active on this day.
    LocalTime stop;

    //! Per break statistics, indexed by break id and then by counter. TODO/STATS: std::map? std::array?
    std::vector<std::vector<int64_t>> break_stats;

    //! Miscellaneous statistics, indexed by counter.
    std::vector<int64_t> misc_stats;

    //! The calendar day these statistics belong to.
    [[nodiscard]] Date date() const
    {
      return date_of(start);
    }
  };
} // namespace workrave::stats

#endif // WORKRAVE_LIBS_STATS_DAILYSTATSRECORD_HH
