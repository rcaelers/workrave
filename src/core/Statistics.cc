// Statistics.cc
//
// Copyright (C) 2002, 2003 Rob Caelers & Raymond Penners
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
#include <assert.h>

#include "debug.hh"

#include "Statistics.hh"

#include "Core.hh"
#include "Util.hh"
#include "Timer.hh"
#include "TimePred.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#endif

const char *WORKRAVESTATS="WorkRaveStats";
const int STATSVERSION = 4;


//! Constructor
Statistics::Statistics() : 
  core(NULL),
  current_day(NULL),
  reset_predicate(NULL)
{
}


//! Destructor
Statistics::~Statistics()
{
  update();
  
  for (vector<DailyStatsImpl *>::iterator i = history.begin(); i != history.end(); i++)
    {
      delete *i;
    }

  delete current_day;
  delete reset_predicate;
}


//! Initializes the Statistics.
void
Statistics::init(Core *control)
{
  core = control;

  start_new_day();
  
#ifdef HAVE_DISTRIBUTION
  init_distribution_manager();
#endif

  load_current_day();
  update_enviromnent();
  load_history();
}


//! Sets the stats reset predicate
void
Statistics::set_reset_predicate(TimePred *predicate)
{
  TRACE_ENTER("Statistics::set_reset_predicate");
  delete reset_predicate;
  reset_predicate = predicate;

  if (predicate != NULL && current_day != NULL)
    {
      time_t t = mktime(&(current_day->start));
      predicate->set_last(t);
    }
  TRACE_EXIT();
}


//! Periodic heartbeat.
bool
Statistics::update()
{
  bool new_day_started = false;
  
  TRACE_ENTER("Statistics::update");
  TRACE_MSG(time(NULL) << " " << reset_predicate->get_next() << " " <<
            (reset_predicate->get_next() - time(NULL)));
  if (reset_predicate != NULL && time(NULL) > reset_predicate->get_next())
    {
      TRACE_MSG("NEW DAY");
      new_day_started = true;
    }
      
  update_current_day();
  save_day(current_day);
  TRACE_EXIT();
  return new_day_started;
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
      tmnow->tm_year !=  current_day->start.tm_year ||
      tmnow->tm_hour !=  current_day->start.tm_hour ||
      tmnow->tm_min  !=  current_day->start.tm_min)
    {

      TRACE_MSG("New day");
      if (current_day != NULL)
        {
          TRACE_MSG("Save old day");
          day_to_history(current_day);
          day_to_remote_history(current_day);
        }

      current_day = new DailyStatsImpl();

      current_day->start = *tmnow;
      current_day->stop = *tmnow;

      if (reset_predicate != NULL)
        {
          time_t t = mktime(&(current_day->start));
          reset_predicate->set_last(t);
        }
    }

  update_current_day();
  save_day(current_day);
  
  TRACE_EXIT();
}


void
Statistics::day_to_history(DailyStatsImpl *stats)
{
  history.push_back(stats);
  
  stringstream ss;
  ss << Util::get_home_directory();
  ss << "historystats" << ends;

  bool exists = Util::file_exists(ss.str());
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
#ifdef HAVE_DISTRIBUTION
  DistributionManager *dist_manager = core->get_distribution_manager();

  if (dist_manager != NULL)
    {
      PacketBuffer state_packet;
      state_packet.create();

      state_packet.pack_byte(STATS_MARKER_HISTORY);
      pack_stats(state_packet, stats);
      state_packet.pack_byte(STATS_MARKER_END);

      dist_manager->broadcast_client_message(DCM_STATS, state_packet);
    }
#endif
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
      for(int j = 0; j < STATS_BREAKVALUE_SIZEOF; j++)
        {
          stats_file << bs[j] << " ";
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
  ss << Util::get_home_directory();
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
      HistoryRIter i = history.rbegin();
      while (i != history.rend())
        {
          DailyStatsImpl *ref = *i;

          if (stats->start.tm_year == ref->start.tm_year  &&
              stats->start.tm_mon == ref->start.tm_mon &&
              stats->start.tm_mday == ref->start.tm_mday)
            {
              *i = stats;
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
            break;
          }
          i++;
        }
    }
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

      ok = (version == STATSVERSION) || (version == 3);
    }

  if (ok)
    { 
      string cmd;
      stats_file >> cmd;

      ok = (cmd == "D");
    }

  if (ok)
    {
      load_day(current_day, stats_file);
    }
  
  TRACE_EXIT();
}


