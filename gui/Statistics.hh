// Statistics.hh
//
// Copyright (C) 2002 Rob Caelers <robc@krandor.org>
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
#include <string>
#include <vector>
#include <map>
#include <list>
#include <time.h>

class BreakInterface;
class TimerInterface;

#ifdef HAVE_DISTRIBUTION
#include "DistributedStateInterface.hh"
#include "PacketBuffer.hh"
#endif

#include "GUIControl.hh"
#include "ActivityMonitorInterface.hh"

class Statistics :
#ifdef HAVE_DISTRIBUTION
  public DistributedStateInterface
#endif  
{
public:
  enum StatsBreakValueType
    {
      STATS_BREAKVALUE_PROMPTED = 0,
      STATS_BREAKVALUE_TAKEN,
      STATS_BREAKVALUE_NATURAL_TAKEN,
      STATS_BREAKVALUE_SKIPPED,
      STATS_BREAKVALUE_POSTPONED,
      STATS_BREAKVALUE_UNIQUE_BREAKS,
      STATS_BREAKVALUE_SIZEOF
    };

  enum StatsValueType
    {
      STATS_VALUE_TOTAL_ACTIVE_TIME = 0,
      STATS_VALUE_TOTAL_MOUSE_MOVEMENT,
      STATS_VALUE_TOTAL_CLICK_MOVEMENT,
      STATS_VALUE_TOTAL_MOVEMENT_TIME,
      STATS_VALUE_TOTAL_CLICKS,
      STATS_VALUE_TOTAL_KEYSTROKES,
      STATS_VALUE_SIZEOF
    };

#ifdef HAVE_DISTRIBUTION
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
#endif
  
  typedef int BreakStats[STATS_BREAKVALUE_SIZEOF];
  typedef int MiscStats[STATS_VALUE_SIZEOF];
  
  struct DailyStats
  {
    DailyStats()
    {
      for(int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
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
    }

    //! Start time of this day.
    struct tm start;

    //! Stop time of this day.
    struct tm stop;

    //! Statistic of each break
    BreakStats break_stats[GUIControl::BREAK_ID_SIZEOF];

    //! Misc statistics
    MiscStats misc_stats;
  };
  
  //! Constructor.
  Statistics();

  //! Destructor
  virtual ~Statistics();

public:
  static Statistics *get_instance();
  void init(ControlInterface *control);
  void heartbeat();
  void dump();
  void start_new_day();

  void increment_break_counter(GUIControl::BreakId, StatsBreakValueType st);

  DailyStats *get_current_day() const;
  DailyStats *get_day(int day) const;
  int get_history_size() const;
  void set_counter(StatsValueType t, int value);
  int get_counter(StatsValueType t);
  
private:  
  void load_current_day();
  void update_current_day();
  void load_history();

private:
  void load_day(DailyStats *stats, ifstream &stats_file);
  void save_day(DailyStats *stats);
  void save_day(DailyStats *stats, ofstream &stats_file);

  void day_to_history(DailyStats *stats);
  void day_to_remote_history(DailyStats *stats);
  void update_enviromnent();

#ifdef HAVE_DISTRIBUTION
  void init_distribution_manager();
  bool get_state(DistributedStateID id, unsigned char **buffer, int *size);
  bool set_state(DistributedStateID id, bool master, unsigned char *buffer, int size);
  bool pack_stats(PacketBuffer &buffer, DailyStats *stats);
#endif
  
private:
  //! The one and only instance
  static Statistics *instance;

  //! Interface to the core_control.
  ControlInterface *core_control;
  
  //! Statistics of current day.
  DailyStats *current_day;

  //! History
  vector<DailyStats *> history;
};


inline Statistics *
Statistics::get_instance()
{
  if (instance == NULL)
    {
      instance = new Statistics();
    }
       
  return instance;
}

#endif // STATISTICS_HH
