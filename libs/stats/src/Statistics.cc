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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "stats/Statistics.hh"

#if defined(PLATFORM_OS_MACOS)
#  include "MacOSHelpers.hh"
#endif

#include <algorithm>
#include <cstring>
#include <optional>
#include <sstream>
#include <cassert>
#include <cmath>

#include "debug.hh"

#include "utils/Paths.hh"
#include "input-monitor/InputMonitorFactory.hh"
#include "input-monitor/IInputMonitor.hh"

#define MAX_JUMP (10000)

using namespace std;
using namespace workrave;
using namespace workrave::utils;
using namespace workrave::input_monitor;

namespace workrave::stats
{

  Statistics::Statistics(IStatisticsContext::Ptr context)
    : context(std::move(context))
    , current_day(nullptr)
    , been_active(false)
    , prev_x(-1)
    , prev_y(-1)
    , click_x(-1)
    , click_y(-1)
  {
  }

  //! Destructor
  Statistics::~Statistics()
  {
    update();

    delete current_day;

    if (input_monitor != nullptr)
      {
        input_monitor->unsubscribe(this);
      }
  }

  //! Initializes the Statistics.
  void Statistics::init()
  {
    input_monitor = InputMonitorFactory::create_monitor(MonitorCapability::Statistics);
    if (input_monitor != nullptr)
      {
        input_monitor->subscribe(this);
      }

    store = StatisticsStoreFactory::create(Paths::get_state_directory());

    current_day = nullptr;
    bool ok = load_current_day();
    if (!ok)
      {
        start_new_day();
      }
  }

  //! Periodic heartbeat.
  void Statistics::update()
  {
    TRACE_ENTRY();
    if (context->is_active())
      {
        const LocalTime now = local_now();
        current_day->stop = now;

        if (!been_active)
          {
            current_day->start = now;
            been_active = true;
          }
      }

    // A core that derives counters from its own timers fills them in here.
    context->update_counters(*this);

    save_day(current_day);
  }

  bool Statistics::delete_all_history()
  {
    update();

    if (!store->delete_all())
      {
        return false;
      }

    if (current_day != nullptr)
      {
        delete current_day;
        current_day = nullptr;
      }
    start_new_day();

    return true;
  }

  //! Starts a new day and archive current any (if exists)
  void Statistics::start_new_day()
  {
    TRACE_ENTRY();
    const LocalTime now = local_now();

    if (current_day == nullptr || date_of(now) != date_of(current_day->start))
      {
        TRACE_MSG("New day");
        if (current_day != nullptr)
          {
            TRACE_MSG("Save old day");
            day_to_history(current_day);
          }

        current_day = new DailyStatsImpl();
        been_active = false;

        current_day->start = now;
        current_day->stop = now;
      }

    update();
    save_day(current_day);
  }

  void Statistics::day_to_history(DailyStatsImpl *stats)
  {
    if (store != nullptr)
      {
        store->append_history(to_record(stats));
      }
  }

  //! Converts a day to the representation used by the statistics store.
  DailyStatsRecord Statistics::to_record(const DailyStatsImpl *stats)
  {
    DailyStatsRecord record;

    record.start = stats->start;
    record.stop = stats->stop;

    record.break_stats.resize(BREAK_ID_SIZEOF);
    for (int i = 0; i < BREAK_ID_SIZEOF; i++)
      {
        const BreakStats &bs = stats->break_stats[i];
        record.break_stats[i].assign(std::begin(bs), std::end(bs));
      }

    record.misc_stats.assign(std::begin(stats->misc_stats), std::end(stats->misc_stats));

    return record;
  }

