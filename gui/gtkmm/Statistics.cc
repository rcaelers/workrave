// Statistics.cc
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sstream>

#include "debug.hh"

#include "ControlInterface.hh"
#include "Util.hh"
#include "Statistics.hh"

Statistics *Statistics::instance = NULL;
const char *WORKRAVESTATS="WorkRaveStats";
const int STATSVERSION = 3;

//! Constructor
Statistics::Statistics() : 
  current_day(NULL),
  core_control(NULL)
{
  start_new_day();
}


//! Destructor
Statistics::~Statistics()
{
  if (current_day != NULL)
    {
      delete current_day;
    }
  
  for (vector<DailyStats *>::iterator i = history.begin(); i != history.end(); i++)
    {
      delete *i;
    }
}


void
Statistics::init(ControlInterface *control)
{
  core_control = control;
}


//! Starts a new day and archive current any (if exists)
void
Statistics::start_new_day()
{
  const time_t now = time(NULL);
  struct tm *tmnow = localtime(&now);

  if (current_day == NULL ||
      tmnow->tm_mday !=  current_day->start.tm_mday ||
      tmnow->tm_mon  !=  current_day->start.tm_mon  ||
      tmnow->tm_year !=  current_day->start.tm_year ||
      tmnow->tm_hour !=  current_day->start.tm_hour ||
      tmnow->tm_min  !=  current_day->start.tm_min)
    {
      
      if (current_day != NULL)
        {
          current_to_history();
        }

      current_day = new DailyStats();

      current_day->start = *tmnow;
      current_day->stop = *tmnow;
    }
}


//! Adds the current day to this history.
void
Statistics::current_to_history()
{
  history.push_back(current_day);
  
  stringstream ss;
  ss << Util::get_home_directory();
  ss << "historystats" << ends;

  const time_t now = time(NULL);
  struct tm *tmnow = localtime(&now);
  current_day->stop = *tmnow;

  bool exists = Util::file_exists(ss.str());

  ofstream stats_file(ss.str().c_str(), ios::app);

  if (!exists)
    {
      stats_file << WORKRAVESTATS << " " << STATSVERSION  << endl;
    }

  save_current_day(stats_file);
  stats_file.close();
}


//! Saves the current day to the specified stream.
void
Statistics::save_current_day(ofstream &stats_file)
{
  stats_file << "D "
             << current_day->start.tm_mday << " "
             << current_day->start.tm_mon << " "
             << current_day->start.tm_year << " " 
             << current_day->start.tm_hour << " "
             << current_day->start.tm_min << " "
             << current_day->stop.tm_mday << " "
             << current_day->stop.tm_mon << " "
             << current_day->stop.tm_year << " " 
             << current_day->stop.tm_hour << " "
             << current_day->stop.tm_min <<  endl;
    
  for(int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      BreakStats &bs = current_day->break_stats[i];

      stats_file << "B " << i << " " << STAT_TYPE_SIZEOF << " ";
      for(int j = 0; j < STAT_TYPE_SIZEOF; j++)
        {
          stats_file << bs[j] << " ";
        }
      stats_file << endl;
    }
  stats_file << "G "
             << current_day->total_active
             << endl;

  stats_file.close();
}


//! Saves the statistics of the current day.
void
Statistics::save_current_day()
{
  stringstream ss;
  ss << Util::get_home_directory();
  ss << "todaystats" << ends;

  const time_t now = time(NULL);
  struct tm *tmnow = localtime(&now);
  current_day->stop = *tmnow;
  
  ofstream stats_file(ss.str().c_str());

  stats_file << WORKRAVESTATS << " " << STATSVERSION  << endl;

  save_current_day(stats_file);
}


//! Load the statistics of the current day.
void
Statistics::load_current_day()
{
  TRACE_ENTER("Statistics::load_current_day");
  stringstream ss;
  ss << Util::get_home_directory();
  ss << "todaystats" << ends;

  ifstream stats_file(ss.str().c_str());

  bool ok = stats_file;

  if (ok)
    {
      string tag;
      stats_file >> tag;

      ok = (tag == WORKRAVESTATS);
    }

  if (ok)
    {
      int version;
      stats_file >> version;

      ok = (version == STATSVERSION);
    }

  if (ok)
    {
      string cmd;
      stats_file >> cmd;

      ok == (cmd == "D");
    }

  if (ok)
    {
      load_day(current_day, stats_file);
    }
  
  TRACE_EXIT();
}


