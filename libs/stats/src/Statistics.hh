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

#ifndef WORKRAVE_LIBS_STATS_STATISTICS_HH
#define WORKRAVE_LIBS_STATS_STATISTICS_HH

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "stats/IStatistics.hh"
#include "IStatisticsStore.hh"

namespace workrave::stats
{
  //! Counts and stores what the user did today, and what they did before.
  class Statistics : public workrave::stats::IStatistics
  {
  public:
    using Ptr = std::shared_ptr<Statistics>;

    explicit Statistics(std::function<bool()> is_active);
    ~Statistics() override;

  public:
    void init();

    void update() override;
    void start_new_day() override;
    bool delete_all_history() override;

    DailyStats *get_current_day() const override;
    StatValue<int64_t> &break_counter(workrave::BreakId break_id, BreakStatValue type) override;
    StatValue<std::chrono::seconds> &total_overdue(workrave::BreakId break_id) override;
    StatValue<std::chrono::seconds> &total_active_time() override;
    std::optional<DailyStats> get_day(const Date &date) const override;
    std::vector<Date> get_dates(const Date &from, const Date &to) const override;
    std::optional<Date> get_previous_date(const Date &date) const override;
    std::optional<Date> get_next_date(const Date &date) const override;
    std::optional<Date> get_first_date() const override;
    std::optional<Date> get_last_date() const override;
    std::chrono::seconds get_total_active_time(const Date &from, const Date &to) const override;

  private:
    bool load_current_day();

  private:
    void save_day(DailyStats *stats);

    static DailyStatsRecord to_record(const DailyStats *stats);
    static DailyStats from_record(const DailyStatsRecord &record);

    void day_to_history(DailyStats *stats);
    void wire_write_through(DailyStats &day);

  private:
    //! Whether the user is active right now.
    std::function<bool()> is_active;

    //! Persistent storage of the statistics.
    IStatisticsStore::Ptr store;

    //! Statistics of current day.
    DailyStats *current_day;
  };
} // namespace workrave::stats

#endif // WORKRAVE_LIBS_STATS_STATISTICS_HH