  //! Converts a day from the representation used by the statistics store.
  /*!
   *  Counters the store knows about but this version of Workrave does not are
   *  discarded, and counters this version expects but the store does not have
   *  keep their initial value.
   */
  Statistics::DailyStatsImpl Statistics::from_record(const DailyStatsRecord &record)
  {
    DailyStatsImpl stats;

    stats.start = record.start;
    stats.stop = record.stop;

    const size_t breaks = std::min<size_t>(record.break_stats.size(), BREAK_ID_SIZEOF);
    for (size_t i = 0; i < breaks; i++)
      {
        const size_t counters = std::min<size_t>(record.break_stats[i].size(), STATS_BREAKVALUE_SIZEOF);
        for (size_t j = 0; j < counters; j++)
          {
            stats.break_stats[i][j] = static_cast<int>(record.break_stats[i][j]);
          }
      }

    const size_t counters = std::min<size_t>(record.misc_stats.size(), STATS_VALUE_SIZEOF);
    for (size_t i = 0; i < counters; i++)
      {
        stats.misc_stats[i] = record.misc_stats[i];
      }

    return stats;
  }

  //! Saves the statistics of the specified day.
  void Statistics::save_day(DailyStatsImpl *stats)
  {
    if (store != nullptr && stats != nullptr)
      {
        store->save_today(to_record(stats));
      }
  }

  //! Load the statistics of the current day.
  bool Statistics::load_current_day()
  {
    TRACE_ENTRY();
    if (store != nullptr)
      {
        std::optional<DailyStatsRecord> record = store->load_today();
        if (record.has_value())
          {
            current_day = new DailyStatsImpl(from_record(record.value()));
          }
      }

    been_active = true;

    return current_day != nullptr;
  }

  //! Increment the specified statistics counter of the current day.
  void Statistics::increment_break_counter(BreakId bt, StatsBreakValueType st)
  {
    if (current_day == nullptr)
      {
        start_new_day();
      }

    BreakStats &bs = current_day->break_stats[bt];
    bs[st]++;
  }

  void Statistics::set_break_counter(BreakId bt, StatsBreakValueType st, int value)
  {
    if (current_day == nullptr)
      {
        start_new_day();
      }

    BreakStats &bs = current_day->break_stats[bt];
    bs[st] = value;
  }

  void Statistics::add_break_counter(BreakId bt, StatsBreakValueType st, int value)
  {
    if (current_day == nullptr)
      {
        start_new_day();
      }

    BreakStats &bs = current_day->break_stats[bt];
    bs[st] += value;
  }

  void Statistics::set_counter(StatsValueType t, int value)
  {
    current_day->misc_stats[t] = value;
  }

  int64_t Statistics::get_counter(StatsValueType t)
  {
    return current_day->misc_stats[t];
  }

  //! Dump
  void Statistics::dump()
  {
    TRACE_ENTRY();
    update();

    stringstream ss;
    for (int i = 0; i < BREAK_ID_SIZEOF; i++)
      {
        BreakStats &bs = current_day->break_stats[i];

        ss << "Break " << i << " ";
        for (auto value: bs)
          {
            ss << value << " ";
          }
      }

    ss << "stats ";
    for (int64_t value: current_day->misc_stats)
      {
        ss << value << " ";
      }
  }

  Statistics::DailyStatsImpl *Statistics::get_current_day() const
  {
    return current_day;
  }

  std::optional<IStatistics::DailyStats> Statistics::get_day(const Date &date) const
  {
    // The day in progress is only written to the store now and then, so its
    // counters are read straight from memory.
    if (current_day != nullptr && date_of(current_day->start) == date)
      {
        return *current_day;
      }

    if (store == nullptr)
      {
        return std::nullopt;
      }

    std::optional<DailyStatsRecord> record = store->load_date(date);
    if (!record.has_value())
      {
        return std::nullopt;
      }

    return from_record(record.value());
  }

  std::vector<IStatistics::Date> Statistics::get_dates(const Date &from, const Date &to) const
  {
    if (store == nullptr)
      {
        return {};
      }

    return store->get_dates(from, to);
  }

  std::optional<IStatistics::Date> Statistics::get_previous_date(const Date &date) const
  {
    return store != nullptr ? store->get_previous_date(date) : std::nullopt;
  }

  std::optional<IStatistics::Date> Statistics::get_next_date(const Date &date) const
  {
    return store != nullptr ? store->get_next_date(date) : std::nullopt;
  }

  std::optional<IStatistics::Date> Statistics::get_first_date() const
  {
    return store != nullptr ? store->get_first_date() : std::nullopt;
  }

  std::optional<IStatistics::Date> Statistics::get_last_date() const
  {
    return store != nullptr ? store->get_last_date() : std::nullopt;
  }

