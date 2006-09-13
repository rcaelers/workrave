// Statistics.hh
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef STATISTICS_HH
#define STATISTICS_HH

#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>

class TimePred;
class BreakInterface;
class TimerInterface;
class PacketBuffer;
class Core;

using namespace std;

#include "StatisticsInterface.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionClientMessageInterface.hh"
#include "PacketBuffer.hh"
#endif

#include "StatisticsInterface.hh"

class Statistics :
  public StatisticsInterface
#ifdef HAVE_DISTRIBUTION
  ,
  public DistributionClientMessageInterface
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

  typedef vector<DailyStatsImpl *> History;
  typedef vector<DailyStatsImpl *>::iterator HistoryIter;
  typedef vector<DailyStatsImpl *>::reverse_iterator HistoryRIter;

public:
  //! Constructor.
  Statistics();

  //! Destructor
  virtual ~Statistics();

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
  int get_counter(StatsValueType t);
  
private:  
  bool load_current_day();
  void update_current_day(bool active);
  void load_history();

private:
  void load_day(DailyStatsImpl *stats, ifstream &stats_file);
  void save_day(DailyStatsImpl *stats);
  void save_day(DailyStatsImpl *stats, ofstream &stats_file);

  void day_to_history(DailyStatsImpl *stats);
  void day_to_remote_history(DailyStatsImpl *stats);
  void update_enviromnent();

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
  
  //! Statistics of current day.
  DailyStatsImpl *current_day;

  //! Has the user been active on the current day?
  bool been_active;
  
  //! History
  History history;
};

#endif // STATISTICS_HH
