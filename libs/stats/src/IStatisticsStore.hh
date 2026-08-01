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

#ifndef WORKRAVE_LIBS_STATS_ISTATISTICSSTORE_HH
#define WORKRAVE_LIBS_STATS_ISTATISTICSSTORE_HH

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include "core/CoreTypes.hh"
#include "DailyStatsRecord.hh"

namespace workrave::stats
{
  //! Persistent storage for daily statistics.
  class IStatisticsStore
  {
  public:
    using Ptr = std::shared_ptr<IStatisticsStore>;

    virtual ~IStatisticsStore() = default;

    //! Loads the statistics of the day that was in progress when Workrave last stopped.
    [[nodiscard]] virtual std::optional<DailyStatsRecord> load_today() = 0;

    //! Loads all archived days, oldest first.
    [[nodiscard]] virtual std::vector<DailyStatsRecord> load_history() = 0;

    //! Stores the statistics of the day currently in progress.
    virtual void save_today(const DailyStatsRecord &record) = 0;

    //! Sets a single break counter of the day in progress to an absolute value, immediately and durably.
    virtual void set_break_counter(workrave::BreakId break_id, int counter, int64_t value) = 0;

    //! Archives a completed day.
    virtual void append_history(const DailyStatsRecord &record) = 0;

    //! Removes all stored statistics, both today's and the archive.
    virtual bool delete_all() = 0;

    //! Loads the statistics of a single date, today included.
    [[nodiscard]] virtual std::optional<DailyStatsRecord> load_date(const Date &date) = 0;

    //! The dates in the inclusive range that have statistics, oldest first.
    [[nodiscard]] virtual std::vector<Date> get_dates(const Date &from, const Date &to) = 0;

    //! The nearest date with statistics before the given date.
    [[nodiscard]] virtual std::optional<Date> get_previous_date(const Date &date) = 0;

    //! The nearest date with statistics after the given date.
    [[nodiscard]] virtual std::optional<Date> get_next_date(const Date &date) = 0;

    //! The oldest date with statistics.
    [[nodiscard]] virtual std::optional<Date> get_first_date() = 0;

    //! The most recent date with statistics.
    [[nodiscard]] virtual std::optional<Date> get_last_date() = 0;

    //! The sum of one of the miscellaneous counters over the inclusive date range.
    [[nodiscard]] virtual int64_t get_total_misc(int counter, const Date &from, const Date &to) = 0;
  };

  class StatisticsStoreFactory
  {
  public:
    //! Creates the live store for the given state directory. Returns nullptr if no
    //! writable store could be opened, leaving the caller without persistence for
    //! this session.
    static IStatisticsStore::Ptr create(const std::filesystem::path &state_directory);
  };
} // namespace workrave::stats

#endif // WORKRAVE_LIBS_STATS_ISTATISTICSSTORE_HH
