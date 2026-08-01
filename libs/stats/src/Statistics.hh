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
#include <memory>
#include <optional>
#include <vector>

#include "stats/IStatistics.hh"
#include "stats/IStatisticsContext.hh"
#include "IStatisticsStore.hh"

namespace workrave::stats
{ //! Counts and stores what the user did today, and what they did before.
  class Statistics : public workrave::stats::IStatistics
  {
  public:
    using Ptr = std::shared_ptr<Statistics>;

  private:
    struct DailyStatsImpl : public workrave::stats::IStatistics::DailyStats
    {
      //! A day that was never started, as opposed to one without any activity.
      bool is_empty() const
      {
        return start == LocalTime{};
      }
    };

  public:
    explicit Statistics(IStatisticsContext::Ptr context);
    ~Statistics() override;

    bool delete_all_history() override;

  public:
    void init() override;
    void update() override;
    void dump() override;
    void start_new_day() override;

    DailyStatsImpl *get_current_day() const override;
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
    void save_day(DailyStatsImpl *stats);

    static DailyStatsRecord to_record(const DailyStatsImpl *stats);
    static DailyStatsImpl from_record(const DailyStatsRecord &record);

    void day_to_history(DailyStatsImpl *stats);

    //! Binds the break counters of the given day to write straight through to the
    //! store. total_overdue/total_active_time are left unbound, since they stay
    //! buffered.
    void wire_write_through(DailyStatsImpl &day);

  private:
    //! What this core does differently.
    IStatisticsContext::Ptr context;

    //! Persistent storage of the statistics.
    IStatisticsStore::Ptr store;

    //! Statistics of current day.
    DailyStatsImpl *current_day;

    //! Has the user been active on the current day?
    bool been_active;
  };
} // namespace workrave::stats

#endif // WORKRAVE_LIBS_STATS_STATISTICS_HH