//! Loads the statistics of a single day from the specified stream.
void
Statistics::load_day(DailyStats *stats, ifstream &stats_file)
{
  bool ok = stats_file;

  if (ok)
    {
      stats_file >> stats->start.tm_mday 
                 >> stats->start.tm_mon
                 >> stats->start.tm_year
                 >> stats->start.tm_hour
                 >> stats->start.tm_min
                 >> stats->stop.tm_mday 
                 >> stats->stop.tm_mon
                 >> stats->stop.tm_year
                 >> stats->stop.tm_hour
                 >> stats->stop.tm_min;
        }
  
  while (ok && !stats_file.eof())
    {
      string cmd;
      GUIControl::BreakId bt;
      stats_file >> cmd;

      if (cmd == "B")
        {
          int bt, size;
          stats_file >> bt;
          stats_file >> size;
          
          BreakStats &bs = stats->break_stats[bt];

          if (size > STAT_TYPE_SIZEOF)
            {
              size = STAT_TYPE_SIZEOF;
            }
          
          for(int j = 0; j < size; j++)
            {
              int value;
              stats_file >> value;

              bs[j] = value;
            }
        }
      else if (cmd == "G")
        {
          stats_file >> stats->total_active;
        }
      else if (cmd == "D")
        {
          ok = false;
        }
    }
}


//! Loads the history.
void
Statistics::load_history()
{
  stringstream ss;
  ss << Util::get_home_directory();
  ss << "historystats" << ends;

  ifstream stats_file(ss.str().c_str());

  bool ok = stats_file;

  if (ok)
    {
      string tag;
      stats_file >> tag;

      ok = (tag == WORKRAVESTATS);
    }

  if (ok)
    {
      int version;
      stats_file >> version;

      ok = (version == STATSVERSION);
    }


  while (ok && !stats_file.eof())
    {
      string cmd;
      stats_file >> cmd;

      ok == (cmd == "D");

      if (ok)
        {
          DailyStats *day = new DailyStats();
          
          load_day(day, stats_file);
        }
    }
}


//! Increment the specified statistics counter of the current day.
void
Statistics::increment_counter(GUIControl::BreakId bt, StatType st)
{
  if (current_day == NULL)
    {
      start_new_day();
    }

  BreakStats &bs = current_day->break_stats[bt];
  bs[st]++;
}


//! Sets the total active time of the current day.
void
Statistics::set_total_active(int active)
{
  current_day->total_active = active;
}


//! Dump
void
Statistics::dump()
{
  TRACE_ENTER("Statistics::dump");

  for(int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      BreakStats &bs = current_day->break_stats[i];

      TRACE_MSG("Break " << i);
      for(int j = 0; j < STAT_TYPE_SIZEOF; j++)
        {
          int value = bs[j];

          TRACE_MSG("STAT " << value);
        }
    }
  TRACE_EXIT();
}


Statistics::DailyStats *
Statistics::get_current_day() const
{
}


Statistics::DailyStats *
Statistics::get_day(int day) const
{
  DailyStats *ret = NULL;
  
  if (day == 0)
    {
      ret = current_day;
    }
  else
    {
      if (day < 0)
        {
          day = history.size() - day;
        }
      else
        {
          day--;
        }

      if (day <= history.size() && day >= 0)
        {
          ret = history[day];
        }
    }

  return ret;
}


int
Statistics::get_history_size() const
{
  return history.size();
}



void
Statistics::update_current_day()
{
  if (core_control != NULL)
    {
      TimerInterface *t = core_control->get_timer("daily_limit");
      assert(t != NULL);

      set_total_active(t->get_elapsed_time());

      ActivityMonitorInterface *monitor = core_control->get_activity_monitor();
      assert(monitor != NULL);

      monitor->get_statistics(current_day->activity_stats);
    }
}


