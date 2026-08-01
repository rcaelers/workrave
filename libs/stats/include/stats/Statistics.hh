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
#include <mutex>
#include <optional>
#include <vector>

#include "input-monitor/IInputMonitor.hh"
#include "input-monitor/IInputMonitorListener.hh"

#include "core/IStatistics.hh"
#include "stats/IStatisticsContext.hh"
#include "stats/IStatisticsStore.hh"

namespace workrave::stats
{
  //! Counts and stores what the user did today, and what they did before.
  class Statistics
    : public workrave::IStatistics
    , public workrave::input_monitor::IInputMonitorListener
  {
  public:
    using Ptr = std::shared_ptr<Statistics>;

  private:
    struct DailyStatsImpl : public workrave::IStatistics::DailyStats
    {
      //! Total time that the mouse was moving.
      std::chrono::system_clock::time_point total_mouse_time;

      DailyStatsImpl()
        : DailyStats()
      {
        for (auto &break_stat: break_stats)
          {
            for (int &stat: break_stat)
              {
                stat = 0;
              }
          }

        for (int64_t &misc_stat: misc_stats)
          {
            misc_stat = 0;
          }
      }

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
    void init();
    void update() override;
    void dump() override;
    void start_new_day();

    void increment_break_counter(workrave::BreakId, StatsBreakValueType st);
    void set_break_counter(workrave::BreakId bt, StatsBreakValueType st, int value);
    void add_break_counter(workrave::BreakId bt, StatsBreakValueType st, int value);

    DailyStatsImpl *get_current_day() const override;
    std::optional<DailyStats> get_day(const Date &date) const override;
    std::vector<Date> get_dates(const Date &from, const Date &to) const override;
    std::optional<Date> get_previous_date(const Date &date) const override;
    std::optional<Date> get_next_date(const Date &date) const override;
    std::optional<Date> get_first_date() const override;
    std::optional<Date> get_last_date() const override;
    int64_t get_total_active_time(const Date &from, const Date &to) const override;
    void set_counter(StatsValueType t, int value);
    int64_t get_counter(StatsValueType t);

  private:
    void action_notify() override;
    void mouse_notify(int x, int y, int wheel = 0) override;
    void button_notify(bool is_press) override;
    void keyboard_notify(bool repeat) override;

    bool load_current_day();

  private:
    void save_day(DailyStatsImpl *stats);

    static DailyStatsRecord to_record(const DailyStatsImpl *stats);
    static DailyStatsImpl from_record(const DailyStatsRecord &record);

    void day_to_history(DailyStatsImpl *stats);

  private:
    //! What this core does differently.
    IStatisticsContext::Ptr context;

    //! Persistent storage of the statistics.
    IStatisticsStore::Ptr store;

    //! Mouse/Keyboard monitoring.
    workrave::input_monitor::IInputMonitor::Ptr input_monitor;

    //! Last time a mouse event was received.
    std::chrono::system_clock::time_point last_mouse_time;

    //! Statistics of current day.
    DailyStatsImpl *current_day;

    //! Has the user been active on the current day?
    bool been_active;

    //! Internal locking
    std::mutex lock;

    //! Previous X coordinate
    int prev_x;

    //! Previous Y coordinate
    int prev_y;

    //! Previous X-click coordinate
    int click_x;

    //! Previous Y-click coordinate
    int click_y;
  };
} // namespace workrave::stats

#endif // WORKRAVE_LIBS_STATS_STATISTICS_HH
