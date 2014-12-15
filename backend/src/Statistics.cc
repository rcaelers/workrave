// Statistics.cc
//
// Copyright (C) 2002 - 2008, 2010, 2012, 2013 Rob Caelers & Raymond Penners
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
#include "config.h"
#endif

#include "Statistics.hh"

#include <boost/filesystem.hpp>

#include <cstring>
#include <sstream>
#include <assert.h>
#include <math.h>

#include "debug.hh"

#include "utils/AssetPath.hh"
#include "Timer.hh"
#include "input-monitor/InputMonitorFactory.hh"
#include "input-monitor/IInputMonitor.hh"

static const char *WORKRAVESTATS="WorkRaveStats";
static const int STATSVERSION = 4;

#define MAX_JUMP (10000)

using namespace workrave::utils;

Statistics::Ptr
Statistics::create(IActivityMonitor::Ptr monitor)
{
  return Ptr(new Statistics(monitor));
}

//! Constructor
Statistics::Statistics(IActivityMonitor::Ptr monitor) :
  monitor(monitor),
  current_day(NULL),
  been_active(false),
  prev_x(-1),
  prev_y(-1),
  click_x(-1),
  click_y(-1)
{
}


//! Destructor
Statistics::~Statistics()
{
  update();

  for (auto &item : history)
    {
      delete item;
    }

  delete current_day;

  if (input_monitor != NULL)
    {
      input_monitor->unsubscribe(this);
    }
}


//! Initializes the Statistics.
void
Statistics::init()
{
  input_monitor = InputMonitorFactory::create_monitor(IInputMonitorFactory::CAPABILITY_STATISTICS);
  if (input_monitor != NULL)
    {
      input_monitor->subscribe(this);
    }

  current_day = NULL;
  bool ok = load_current_day();
  if (!ok)
    {
      start_new_day();
    }

  load_history();
}


//! Periodic heartbeat.
void
Statistics::update()
{
  TRACE_ENTER("Statistics::update");

  if (monitor->is_active())
    {
      const time_t now = time(NULL);
      struct tm *tmnow = localtime(&now);
      current_day->stop = *tmnow;
      
      if (!been_active)
        {
          current_day->start = *tmnow;
          been_active = true;
        }
    }
  save_day(current_day);
  TRACE_EXIT();
}


bool 
Statistics::delete_all_history()
{
    update();

    string histfile = AssetPath::get_home_directory() + "historystats";
    boost::filesystem::path histpath(histfile);
    
    if( boost::filesystem::is_regular_file(histpath) && std::remove( histfile.c_str() ) )
    {
        return false;
    }
    else
    {
        for( vector<DailyStatsImpl *>::iterator i = history.begin(); ( i != history.end() ); delete *i++ )
            ;

        history.clear();
    }

    string todayfile = AssetPath::get_home_directory() + "todaystats";
    boost::filesystem::path todaypath(todayfile);

    if( boost::filesystem::is_regular_file(todaypath) && std::remove( todayfile.c_str() ) )
    {
        return false;
    }
    else
    {
        if( current_day )
        {
            delete current_day;
            current_day = NULL;
        }
        start_new_day();
   }

    return true;
}


//! Starts a new day and archive current any (if exists)
void
Statistics::start_new_day()
{
  TRACE_ENTER("Statistics::start_new_day");
  const time_t now = time(NULL);
  struct tm *tmnow = localtime(&now);

  if (current_day == NULL ||
      tmnow->tm_mday !=  current_day->start.tm_mday ||
      tmnow->tm_mon  !=  current_day->start.tm_mon  ||
      tmnow->tm_year !=  current_day->start.tm_year
      )
    {
      TRACE_MSG("New day");
      if (current_day != NULL)
        {
          TRACE_MSG("Save old day");
          day_to_history(current_day);
          day_to_remote_history(current_day);
        }

      current_day = new DailyStatsImpl();
      been_active = false;

      current_day->start = *tmnow;
      current_day->stop = *tmnow;
    }

  update();
  save_day(current_day);

  TRACE_EXIT();
}


