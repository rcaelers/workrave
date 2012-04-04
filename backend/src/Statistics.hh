// Statistics.hh
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2010 Rob Caelers & Raymond Penners
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

#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <string.h>

#include "IStatistics.hh"
#include "IInputMonitorListener.hh"
#include "Mutex.hh"

// Forward declarion of external interface.
namespace workrave {
  class IBreak;
}

class TimePred;
class PacketBuffer;
class Core;
class IInputMonitor;

using namespace workrave;
using namespace std;

#ifdef HAVE_DISTRIBUTION
#include "IDistributionClientMessage.hh"
#include "PacketBuffer.hh"
#endif

class Statistics :
  public IStatistics,
  public IInputMonitorListener
#ifdef HAVE_DISTRIBUTION
  ,
  public IDistributionClientMessage
#endif
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
    GTimeVal total_mouse_time;

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

      total_mouse_time.tv_sec = 0;
      total_mouse_time.tv_usec = 0;
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
  Statistics();

  //! Destructor
  virtual ~Statistics();

  bool delete_all_history();

public:
  void init(Core *core);
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
  void update_current_day(bool active);
  void load_history();

private:
  void save_day(DailyStatsImpl *stats);
  void save_day(DailyStatsImpl *stats, std::ofstream &stats_file);
  void load(std::ifstream &infile, bool history);

  void day_to_history(DailyStatsImpl *stats);
  void day_to_remote_history(DailyStatsImpl *stats);

  void add_history(DailyStatsImpl *stats);

#ifdef HAVE_DISTRIBUTION
  void init_distribution_manager();
  bool request_client_message(DistributionClientMessageID id, PacketBuffer &buffer);
  bool client_message(DistributionClientMessageID id, bool master, const char *client_id,
                      PacketBuffer &buffer);
  bool pack_stats(PacketBuffer &buffer, const DailyStatsImpl *stats);
#endif

private:
  //! Interface to the core_control.
  Core *core;

  //! Mouse/Keyboard monitoring.
  IInputMonitor *input_monitor;

  //! Last time a mouse event was received.
  GTimeVal last_mouse_time;

  //! Statistics of current day.
  DailyStatsImpl *current_day;

  //! Has the user been active on the current day?
  bool been_active;

  //! History
  History history;

  //! Internal locking
  Mutex lock;

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
