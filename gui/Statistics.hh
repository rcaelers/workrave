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

#include "GUIControl.hh"
#include "ActivityMonitorInterface.hh"

class Statistics
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
  
  void start_new_day();
  void load_current_day();
  void save_current_day();
  void update_current_day();
  void load_history();

  void increment_break_counter(GUIControl::BreakId, StatsBreakValueType st);
  void set_counter(StatsValueType t, int value);
  int get_counter(StatsValueType t);
                 
  void set_total_active(int active);

  DailyStats *get_current_day() const;
  DailyStats *get_day(int day) const;
  int get_history_size() const;
  
  void dump();

private:
  void load_day(DailyStats *stats, ifstream &stats_file);
  void save_current_day(ofstream &statsFile);
  void current_to_history();
  
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
