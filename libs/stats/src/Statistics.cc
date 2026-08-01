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

#include "Statistics.hh"

#if defined(PLATFORM_OS_MACOS)
#  include "MacOSHelpers.hh"
#endif

#include <algorithm>
#include <cstring>
#include <optional>
#include <cassert>

#include "debug.hh"

#include "utils/EnumIterator.hh"
#include "utils/Paths.hh"

using namespace std;
using namespace workrave;
using namespace workrave::utils;

namespace
{
  //! Where total_overdue sits within each break's counter row in the store, matching
  //! where STATS_BREAKVALUE_TOTAL_OVERDUE used to sit before it became its own field.
  constexpr size_t OVERDUE_STORAGE_INDEX = static_cast<size_t>(workrave::stats::STATS_BREAKVALUE_SIZEOF);

  //! Where total_active_time sits within misc_stats in the store, matching where
  //! STATS_VALUE_TOTAL_ACTIVE_TIME used to sit before it became its own field.
  constexpr size_t ACTIVE_TIME_STORAGE_INDEX = 0;
} // namespace

using namespace workrave::stats;

std::shared_ptr<IStatistics>
workrave::stats::create(std::shared_ptr<IStatisticsContext> context)
{
  auto statistics = std::make_shared<Statistics>(context);
  statistics->init();
  return statistics;
}

Statistics::Statistics(IStatisticsContext::Ptr context)
  : context(std::move(context))
  , current_day(nullptr)
{
}

//! Destructor
Statistics::~Statistics()
{
  update();

  delete current_day;
}

//! Initializes the Statistics.
void
Statistics::init()
{
  store = StatisticsStoreFactory::create(Paths::get_state_directory());

  current_day = nullptr;
  bool ok = load_current_day();
  if (!ok)
    {
      start_new_day();
    }
}

//! Periodic heartbeat.
void
Statistics::update()
{
  TRACE_ENTRY();
  if (context->is_active())
    {
      const LocalTime now = local_now();

      if (!current_day->start.has_value())
        {
          current_day->start = now;
        }
      current_day->stop = now;
    }

  save_day(current_day);
}

bool
Statistics::delete_all_history()
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

//! Starts a new day and archives the current one (if it saw any activity).
void
Statistics::start_new_day()
{
  TRACE_ENTRY();
  const Date today = date_of(local_now());

  // A day without activity has nothing worth archiving, so it is simply kept
  // around and reused until either activity or a genuine day change occurs.
  const bool day_changed = current_day != nullptr && current_day->start.has_value() && date_of(*current_day->start) != today;

  if (current_day == nullptr || day_changed)
    {
      if (day_changed)
        {
          TRACE_MSG("Save old day");
          day_to_history(current_day);
        }

      delete current_day;
      current_day = new DailyStats();
      wire_write_through(*current_day);
    }

  update();
  save_day(current_day);
}

void
Statistics::day_to_history(DailyStats *stats)
{
  if (store != nullptr)
    {
      store->append_history(to_record(stats));
    }
}

//! Converts a day to the representation used by the statistics store.
DailyStatsRecord
Statistics::to_record(const DailyStats *stats)
{
  DailyStatsRecord record;

  // A day that has not seen any activity yet has no start/stop of its own; the
  // store still needs a concrete time, so "now" is used as a placeholder until
  // real activity sets one. This only ever backs the transient today-in-progress
  // snapshot: an empty day is never archived to history (see start_new_day()).
  const LocalTime now = local_now();
  record.start = stats->start.value_or(now);
  record.stop = stats->stop.value_or(now);

  record.break_stats.resize(BREAK_ID_SIZEOF);
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      std::vector<int64_t> &counters = record.break_stats[i];
      counters.resize(OVERDUE_STORAGE_INDEX + 1);

      for (auto type: workrave::utils::enum_range<BreakStatValue>())
        {
          counters[static_cast<size_t>(type)] = stats->break_stats[i][type].to_storage();
        }
      counters[OVERDUE_STORAGE_INDEX] = stats->total_overdue[i].to_storage();
    }

  record.misc_stats.resize(ACTIVE_TIME_STORAGE_INDEX + 1);
  record.misc_stats[ACTIVE_TIME_STORAGE_INDEX] = stats->total_active_time.to_storage();

  return record;
}

//! Converts a day from the representation used by the statistics store.
/*!
 *  Counters the store knows about but this version of Workrave does not are
 *  discarded, and counters this version expects but the store does not have
 *  keep their initial value.
 */