void
Statistics::day_to_history(DailyStatsImpl *stats)
{
  add_history(stats);

  stringstream ss;
  ss << AssetPath::get_home_directory();
  ss << "historystats" << ends;

  boost::filesystem::path path(ss.str());
  bool exists = boost::filesystem::is_regular_file(path);
  
  ofstream stats_file(ss.str().c_str(), ios::app);

  if (!exists)
    {
      stats_file << WORKRAVESTATS << " " << STATSVERSION  << endl;
    }

  save_day(stats, stats_file);
  stats_file.close();

}


//! Adds the current day to this history.
void
Statistics::day_to_remote_history(DailyStatsImpl *stats)
{
  (void) stats;
}


//! Saves the current day to the specified stream.
void
Statistics::save_day(DailyStatsImpl *stats, ofstream &stats_file)
{
  stats_file << "D "
             << stats->start.tm_mday << " "
             << stats->start.tm_mon << " "
             << stats->start.tm_year << " "
             << stats->start.tm_hour << " "
             << stats->start.tm_min << " "
             << stats->stop.tm_mday << " "
             << stats->stop.tm_mon << " "
             << stats->stop.tm_year << " "
             << stats->stop.tm_hour << " "
             << stats->stop.tm_min <<  endl;

  for(int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakStats &bs = stats->break_stats[i];

      stats_file << "B " << i << " " << STATS_BREAKVALUE_SIZEOF << " ";
      for(auto &b : bs)
        {
          stats_file << b << " ";
        }
      stats_file << endl;
    }

  stats_file << "m " << STATS_VALUE_SIZEOF << " ";
  for(int j = 0; j < STATS_VALUE_SIZEOF; j++)
    {
      stats_file << stats->misc_stats[j] << " ";
    }
  stats_file << endl;

  stats_file.close();
}


//! Saves the statistics of the specified day.
void
Statistics::save_day(DailyStatsImpl *stats)
{
  stringstream ss;
  ss << AssetPath::get_home_directory();
  ss << "todaystats" << ends;

  ofstream stats_file(ss.str().c_str());

  stats_file << WORKRAVESTATS << " " << STATSVERSION  << endl;

  save_day(stats, stats_file);
}


//! Add the stats the the history list.
void
Statistics::add_history(DailyStatsImpl *stats)
{
  if (history.size() == 0)
    {
      history.push_back(stats);
    }
  else
    {
      bool found = false;
      HistoryRIter i = history.rbegin();
      while (i != history.rend())
        {
          DailyStatsImpl *ref = *i;

          if (stats->start.tm_year == ref->start.tm_year  &&
              stats->start.tm_mon == ref->start.tm_mon &&
              stats->start.tm_mday == ref->start.tm_mday)
            {
              delete *i;
              *i = stats;
              found = true;
              break;
            }

          else if ( stats->start.tm_year > ref->start.tm_year
                    || (stats->start.tm_year == ref->start.tm_year
                        && (stats->start.tm_mon > ref->start.tm_mon
                            || (stats->start.tm_mon == ref->start.tm_mon
                                && stats->start.tm_mday > ref->start.tm_mday))))
          {
            if (i == history.rbegin())
              {
                history.push_back(stats);
              }
            else
              {
                history.insert(i.base(), stats);
              }
            found = true;
            break;
          }
          i++;
        }

      if (!found)
        {
          history.insert(history.begin(), stats);
        }
    }
}

//! Load the statistics of the current day.
bool
Statistics::load_current_day()
{
  TRACE_ENTER("Statistics::load_current_day");
  stringstream ss;
  ss << AssetPath::get_home_directory();
  ss << "todaystats" << ends;

  ifstream stats_file(ss.str().c_str());

  load(stats_file, false);

  been_active = true;

  TRACE_EXIT();
  return current_day != NULL;
}


//! Loads the history.
void
Statistics::load_history()
{
  TRACE_ENTER("Statistics::load_history");

  stringstream ss;
  ss << AssetPath::get_home_directory();
  ss << "historystats" << ends;

  ifstream stats_file(ss.str().c_str());

  load(stats_file, true);
  TRACE_EXIT();
}


