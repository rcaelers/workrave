// Copyright (C) 2002 - 2013 Rob Caelers & Raymond Penners
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

#ifndef WORKRAVE_BACKEND_ISTATISTICS_HH
#define WORKRAVE_BACKEND_ISTATISTICS_HH

#include <ctime>
#include <memory>
#include <optional>
#include <vector>

#if defined(PLATFORM_OS_WINDOWS_NATIVE)
typedef __int64 int64_t;
#else
#  include <cstdint>
#endif

#include "core/CoreTypes.hh"
#include "stats/DailyStatsRecord.hh"
#include "stats/LocalTime.hh"

namespace workrave
{
  class IStatistics
  {
  public:
    using Ptr = std::shared_ptr<IStatistics>;

    enum StatsBreakValueType
    {
      STATS_BREAKVALUE_PROMPTED = 0,
      STATS_BREAKVALUE_TAKEN,
      STATS_BREAKVALUE_NATURAL_TAKEN,
      STATS_BREAKVALUE_SKIPPED,
      STATS_BREAKVALUE_POSTPONED,
      STATS_BREAKVALUE_UNIQUE_BREAKS,
      STATS_BREAKVALUE_TOTAL_OVERDUE,
      STATS_BREAKVALUE_SIZEOF
    };

    enum StatsValueType
    {
      STATS_VALUE_TOTAL_ACTIVE_TIME = 0,
      STATS_VALUE_TOTAL_MOUSE_MOVEMENT,
      STATS_VALUE_TOTAL_CLICK_MOVEMENT,
      STATS_VALUE_TOTAL_MOVEMENT_TIME,
      STATS_VALUE_TOTAL_CLICKS,
      STATS_VALUE_TOTAL_KEYSTROKES,
      STATS_VALUE_SIZEOF
    };

    using BreakStats = int[STATS_BREAKVALUE_SIZEOF];
    using MiscStats = int64_t[STATS_VALUE_SIZEOF];

    struct DailyStats
    {
      //! When this day started, in local time.
      workrave::stats::LocalTime start{};

      //! When the user was last active on this day, in local time.
      workrave::stats::LocalTime stop{};

      //! Statistic of each break
      BreakStats break_stats[BREAK_ID_SIZEOF];

      //! Misc statistics
      MiscStats misc_stats;
    };

    //! A local calendar date, as statistics are kept per calendar day.
    using Date = workrave::stats::Date;

  public:
    virtual ~IStatistics() = default;

    virtual bool delete_all_history() = 0;
    virtual void update() = 0;
    virtual void dump() = 0;

    //! The day in progress, which is being counted right now.
    virtual DailyStats *get_current_day() const = 0;

    //! The statistics of a single date, today included.
    virtual std::optional<DailyStats> get_day(const Date &date) const = 0;

    //! The dates in the inclusive range that have statistics, oldest first.
    virtual std::vector<Date> get_dates(const Date &from, const Date &to) const = 0;

    //! The nearest date with statistics before the given date.
    virtual std::optional<Date> get_previous_date(const Date &date) const = 0;

    //! The nearest date with statistics after the given date.
    virtual std::optional<Date> get_next_date(const Date &date) const = 0;

    //! The oldest date with statistics.
    virtual std::optional<Date> get_first_date() const = 0;

    //! The most recent date with statistics, which is normally today.
    virtual std::optional<Date> get_last_date() const = 0;

    //! The total active time over the inclusive date range.
    virtual int64_t get_total_active_time(const Date &from, const Date &to) const = 0;
  };
} // namespace workrave

#endif // WORKRAVE_BACKEND_ISTATISTICS_HH