Statistics::DailyStats
Statistics::from_record(const DailyStatsRecord &record)
{
  DailyStats stats;

  stats.start = record.start;
  stats.stop = record.stop;

  const size_t breaks = std::min<size_t>(record.break_stats.size(), BREAK_ID_SIZEOF);
  for (size_t i = 0; i < breaks; i++)
    {
      const std::vector<int64_t> &counters = record.break_stats[i];

      const size_t known = std::min<size_t>(counters.size(), static_cast<size_t>(workrave::stats::STATS_BREAKVALUE_SIZEOF));
      for (size_t j = 0; j < known; j++)
        {
          stats.break_stats[i][j].from_storage(counters[j]);
        }

      if (counters.size() > OVERDUE_STORAGE_INDEX)
        {
          stats.total_overdue[i].from_storage(counters[OVERDUE_STORAGE_INDEX]);
        }
    }

  if (record.misc_stats.size() > ACTIVE_TIME_STORAGE_INDEX)
    {
      stats.total_active_time.from_storage(record.misc_stats[ACTIVE_TIME_STORAGE_INDEX]);
    }

  return stats;
}

//! Saves the statistics of the specified day.
void
Statistics::save_day(DailyStats *stats)
{
  if (store != nullptr && stats != nullptr)
    {
      store->save_today(to_record(stats));
    }
}

//! Load the statistics of the current day.
bool
Statistics::load_current_day()
{
  TRACE_ENTRY();
  if (store != nullptr)
    {
      std::optional<DailyStatsRecord> record = store->load_today();
      if (record.has_value())
        {
          current_day = new DailyStats(from_record(record.value()));
          wire_write_through(*current_day);
        }
    }

  return current_day != nullptr;
}

//! Binds the break counters of the given day to write straight through to the
//! store. total_overdue/total_active_time are left unbound, since they stay buffered.
void
Statistics::wire_write_through(DailyStats &day)
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      for (auto type: workrave::utils::enum_range<BreakStatValue>())
        {
          const auto break_id = static_cast<BreakId>(i);
          const int counter = static_cast<int>(type);
          day.break_stats[i][type].on_change([this, break_id, counter](int64_t value) {
            if (store != nullptr)
              {
                store->set_break_counter(break_id, counter, value);
              }
          });
        }
    }
}

Statistics::DailyStats *
Statistics::get_current_day() const
{
  return current_day;
}

StatValue<int64_t> &
Statistics::break_counter(BreakId break_id, BreakStatValue type)
{
  return current_day->break_stats[break_id][type];
}

StatValue<std::chrono::seconds> &
Statistics::total_overdue(BreakId break_id)
{
  return current_day->total_overdue[break_id];
}

StatValue<std::chrono::seconds> &
Statistics::total_active_time()
{
  return current_day->total_active_time;
}

std::optional<IStatistics::DailyStats>
Statistics::get_day(const Date &date) const
{
  // The day in progress is only written to the store now and then, so its
  // counters are read straight from memory.
  if (current_day != nullptr && current_day->start.has_value() && date_of(*current_day->start) == date)
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

std::vector<IStatistics::Date>
Statistics::get_dates(const Date &from, const Date &to) const
{
  if (store == nullptr)
    {
      return {};
    }

  return store->get_dates(from, to);
}

std::optional<IStatistics::Date>
Statistics::get_previous_date(const Date &date) const
{
  return store != nullptr ? store->get_previous_date(date) : std::nullopt;
}

std::optional<IStatistics::Date>
Statistics::get_next_date(const Date &date) const
{
  return store != nullptr ? store->get_next_date(date) : std::nullopt;
}

std::optional<IStatistics::Date>
Statistics::get_first_date() const
{
  return store != nullptr ? store->get_first_date() : std::nullopt;
}

std::optional<IStatistics::Date>
Statistics::get_last_date() const
{
  return store != nullptr ? store->get_last_date() : std::nullopt;
}

std::chrono::seconds
Statistics::get_total_active_time(const Date &from, const Date &to) const
{
  if (store == nullptr)
    {
      return std::chrono::seconds::zero();
    }

  int64_t total = store->get_total_misc(static_cast<int>(ACTIVE_TIME_STORAGE_INDEX), from, to);

  // Correct the stored value of the day in progress, which is behind by up to
  // one save interval, with what has been counted since.
  if (current_day != nullptr && current_day->start.has_value())
    {
      const Date today = date_of(*current_day->start);
      if (today >= from && today <= to)
        {
          std::optional<DailyStatsRecord> stored = store->load_date(today);
          if (stored.has_value() && stored->misc_stats.size() > ACTIVE_TIME_STORAGE_INDEX)
            {
              total -= stored->misc_stats[ACTIVE_TIME_STORAGE_INDEX];
            }
          total += current_day->total_active_time.get().count();
        }
    }

  return std::chrono::seconds{total};
}
// namespace workrave::stats