  int64_t Statistics::get_total_active_time(const Date &from, const Date &to) const
  {
    if (store == nullptr)
      {
        return 0;
      }

    int64_t total = store->get_total_misc(STATS_VALUE_TOTAL_ACTIVE_TIME, from, to);

    // Correct the stored value of the day in progress, which is behind by up to
    // one save interval, with what has been counted since.
    if (current_day != nullptr)
      {
        const Date today = date_of(current_day->start);
        if (today >= from && today <= to)
          {
            std::optional<DailyStatsRecord> stored = store->load_date(today);
            if (stored.has_value() && stored->misc_stats.size() > STATS_VALUE_TOTAL_ACTIVE_TIME)
              {
                total -= stored->misc_stats[STATS_VALUE_TOTAL_ACTIVE_TIME];
              }
            total += current_day->misc_stats[STATS_VALUE_TOTAL_ACTIVE_TIME];
          }
      }

    return total;
  }

  //! Activity is reported by the input monitor.
  void Statistics::action_notify()
  {
  }

  //! Mouse activity is reported by the input monitor.
  void Statistics::mouse_notify(int x, int y, int wheel_delta)
  {
    static const int sensitivity = 3;

    lock.lock();

    if (current_day != nullptr && x >= 0 && y >= 0)
      {
        int delta_x = sensitivity;
        int delta_y = sensitivity;

        if (prev_x != -1 && prev_y != -1)
          {
            delta_x = abs(x - prev_x);
            delta_y = abs(y - prev_y);
          }

        prev_x = x;
        prev_y = y;

        // Sanity checks, ignore unreasonable large jumps...
        if (delta_x < MAX_JUMP && delta_y < MAX_JUMP && (delta_x >= sensitivity || delta_y >= sensitivity || wheel_delta != 0))
          {
            int64_t movement = current_day->misc_stats[STATS_VALUE_TOTAL_MOUSE_MOVEMENT];
            int distance = int(sqrt(static_cast<double>(delta_x * delta_x + delta_y * delta_y)));

            movement += distance;
            if (movement > 0)
              {
                current_day->misc_stats[STATS_VALUE_TOTAL_MOUSE_MOVEMENT] = movement;
              }

            auto now = std::chrono::system_clock::now();
            auto tv = now - last_mouse_time;

            if (tv < std::chrono::seconds(1))
              {
                current_day->total_mouse_time += tv;
                current_day->misc_stats[STATS_VALUE_TOTAL_MOVEMENT_TIME] = std::chrono::duration_cast<std::chrono::seconds>(
                                                                             current_day->total_mouse_time.time_since_epoch())
                                                                             .count();
              }

            last_mouse_time = now;
          }
      }

    lock.unlock();
  }

  //! Mouse button activity is reported by the input monitor.
  void Statistics::button_notify(bool is_press)
  {
    lock.lock();
    if (current_day != nullptr)
      {
        if (click_x != -1 && click_y != -1 && prev_x != -1 && prev_y != -1)
          {
            int delta_x = click_x - prev_x;
            int delta_y = click_y - prev_y;

            int64_t movement = current_day->misc_stats[STATS_VALUE_TOTAL_CLICK_MOVEMENT];
            int64_t distance = int(sqrt(static_cast<double>(delta_x * delta_x + delta_y * delta_y)));

            movement += distance;
            if (movement > 0)
              {
                current_day->misc_stats[STATS_VALUE_TOTAL_CLICK_MOVEMENT] = movement;
              }
          }

        click_x = prev_x;
        click_y = prev_y;

        if (is_press)
          {
            current_day->misc_stats[STATS_VALUE_TOTAL_CLICKS]++;
          }
      }
    lock.unlock();
  }

  //! Keyboard activity is reported by the input monitor.
  void Statistics::keyboard_notify(bool repeat)
  {
    if (repeat)
      {
        return;
      }

    lock.lock();
    if (current_day != nullptr)
      {
        current_day->misc_stats[STATS_VALUE_TOTAL_KEYSTROKES]++;
      }
    lock.unlock();
  }
} // namespace workrave::stats