//! Loads the statistics.
void
Statistics::load(ifstream &infile, bool history)
{
  TRACE_ENTER("Statistics::load");

  DailyStatsImpl *stats = NULL;

  bool ok = infile.good();

  if (ok)
    {
      string tag;
      infile >> tag;

      ok = (tag == WORKRAVESTATS);
    }

  if (ok)
    {
      int version;
      infile >> version;

      ok = (version == STATSVERSION) || (version == 3);
    }


  while (ok && !infile.eof())
    {
      char line[BUFSIZ] = "";
      char cmd;

      infile.getline(line, BUFSIZ);

      if (strlen(line) > 1)
        {
          cmd = line[0];

          stringstream ss(line+1);

          if (cmd == 'D')
            {
              if (history && stats != NULL)
                {
                  add_history(stats);
                  stats = NULL;
                }
              else if (!history && stats != NULL)
                {
                  /* Corrupt today stats */
                  return;
                }

              stats = new DailyStatsImpl();

              ss >> stats->start.tm_mday
                 >> stats->start.tm_mon
                 >> stats->start.tm_year
                 >> stats->start.tm_hour
                 >> stats->start.tm_min
                 >> stats->stop.tm_mday
                 >> stats->stop.tm_mon
                 >> stats->stop.tm_year
                 >> stats->stop.tm_hour
                 >> stats->stop.tm_min;

              if (!history)
                {
                  current_day = stats;
                }
            }
          else if (stats != NULL)
            {
              if (cmd == 'B')
                {
                  int bt, size;
                  ss >> bt;
                  ss >> size;

                  BreakStats &bs = stats->break_stats[bt];

                  if (size > STATS_BREAKVALUE_SIZEOF)
                    {
                      size = STATS_BREAKVALUE_SIZEOF;
                    }

                  for(int j = 0; j < size; j++)
                    {
                      int value;
                      ss >> value;

                      bs[j] = value;
                    }
                }
              else if (cmd == 'M' || cmd == 'm')
                {
                  int size;
                  ss >> size;

                  if (size > STATS_VALUE_SIZEOF)
                    {
                      size = STATS_VALUE_SIZEOF;
                    }

                  for(int j = 0; j < size; j++)
                    {
                      int value;
                      ss >> value;

                      if (cmd == 'm')
                        {
                          // Ignore older 'M' stats. they are broken....
                          stats->misc_stats[j] = value;
                        }
                      else
                        {
                          stats->misc_stats[j] = 0;
                        }
                    }
                }
              else if (cmd == 'G')
                {
                  int total_active;
                  ss >> total_active;

                  stats->misc_stats[STATS_VALUE_TOTAL_ACTIVE_TIME] = total_active;
                }
            }
        }
    }

  if (history && stats != NULL)
    {
      add_history(stats);
    }

  TRACE_EXIT();
}


//! Increment the specified statistics counter of the current day.
void
Statistics::increment_break_counter(BreakId bt, StatsBreakValueType st)
{
  if (current_day == NULL)
    {
      start_new_day();
    }

  BreakStats &bs = current_day->break_stats[bt];
  bs[st]++;
}

void
Statistics::set_break_counter(BreakId bt, StatsBreakValueType st, int value)
{
  if (current_day == NULL)
    {
      start_new_day();
    }

  BreakStats &bs = current_day->break_stats[bt];
  bs[st] = value;
}


void
Statistics::add_break_counter(BreakId bt, StatsBreakValueType st, int value)
{
  if (current_day == NULL)
    {
      start_new_day();
    }

  BreakStats &bs = current_day->break_stats[bt];
  bs[st] += value;
}

void
Statistics::set_counter(StatsValueType t, int value)
{
  current_day->misc_stats[t] = value;
}


int64_t
Statistics::get_counter(StatsValueType t)
{
  return current_day->misc_stats[t];
}


//! Dump
void
Statistics::dump()
{
  TRACE_ENTER("Statistics::dump");

  update();

  stringstream ss;
  for(int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakStats &bs = current_day->break_stats[i];

      ss << "Break " << i << " ";
      for(auto value : bs)
        {
          ss << value << " ";
        }
    }

  ss << "stats ";
  for(int j = 0; j < STATS_VALUE_SIZEOF; j++)
    {
      int64_t value = current_day->misc_stats[j];

      ss  << value << " ";
    }

  TRACE_EXIT();
}


