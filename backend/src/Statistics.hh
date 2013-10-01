// Statistics.hh
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2012, 2013 Rob Caelers & Raymond Penners
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

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <chrono>

#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <string.h>

#include "input-monitor/IInputMonitor.hh"
#include "input-monitor/IInputMonitorListener.hh"

#include "IStatistics.hh"
#include "ActivityMonitor.hh"

using namespace workrave;
using namespace workrave::input_monitor;
using namespace std;

class Statistics
  : public IStatistics,
    public IInputMonitorListener
{
public:
  typedef boost::shared_ptr<Statistics> Ptr;

public:
  static Ptr create(ActivityMonitor::Ptr monitor);

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

      for(int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          for(int j = 0; j < STATS_BREAKVALUE_SIZEOF; j++)
            {
              break_stats[i][j] = 0;
            }
        }

      for(int j = 0; j < STATS_VALUE_SIZEOF; j++)
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
  //! Constructor.
  Statistics(ActivityMonitor::Ptr monitor);

  //! Destructor
  virtual ~Statistics();

  bool delete_all_history();

public:
  void init();
  void update();
  void dump();
  void start_new_day();

  void increment_break_counter(BreakId, StatsBreakValueType st);
  void set_break_counter(BreakId bt, StatsBreakValueType st, int value);
  void add_break_counter(BreakId bt, StatsBreakValueType st, int value);
  
  DailyStatsImpl *get_current_day() const;
  DailyStatsImpl *get_day(int day) const;
  void get_day_index_by_date(int y, int m, int d, int &idx, int &next, int &prev) const;

  int get_history_size() const;
  void set_counter(StatsValueType t, int value);
  int64_t get_counter(StatsValueType t);

private:
  void action_notify();
  void mouse_notify(int x, int y, int wheel = 0);
  void button_notify(bool is_press);
  void keyboard_notify(bool repeat);

  bool load_current_day();
  void load_history();

private:
  void save_day(DailyStatsImpl *stats);
  void save_day(DailyStatsImpl *stats, std::ofstream &stats_file);
  void load(std::ifstream &infile, bool history);

  void day_to_history(DailyStatsImpl *stats);
  void day_to_remote_history(DailyStatsImpl *stats);

  void add_history(DailyStatsImpl *stats);

private:
  ActivityMonitor::Ptr monitor;
  
  //! Mouse/Keyboard monitoring.
  IInputMonitor::Ptr input_monitor;

  //! Last time a mouse event was received.
  std::chrono::system_clock::time_point last_mouse_time;

  //! Statistics of current day.
  DailyStatsImpl *current_day;

  //! Has the user been active on the current day?
  bool been_active;

  //! History
  History history;

  //! Internal locking
  boost::mutex lock;

  //! Previous X coordinate
  int prev_x;

  //! Previous Y coordinate
  int prev_y;

  //! Previous X-click coordinate
  int click_x;

  //! Previous Y-click coordinate
  int click_y;
};

#endif // STATISTICS_HH
