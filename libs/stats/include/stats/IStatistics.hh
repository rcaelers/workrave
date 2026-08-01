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

#include <array>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#if defined(PLATFORM_OS_WINDOWS_NATIVE)
typedef __int64 int64_t;
#else
#  include <cstdint>
#endif

#include "core/CoreTypes.hh"
#include "stats/LocalTime.hh"
#include "stats/StatValue.hh"
#include "utils/Enum.hh"

namespace workrave::stats
{
  //! How often the user was prompted for, took, skipped or postponed a break today.
  enum class BreakStatValue
  {
    Prompted = 0,
    Taken,
    NaturalTaken,
    Skipped,
    Postponed,
    UniqueBreaks,
  };
} // namespace workrave::stats

template<>
struct workrave::utils::enum_traits<workrave::stats::BreakStatValue>
{
  static constexpr auto min = workrave::stats::BreakStatValue::Prompted;
  static constexpr auto max = workrave::stats::BreakStatValue::UniqueBreaks;
  static constexpr auto linear = true;
};

namespace workrave::stats
{
  static constexpr auto STATS_BREAKVALUE_SIZEOF = workrave::utils::enum_count<BreakStatValue>();

  class IStatistics
  {
  public:
    using Ptr = std::shared_ptr<IStatistics>;

    //! The break-prompt/taken/skipped/postponed counts of a single break, today.
    using BreakCounters = workrave::utils::array<BreakStatValue, workrave::stats::StatValue<int64_t>>;

    struct DailyStats
    {
      //! When the user was first observed active today. Unset until then.
      std::optional<workrave::stats::LocalTime> start;

      //! When the user was last observed active today. Unset the same way as start.
      std::optional<workrave::stats::LocalTime> stop;

      //! How often the user was prompted for, took, skipped or postponed each break today.
      std::array<BreakCounters, BREAK_ID_SIZEOF> break_stats{};

      //! How long each break has been overdue today. Recomputed continuously.
      std::array<workrave::stats::StatValue<std::chrono::seconds>, BREAK_ID_SIZEOF> total_overdue{};

      //! How long the user has been active today. Recomputed continuously.
      workrave::stats::StatValue<std::chrono::seconds> total_active_time;

      //! A day that was never started, as opposed to one without any activity.
      //! start and stop are always set together, but both are checked so that
      //! callers never need to assume that invariant to dereference safely.
      [[nodiscard]] bool is_empty() const
      {
        return !start.has_value() || !stop.has_value();
      }
    };

    //! A local calendar date, as statistics are kept per calendar day.
    using Date = std::chrono::year_month_day;

  public:
    virtual ~IStatistics() = default;

    virtual void start_new_day() = 0;

    virtual bool delete_all_history() = 0;

    //! Refreshes in-memory bookkeeping (activity start/stop). Cheap: safe to call
    //! every heartbeat.
    virtual void update() = 0;

    //! Persists the day in progress. Causes disk I/O: callers should rate-limit
    //! this to avoid wearing out storage.
    virtual void save() = 0;

    //! The day in progress, which is being counted right now.
    virtual DailyStats *get_current_day() const = 0;

    //! Today's break-prompt/taken/skipped/postponed count. Live: add()/set() persist.
    virtual workrave::stats::StatValue<int64_t> &break_counter(workrave::BreakId break_id, BreakStatValue type) = 0;

    //! How long a break has been overdue today. Live: set() updates it in place.
    virtual workrave::stats::StatValue<std::chrono::seconds> &total_overdue(workrave::BreakId break_id) = 0;

    //! How long the user has been active today. Live: set() updates it in place.
    virtual workrave::stats::StatValue<std::chrono::seconds> &total_active_time() = 0;

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
    virtual std::chrono::seconds get_total_active_time(const Date &from, const Date &to) const = 0;
  };

  //! Creates the statistics. is_active reports whether the user is active right now.
  std::shared_ptr<IStatistics> create(std::function<bool()> is_active);

} // namespace workrave::stats

#endif // WORKRAVE_BACKEND_ISTATISTICS_HH