Statistics::DailyStatsImpl *
Statistics::get_current_day() const
{
  return current_day;
}


Statistics::DailyStatsImpl *
Statistics::get_day(int day) const
{
  DailyStatsImpl *ret = NULL;

  if (day == 0)
    {
      ret = current_day;
    }
  else
    {
      if (day > 0)
        {
          day = history.size() - day;
        }
      else
        {
          day = -day;
          day--;
        }

      if (day < int(history.size()) && day >= 0)
        {
          ret = history[day];
        }
    }

  return ret;
}

void
Statistics::get_day_index_by_date(int y, int m, int d,
                                  int &idx, int &next, int &prev) const
{
  TRACE_ENTER_MSG("Statistics::get_day_by_date", y << "/" << m << "/" << d);
  idx = next = prev = -1;
  for (int i = 0; i <= int(history.size()); i++)
    {
      int j = history.size() - i;
      DailyStatsImpl *stats = j == 0 ? current_day : history[i];
      if (idx < 0 && stats->starts_at_date(y, m, d))
        {
          idx = j;
        }
      else if (stats->starts_before_date(y, m, d))
        {
          prev = j;
        }
      else if (next < 0)
        {
          next = j;
        }
    }

  if (prev < 0 && current_day->starts_before_date(y, m, d))
    {
      prev = 0;
    }
  if (next < 0 && !current_day->starts_at_date(y, m, d)
      && !current_day->starts_before_date(y, m, d))
    {
      next = 0;
    }
  TRACE_EXIT();
}


int
Statistics::get_history_size() const
{
  return history.size();
}



bool
Statistics::DailyStatsImpl::starts_at_date(int y, int m, int d)
{
  return (start.tm_year + 1900 == y
          && start.tm_mon + 1 == m
          && start.tm_mday == d);
}

bool
Statistics::DailyStatsImpl::starts_before_date(int y, int m, int d)
{
  return (start.tm_year + 1900 < y
          || (start.tm_year + 1900 == y
              && (start.tm_mon + 1 < m
                  || (start.tm_mon + 1 == m
                      && start.tm_mday < d))));
}


//! Activity is reported by the input monitor.
void
Statistics::action_notify()
{
}


//! Mouse activity is reported by the input monitor.
void
Statistics::mouse_notify(int x, int y, int wheel_delta)
{
  static const int sensitivity = 3;

  lock.lock();

  if (current_day != NULL && x >=0 && y >= 0)
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
      if ( delta_x < MAX_JUMP && delta_y < MAX_JUMP &&
          (delta_x >= sensitivity || delta_y >= sensitivity || wheel_delta != 0 ))
        {
          int64_t movement = current_day->misc_stats[STATS_VALUE_TOTAL_MOUSE_MOVEMENT];
          int distance = int(sqrt((double)(delta_x * delta_x + delta_y * delta_y)));

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

              current_day->misc_stats[STATS_VALUE_TOTAL_MOVEMENT_TIME] =
                std::chrono::duration_cast<std::chrono::seconds>(current_day->total_mouse_time.time_since_epoch()).count();
            }

          last_mouse_time = now;
        }
    }

  lock.unlock();
}


//! Mouse button activity is reported by the input monitor.
void
Statistics::button_notify(bool is_press)
{
  lock.lock();
  if (current_day != NULL)
    {
      if (click_x != -1 && click_y != -1 &&
          prev_x != -1  && prev_y != -1)
        {
          int delta_x = click_x - prev_x;
          int delta_y = click_y - prev_y;

          int64_t movement = current_day->misc_stats[STATS_VALUE_TOTAL_CLICK_MOVEMENT];
          int64_t distance = int(sqrt((double)(delta_x * delta_x + delta_y * delta_y)));

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
void
Statistics::keyboard_notify(bool repeat)
{
  if (repeat)
    return;

  lock.lock();
  if (current_day != NULL)
    {
      current_day->misc_stats[STATS_VALUE_TOTAL_KEYSTROKES]++;
    }
  lock.unlock();
}
