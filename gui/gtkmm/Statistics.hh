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

class Statistics
{
public:
  enum StatType
    {
      STAT_TYPE_PROMPTED = 0,
      STAT_TYPE_TAKEN,
      STAT_TYPE_NATURAL_TAKEN,
      STAT_TYPE_SKIPPED,
      STAT_TYPE_POSTPONED,
      STAT_TYPE_UNIQUE_BREAKS,
      STAT_TYPE_SIZEOF
    };

  typedef int BreakStats[STAT_TYPE_SIZEOF];
  
  struct DailyStats
  {
    DailyStats()
    {
      for(int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
        {
          for(int j = 0; j < STAT_TYPE_SIZEOF; j++)
            {
              break_stats[i][j] = 0;
            }
        }
      total_active = 0;
    }
        
    struct tm start, stop;
    BreakStats break_stats[GUIControl::BREAK_ID_SIZEOF];
    int total_active;
  };
  
  //! Constructor.
  Statistics();

  //! Destructor
  virtual ~Statistics();

public:
  static Statistics *get_instance();

  void start_new_day();
  void load_current_day();
  void save_current_day();
  void load_history();

  void increment_counter(GUIControl::BreakId, StatType st);
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
