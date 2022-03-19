// Copyright (C) 2002 - 2010 Rob Caelers & Raymond Penners
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

#ifndef STATISTICS_HH
#define STATISTICS_HH

#include <memory>
#include <thread>
#include <mutex>

#include <chrono>

#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <cstring>

#include "core/IStatistics.hh"
#include "input-monitor/IInputMonitor.hh"
#include "input-monitor/IInputMonitorListener.hh"
#include "core/IStatistics.hh"

// Forward declarion of external interface.
namespace workrave
{
  class IBreak;
}

class TimePred;
class PacketBuffer;
class Core;
class IInputMonitor;

class Statistics
  : public workrave::IStatistics
  , public workrave::input_monitor::IInputMonitorListener
{
private:
  enum StatsMarker
  {
    STATS_MARKER_TODAY,
    STATS_MARKER_HISTORY,
    STATS_MARKER_END,
    STATS_MARKER_STARTTIME,
    STATS_MARKER_STOPTIME,
    STATS_MARKER_BREAK_STATS,
    STATS_MARKER_MISC_STATS,
  };

  struct DailyStatsImpl : public DailyStats
  {
    //! Total time that the mouse was moving.
    std::chrono::system_clock::time_point total_mouse_time;

    DailyStatsImpl()
    {
      memset((void *)&start, 0, sizeof(start));
      memset((void *)&stop, 0, sizeof(stop));

      for (int i = 0; i < workrave::BREAK_ID_SIZEOF; i++)
        {
          for (int j = 0; j < STATS_BREAKVALUE_SIZEOF; j++)
            {
              break_stats[i][j] = 0;
            }
        }

      for (int j = 0; j < STATS_VALUE_SIZEOF; j++)
        {
          misc_stats[j] = 0;
        }

      // Empty marker.
      start.tm_year = 0;
    }

    bool starts_at_date(int y, int m, int d);
    bool starts_before_date(int y, int m, int d);
    bool is_empty() const
    {
      return start.tm_year == 0;
    }
  };

  typedef std::vector<DailyStatsImpl *> History;
  typedef std::vector<DailyStatsImpl *>::iterator HistoryIter;
  typedef std::vector<DailyStatsImpl *>::reverse_iterator HistoryRIter;

public:
  Statistics() = default;
  ~Statistics() override;

  bool delete_all_history() override;

public:
  void init(Core *core);
  void update() override;
  void dump() override;
  void start_new_day();

  void increment_break_counter(workrave::BreakId, StatsBreakValueType st);
  void set_break_counter(workrave::BreakId bt, StatsBreakValueType st, int value);
  void add_break_counter(workrave::BreakId bt, StatsBreakValueType st, int value);

  DailyStatsImpl *get_current_day() const override;
  DailyStatsImpl *get_day(int day) const override;
  void get_day_index_by_date(int y, int m, int d, int &idx, int &next, int &prev) const override;

  int get_history_size() const override;
  void set_counter(StatsValueType t, int value);
  int64_t get_counter(StatsValueType t);

private:
  void action_notify() override;
  void mouse_notify(int x, int y, int wheel = 0) override;
  void button_notify(bool is_press) override;
  void keyboard_notify(bool repeat) override;

  bool load_current_day();
  void update_current_day(bool active);
  void load_history();

private:
  void save_day(DailyStatsImpl *stats);
  void save_day(DailyStatsImpl *stats, std::ofstream &stats_file);
  void load(std::ifstream &infile, bool history);
  void day_to_history(DailyStatsImpl *stats);
  void add_history(DailyStatsImpl *stats);

private:
  //! Interface to the core_control.
  Core *core{nullptr};

  //! Mouse/Keyboard monitoring.
  workrave::input_monitor::IInputMonitor::Ptr input_monitor;

  //! Last time a mouse event was received.
  std::chrono::system_clock::time_point last_mouse_time;

  //! Statistics of current day.
  DailyStatsImpl *current_day{nullptr};

  //! Has the user been active on the current day?
  bool been_active{false};

  //! History
  History history;

  //! Internal locking
  std::mutex lock;

  //! Previous X coordinate
  int prev_x{-1};

  //! Previous Y coordinate
  int prev_y{-1};

  //! Previous X-click coordinate
  int click_x{-1};

  //! Previous Y-click coordinate
  int click_y{-1};
};

#endif // STATISTICS_HH
