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

#include "Statistics.hh"

#include "ControlInterface.hh"
#include "Util.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#endif

Statistics *Statistics::instance = NULL;
const char *WORKRAVESTATS="WorkRaveStats";
const int STATSVERSION = 4;

//! Constructor
Statistics::Statistics() : 
  core_control(NULL),
  current_day(NULL)
{
  start_new_day();

#ifdef HAVE_DISTRIBUTION
  init_distribution_manager();
#endif
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

      stats_file << "B " << i << " " << STATS_BREAKVALUE_SIZEOF << " ";
      for(int j = 0; j < STATS_BREAKVALUE_SIZEOF; j++)
        {
          stats_file << bs[j] << " ";
        }
      stats_file << endl;
    }

  stats_file << "M " << STATS_VALUE_SIZEOF << " ";
  for(int j = 0; j < STATS_VALUE_SIZEOF; j++)
    {
      stats_file << current_day->misc_stats[j] << " ";
    }
  stats_file << endl;
  
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
      else if (cmd == "M")
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

              stats->misc_stats[j] = value;
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

      ok = (cmd == "D");

      if (ok)
        {
          DailyStats *day = new DailyStats();
          
          load_day(day, stats_file);
        }
    }
}


//! Increment the specified statistics counter of the current day.
void
Statistics::increment_break_counter(GUIControl::BreakId bt, StatsBreakValueType st)
{
  if (current_day == NULL)
    {
      start_new_day();
    }

  BreakStats &bs = current_day->break_stats[bt];
  bs[st]++;
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


//! Sets the total active time of the current day.
void
Statistics::set_total_active(int active)
{
  current_day->misc_stats[STATS_VALUE_TOTAL_ACTIVE_TIME] = active;
}


//! Dump
void
Statistics::dump()
{
  TRACE_ENTER("Statistics::dump");

  update_current_day();
  
  stringstream ss;
  for(int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
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

  TRACE_MSG(ss.str());
  TRACE_EXIT();
}


Statistics::DailyStats *
Statistics::get_current_day() const
{
  //FIXME:
  return NULL;
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
      // Collect total active time from dialy limit timer.
      TimerInterface *t = core_control->get_timer("daily_limit");
      assert(t != NULL);
      set_total_active(t->get_elapsed_time());

      // Collect activity monitor stats.
      ActivityMonitorInterface *monitor = core_control->get_activity_monitor();
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
  if (core_control != NULL)
    {
      // Collect activity monitor stats.
      ActivityMonitorInterface *monitor = core_control->get_activity_monitor();
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
  DistributionManager *dist_manager = DistributionManager::get_instance();

  if (dist_manager != NULL)
    {
      dist_manager->register_state(DISTR_STATE_STATS,  this);
    }
}

bool
Statistics::get_state(DistributedStateID id, unsigned char **buffer, int *size)
{
  TRACE_ENTER("Statistics::get_state");

  update_current_day();
  dump();
  
  PacketBuffer state_packet;
  state_packet.create();

  state_packet.pack_byte(STATS_MARKER_TODAY);
  pack_stats(state_packet, current_day);
  state_packet.pack_byte(STATS_MARKER_END);

  *size = state_packet.bytes_written();
  *buffer = new unsigned char[*size + 1];
  memcpy(*buffer, state_packet.get_buffer(), *size);

  return true;
}


bool
Statistics::pack_stats(PacketBuffer &buf, DailyStats *stats)
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
  
  for(int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
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
Statistics::set_state(DistributedStateID id, bool master, unsigned char *buffer, int size)
{
  TRACE_ENTER("Statistics::set_state");

  PacketBuffer state_packet;
  state_packet.create();

  state_packet.pack_raw(buffer, size);

  DailyStats *stats = NULL;
  int pos = 0;
  
  StatsMarker marker = (StatsMarker) state_packet.unpack_byte();
  while (marker != STATS_MARKER_END && state_packet.bytes_available() > 0)
    {
      TRACE_MSG("Marker = " << marker);
      switch (marker)
        {
        case STATS_MARKER_TODAY:
          stats = current_day;
          break;
          
        case STATS_MARKER_HISTORY:
          assert(1==0);
          break;
          
        case STATS_MARKER_STARTTIME:
          {
            int size = state_packet.unpack_ushort();
            
            stats->start.tm_mday = state_packet.unpack_byte();
            stats->start.tm_mon = state_packet.unpack_byte();
            stats->start.tm_year = state_packet.unpack_ushort();
            stats->start.tm_hour = state_packet.unpack_byte();
            stats->start.tm_min = state_packet.unpack_byte();
          }
          break;
          
        case STATS_MARKER_STOPTIME:
          {
            int size = state_packet.unpack_ushort();
            
            stats->stop.tm_mday = state_packet.unpack_byte();
            stats->stop.tm_mon = state_packet.unpack_byte();
            stats->stop.tm_year = state_packet.unpack_ushort();
            stats->stop.tm_hour = state_packet.unpack_byte();
            stats->stop.tm_min = state_packet.unpack_byte();
          }
          break;

        case STATS_MARKER_BREAK_STATS:
          {
            int size = state_packet.read_size(pos);
            int bt = state_packet.unpack_byte();

            BreakStats &bs = stats->break_stats[bt];

            int count = state_packet.unpack_ushort();

            if (count > STATS_BREAKVALUE_SIZEOF)
              {
                count = STATS_BREAKVALUE_SIZEOF;
              }
            
            for(int j = 0; j < count; j++)
              {
                bs[j] = state_packet.unpack_ulong();
              }

            state_packet.skip_size(pos);
          }
          break;
          
        case STATS_MARKER_MISC_STATS:
          {
            int size = state_packet.read_size(pos);
            int count = state_packet.unpack_ushort();

            if (count > STATS_VALUE_SIZEOF)
              {
                count = STATS_VALUE_SIZEOF;
              }
            
            for(int j = 0; j < count; j++)
              {
                stats->misc_stats[j] = state_packet.unpack_ulong();
              }

            state_packet.skip_size(pos);
          }
          break;
          
        case STATS_MARKER_END:
          break;

        default:
          {
            TRACE_MSG("Unknown marker");
            int size = state_packet.read_size(pos);
            state_packet.skip_size(pos);
          }
        }
    
      marker = (StatsMarker) state_packet.unpack_byte();
    }

  update_enviromnent();
  dump();

  TRACE_EXIT();
  return true;
}


#endif