//! Loads the statistics of a single day from the specified stream.
void
Statistics::load_day(DailyStatsImpl *stats, ifstream &stats_file)
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
      stats_file >> cmd;

      if (cmd == "B")
        {
          int bt, size;
          stats_file >> bt;
          stats_file >> size;
          
          BreakStats &bs = stats->break_stats[bt];

          if (size > STATS_BREAKVALUE_SIZEOF)
            {
              size = STATS_BREAKVALUE_SIZEOF;
            }
          
          for(int j = 0; j < size; j++)
            {
              int value;
              stats_file >> value;

              bs[j] = value;
            }
        }
      else if (cmd == "M" || cmd == "m")
        {
          int size;
          stats_file >> size;
          
          if (size > STATS_VALUE_SIZEOF)
            {
             size = STATS_VALUE_SIZEOF;
            }
          
          for(int j = 0; j < size; j++)
            {
              int value;
              stats_file >> value;

              if (cmd == "m")
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
      else if (cmd == "G")
        {
          int total_active;
          stats_file >> total_active;

          stats->misc_stats[STATS_VALUE_TOTAL_ACTIVE_TIME] = total_active;
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
  TRACE_ENTER("Statistics::load_history");
  
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

      ok = (version == STATSVERSION) || (version == 3);
    }


  bool first = true;
  while (ok && !stats_file.eof())
    {
      string cmd;

      if (first)
        {
          stats_file >> cmd;
          
          ok = cmd == "D";
        }

      if (ok)
        {
          DailyStatsImpl *day = new DailyStatsImpl(); // FIXME: leak
          
          load_day(day, stats_file);

          add_history(day);
          first = false;
        }
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


int
Statistics::get_counter(StatsValueType t)
{
  return current_day->misc_stats[t];
}


//! Dump
void
Statistics::dump()
{
  TRACE_ENTER("Statistics::dump");

  update_current_day();
  
  stringstream ss;
  for(int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakStats &bs = current_day->break_stats[i];

      ss << "Break " << i << " ";
      for(int j = 0; j < STATS_BREAKVALUE_SIZEOF; j++)
        {
          int value = bs[j];

          ss << value << " ";
        }
    }

  ss << "stats ";
  for(int j = 0; j < STATS_VALUE_SIZEOF; j++)
    {
      int value = current_day->misc_stats[j];
      
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



void
Statistics::update_current_day()
{
  if (core != NULL)
    {
      // Collect total active time from dialy limit timer.
      Timer *t = core->get_break(BREAK_ID_DAILY_LIMIT)->get_timer();
      assert(t != NULL);
      current_day->misc_stats[STATS_VALUE_TOTAL_ACTIVE_TIME] = t->get_elapsed_time();

      const time_t now = time(NULL);
      struct tm *tmnow = localtime(&now);
      current_day->stop = *tmnow;
      
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          Timer *t = core->get_break(BreakId(i))->get_timer();
          assert(t != NULL);
      
          int overdue = t->get_total_overdue_time();

          set_break_counter(((BreakId)i),
                            Statistics::STATS_BREAKVALUE_TOTAL_OVERDUE, overdue);
        }

      
      // Collect activity monitor stats.
      ActivityMonitorInterface *monitor = core->get_activity_monitor();
      assert(monitor != NULL);

      ActivityMonitorStatistics ams;
      monitor->get_statistics(ams);

      
      current_day->misc_stats[STATS_VALUE_TOTAL_MOUSE_MOVEMENT] = ams.total_movement;
      current_day->misc_stats[STATS_VALUE_TOTAL_CLICK_MOVEMENT] = ams.total_click_movement;
      current_day->misc_stats[STATS_VALUE_TOTAL_MOVEMENT_TIME] = ams.total_movement_time;
      current_day->misc_stats[STATS_VALUE_TOTAL_CLICKS] = ams.total_clicks;
      current_day->misc_stats[STATS_VALUE_TOTAL_KEYSTROKES] = ams.total_keystrokes;
    }
}


void
Statistics::update_enviromnent()
{
  if (core != NULL)
    {
      // Collect activity monitor stats.
      ActivityMonitorInterface *monitor = core->get_activity_monitor();
      assert(monitor != NULL);

      ActivityMonitorStatistics ams;
      
      ams.total_movement = current_day->misc_stats[STATS_VALUE_TOTAL_MOUSE_MOVEMENT];
      ams.total_click_movement = current_day->misc_stats[STATS_VALUE_TOTAL_CLICK_MOVEMENT];
      ams.total_movement_time = current_day->misc_stats[STATS_VALUE_TOTAL_MOVEMENT_TIME];
      ams.total_clicks = current_day->misc_stats[STATS_VALUE_TOTAL_CLICKS];
      ams.total_keystrokes = current_day->misc_stats[STATS_VALUE_TOTAL_KEYSTROKES];

      monitor->set_statistics(ams);
    }
}



#ifdef HAVE_DISTRIBUTION
// Create the monitor based on the specified configuration.
void
Statistics::init_distribution_manager()
{
  DistributionManager *dist_manager = core->get_distribution_manager();

  if (dist_manager != NULL)
    {
      dist_manager->register_client_message(DCM_STATS, DCMT_MASTER, this);
    }
}

bool
Statistics::request_client_message(DistributionClientMessageID id, PacketBuffer &buffer)
{
  TRACE_ENTER("Statistics::request_client_message");
  (void) id;
  
  update_current_day();
  dump();
  
  buffer.pack_byte(STATS_MARKER_TODAY);
  pack_stats(buffer, current_day);
  buffer.pack_byte(STATS_MARKER_END);

  TRACE_EXIT();
  return true;
}


bool
Statistics::pack_stats(PacketBuffer &buf, const DailyStatsImpl *stats)
{
  TRACE_ENTER("Statistics::pack_stats");

  int pos = 0;

  buf.pack_byte(STATS_MARKER_STARTTIME);
  buf.reserve_size(pos);
  buf.pack_byte(stats->start.tm_mday);
  buf.pack_byte(stats->start.tm_mon);
  buf.pack_ushort(stats->start.tm_year);
  buf.pack_byte(stats->start.tm_hour);
  buf.pack_byte(stats->start.tm_min);
  buf.update_size(pos);

  buf.pack_byte(STATS_MARKER_STOPTIME);
  buf.reserve_size(pos);
  buf.pack_byte(stats->stop.tm_mday);
  buf.pack_byte(stats->stop.tm_mon);
  buf.pack_ushort(stats->stop.tm_year);
  buf.pack_byte(stats->stop.tm_hour);
  buf.pack_byte(stats->stop.tm_min);
  buf.update_size(pos);
  
  for(int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakStats &bs = current_day->break_stats[i];

      buf.pack_byte(STATS_MARKER_BREAK_STATS);
      buf.reserve_size(pos);
      buf.pack_byte(i);
      buf.pack_ushort(STATS_BREAKVALUE_SIZEOF);
      
      for(int j = 0; j < STATS_BREAKVALUE_SIZEOF; j++)
        {
          buf.pack_ulong(bs[j]);
        }
      buf.update_size(pos);
    }

  buf.pack_byte(STATS_MARKER_MISC_STATS);
  buf.reserve_size(pos);
  buf.pack_ushort(STATS_VALUE_SIZEOF);
  
  for(int j = 0; j < STATS_VALUE_SIZEOF; j++)
    {
      buf.pack_ulong(current_day->misc_stats[j]);
    }
  buf.update_size(pos);
  
  TRACE_EXIT();
  return true;
}

bool
Statistics::client_message(DistributionClientMessageID id, bool master, const char *client_id,
                           PacketBuffer &buffer)
{
  TRACE_ENTER("Statistics::client_message");

  (void) id;
  (void) master;
  (void) client_id;

  return false;
  
  DailyStatsImpl *stats = NULL;
  int pos = 0;
  bool stats_to_history = false;
  
  while (buffer.bytes_available() > 0)
    {
      StatsMarker marker = (StatsMarker) buffer.unpack_byte();
      TRACE_MSG("Marker = " << marker);
      switch (marker)
        {
        case STATS_MARKER_TODAY:
          stats = current_day;
          break;
          
        case STATS_MARKER_HISTORY:
          stats = new DailyStatsImpl();
          stats_to_history = true;
          break;
          
        case STATS_MARKER_STARTTIME:
          {
            int size = buffer.unpack_ushort();
            (void) size;
            
            stats->start.tm_mday = buffer.unpack_byte();
            stats->start.tm_mon = buffer.unpack_byte();
            stats->start.tm_year = buffer.unpack_ushort();
            stats->start.tm_hour = buffer.unpack_byte();
            stats->start.tm_min = buffer.unpack_byte();
          }
          break;
          
        case STATS_MARKER_STOPTIME:
          {
            int size = buffer.unpack_ushort();
            (void) size;
            
            stats->stop.tm_mday = buffer.unpack_byte();
            stats->stop.tm_mon = buffer.unpack_byte();
            stats->stop.tm_year = buffer.unpack_ushort();
            stats->stop.tm_hour = buffer.unpack_byte();
            stats->stop.tm_min = buffer.unpack_byte();
          }
          break;

        case STATS_MARKER_BREAK_STATS:
          {
            int size = buffer.read_size(pos);
            int bt = buffer.unpack_byte();
            (void) size;

            BreakStats &bs = stats->break_stats[bt];

            int count = buffer.unpack_ushort();

            if (count > STATS_BREAKVALUE_SIZEOF)
              {
                count = STATS_BREAKVALUE_SIZEOF;
              }
            
            for(int j = 0; j < count; j++)
              {
                bs[j] = buffer.unpack_ulong();
              }

            buffer.skip_size(pos);
          }
          break;
          
        case STATS_MARKER_MISC_STATS:
          {
            int size = buffer.read_size(pos);
            int count = buffer.unpack_ushort();
            (void) size;

            if (count > STATS_VALUE_SIZEOF)
              {
                count = STATS_VALUE_SIZEOF;
              }
            
            for(int j = 0; j < count; j++)
              {
                stats->misc_stats[j] = buffer.unpack_ulong();
              }

            buffer.skip_size(pos);
          }
          break;
          
        case STATS_MARKER_END:
          if (stats_to_history)
            {
              TRACE_MSG("Save to history");
              day_to_history(stats);
              stats_to_history = false;
            }
          break;

        default:
          {
            TRACE_MSG("Unknown marker");
            int size = buffer.read_size(pos);
            (void) size;
            buffer.skip_size(pos);
          }
        }
    }

  if (stats_to_history)
    {
      // this should not happend. but just to avoid a potential memory leak...
      TRACE_MSG("Save to history");
      day_to_history(stats);
      stats_to_history = false;
    }
  
  update_enviromnent();
  dump();

  TRACE_EXIT();
  return true;
}

#endif

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
