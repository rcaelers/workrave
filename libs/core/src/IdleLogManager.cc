// IdleLogManager.cc
//
// Copyright (C) 2003, 2004, 2005, 2007, 2009, 2010, 2011 Rob Caelers <robc@krandor.org>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef PLATFORM_OS_MACOS
#  include "MacOSHelpers.hh"
#endif

#include "debug.hh"
#include <cassert>
#include <fstream>
#include <sstream>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include "utils/Util.hh"
#include "IdleLogManager.hh"
#include "utils/TimeSource.hh"
#include "PacketBuffer.hh"

#define IDLELOG_MAXSIZE (4000)
#define IDLELOG_MAXAGE (12 * 60 * 60)
#define IDLELOG_INTERVAL (30 * 60)
#define IDLELOG_VERSION (3)
#define IDLELOG_INTERVAL_SIZE (17)

using namespace workrave::utils;
using namespace std;

//! Constructs a new idlelog manager.
IdleLogManager::IdleLogManager(string myid)
{
  this->myid = myid;
  this->last_expiration_time = 0;
}

//! Update the idlelogs of all clients.
void
IdleLogManager::update_all_idlelogs(string master_id, ActivityState current_state)
{
  TRACE_ENTER_MSG("IdleLogManager::update_all_idlelogs", master_id << " " << current_state);

  if (current_state == ACTIVITY_NOISE)
    {
      // Not interested in noise.
      current_state = ACTIVITY_IDLE;
    }

  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = i->second;

      // Default: remote client is idle and not master
      ActivityState state = ACTIVITY_IDLE;
      bool master = false;

      // Only the master can be active.
      if (i->first == master_id)
        {
          // This client is master, sets its state.
          state = current_state;
          master = true;
        }

      // Update history.
      update_idlelog(info, state, master);

      // Remember current state/master status.
      info.master = master;
      info.state = state;
    }

  expire();

  TRACE_EXIT();
}

//! Resets the total active time of all clients.
void
IdleLogManager::reset()
{
  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = (*i).second;
      info.update_active_time(TimeSource::get_monotonic_time_sec() );
      info.total_active_time = 0;
    }
}

//! Initializes the idlelog manager.
void
IdleLogManager::init()
{
  TRACE_ENTER("IdleLogManager::init()");

  load();

  if (clients.find(myid) == clients.end())
    {
      TRACE_MSG("Didn't find myself");

      ClientInfo &myinfo = clients[myid];
      myinfo.current_interval = IdleInterval(1, TimeSource::get_monotonic_time_sec() );
      myinfo.client_id = myid;

      save();
    }
  else
    {
      ClientInfo &myinfo = clients[myid];
      myinfo.current_interval = IdleInterval(TimeSource::get_monotonic_time_sec() , TimeSource::get_monotonic_time_sec() );
    }

  TRACE_EXIT();
}

//! Terminates the idlelog manager.
void
IdleLogManager::terminate()
{
  save();
}

//! Expire entries that are too old.
void
IdleLogManager::expire()
{
  time_t current_time = TimeSource::get_monotonic_time_sec();
  if (last_expiration_time == 0)
    {
      last_expiration_time = current_time + IDLELOG_INTERVAL;
    }
  else if (current_time <= last_expiration_time)
    {
      last_expiration_time = current_time + IDLELOG_INTERVAL;
      for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
        {
          ClientInfo &info = (*i).second;
          expire(info);
        }
    }
}

//! Expire entries that are too old.
void
IdleLogManager::expire(ClientInfo &info)
{
  if (info.idlelog.size() > IDLELOG_MAXSIZE)
    {
      info.idlelog.resize(IDLELOG_MAXSIZE);
    }

  time_t current_time = TimeSource::get_monotonic_time_sec();
  int count = 0;
  for (IdleLogRIter i = info.idlelog.rbegin(); i != info.idlelog.rend(); i++)
    {
      IdleInterval &idle = info.idlelog.back();
      if (idle.end_idle_time < current_time - IDLELOG_MAXAGE)
        {
          count++;
        }
      else
        {
          break;
        }
    }

  if (count != 0)
    {
      if (info.idlelog.size() > (size_t)count)
        {
          info.idlelog.resize(info.idlelog.size() - count);
        }
      else
        {
          info.idlelog.clear();
        }
    }
}

//! Update the idle log of a single client.
void
IdleLogManager::update_idlelog(ClientInfo &info, ActivityState state, bool master)
{
  (void)master;
  TRACE_ENTER_MSG("IdleLogManager::update_idlelog", ((int)state) << " " << master);

  // Did the state/master status change?
  bool changed = state != info.state; // RC: removed... || master != info.master;

  time_t current_time = TimeSource::get_monotonic_time_sec();
  IdleInterval *idle = &(info.current_interval);
  idle->end_time = current_time;

  if (state == ACTIVITY_IDLE)
    {
      if (changed)
        {
          // State changed from active to idle:

          // update active time of last interval.
          info.update_active_time(current_time);

          // save front.
          if (info.idlelog.size() > 0)
            {
              IdleInterval *save_interval = &(info.idlelog.front());
              if (save_interval->to_be_saved)
                {
                  update_idlelog(info, *save_interval);
                  save_interval->to_be_saved = false;
                }
            }

          // Push current
          info.current_interval.to_be_saved = true;
          info.idlelog.push_front(info.current_interval);

          // create a new (empty) idle interval.
          info.current_interval = IdleInterval(current_time, current_time);
          idle = &(info.current_interval);
        }
      else
        {
          // State remained idle. Update end time of idle interval.
          idle->end_idle_time = current_time;

          if (info.idlelog.size() > 0)
            {
              time_t total_idle = idle->end_idle_time - idle->begin_time;
              if (total_idle >= 10)
                {
                  IdleInterval *save_interval = &(info.idlelog.front());
                  if (save_interval->to_be_saved)
                    {
                      TRACE_MSG("Saving");
                      update_idlelog(info, *save_interval);
                      save_interval->to_be_saved = false;
                    }
                }
            }
        }
    }
  else if (state == ACTIVITY_ACTIVE)
    {
      if (changed)
        {
          // State changed from idle to active:

          idle->end_idle_time = current_time;

          time_t total_idle = idle->end_idle_time - idle->begin_time;
          if (total_idle < 10 && info.idlelog.size() > 1)
            {
              // Idle period too short. remove it. and reuse previous
              IdleInterval &oldidle = info.idlelog.front();

              if (oldidle.to_be_saved)
                {
                  info.current_interval = oldidle;
                  info.idlelog.pop_front();
                  idle = &(info.current_interval);
                }
            }

          // Update start time of last active period.
          info.last_active_time = 0;
          info.last_active_begin_time = current_time;
        }
      else if (info.last_active_begin_time != 0)
        {
          // State remained active.
          info.last_active_time = current_time - info.last_active_begin_time;
        }
    }

  info.last_update_time = current_time;
  dump_idlelog(info);
  TRACE_EXIT();
}

//! Returns the total active time of all clients.
time_t
IdleLogManager::compute_total_active_time()
{
  TRACE_ENTER("IdleLogManager::compute_total_active_time");
  time_t current_time = TimeSource::get_monotonic_time_sec();
  time_t active_time = 0;
  for (ClientMapIter it = clients.begin(); it != clients.end(); it++)
    {
      ClientInfo &info = (*it).second;
      info.update_active_time(current_time);
      active_time += info.total_active_time;
    }
  TRACE_EXIT();
  return active_time;
}

//! Returns the active time since an idle period of a least the specified amount of time.
time_t
IdleLogManager::compute_active_time(int length)
{
  TRACE_ENTER("IdleLogManager::compute_active_time");

  time_t current_time = TimeSource::get_monotonic_time_sec();

  // Number of client.
  int size = clients.size();

  // Data for each client.
  IdleLogIter *iterators = new IdleLogIter[size];
  IdleLogIter *end_iterators = new IdleLogIter[size];
  bool *at_end = new bool[size];
  time_t *active_time = new time_t[size];

  // Init data for all clients.
  int count = 0;
  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = (*i).second;

      iterators[count] = info.idlelog.begin();
      end_iterators[count] = info.idlelog.end();
      active_time[count] = 0;
      at_end[count] = true;

      info.update_active_time(current_time);
      count++;
    }

  // Number of simultaneous idle periods.
  int idle_count = 0;

  // Time of last unprocessed event.
  time_t last_time = -1;

  // Iterator of last unprocessed event.
  int last_iter = -1;

  // Stop criterium
  bool stop = false;

  // Begin and End time of idle perdiod.
  time_t end_idle_time = -1;

  while (!stop)
    {
      // Find latest event.
      last_time = -1;
      for (int i = 0; i < size; i++)
        {
          if (iterators[i] != end_iterators[i])
            {
              IdleInterval &ii = *(iterators[i]);
              time_t t = at_end[i] ? ii.end_idle_time : ii.begin_time;

              if (last_time == -1 || t > last_time)
                {
                  last_time = t;
                  last_iter = i;
                }
            }
        }

      // Did we found one?
      if (last_time != -1)
        {
          IdleInterval &ii = *(iterators[last_iter]);
          if (at_end[last_iter])
            {
              TRACE_MSG("End time " << ii.end_idle_time << " active " << ii.active_time);
              idle_count++;

              at_end[last_iter] = false;
              active_time[last_iter] += ii.active_time;
              end_idle_time = ii.end_idle_time;
            }
          else
            {
              TRACE_MSG("Begin time " << ii.begin_time);

              at_end[last_iter] = true;
              iterators[last_iter]++;

              if (idle_count == size)
                {
                  TRACE_MSG("Common idle period of " << (end_idle_time - ii.begin_time));
                  if ((end_idle_time - ii.begin_time) > length)
                    {
                      stop = true;
                    }
                }

              idle_count--;
            }
        }
      else
        {
          stop = true;
        }
    }

  time_t total_active_time = 0;
  for (int i = 0; i < size; i++)
    {
      TRACE_MSG("active time of " << i << " = " << active_time[i]);
      total_active_time += active_time[i];
    }

  TRACE_MSG("total = " << total_active_time);

  delete[] iterators;
  delete[] end_iterators;
  delete[] at_end;
  delete[] active_time;

  TRACE_EXIT();
  return total_active_time;
}

//! Computes the current idle time.
time_t
IdleLogManager::compute_idle_time()
{
  TRACE_ENTER("IdleLogManager::compute_idle_time");

  time_t current_time = TimeSource::get_monotonic_time_sec();

  int count = 0;
  time_t latest_start_time = 0;

  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = (*i).second;
      info.update_active_time(current_time);

      IdleInterval &idle = info.idlelog.front();
      if (idle.active_time == 0)
        {
          count++;
        }
      if (idle.begin_time > latest_start_time)
        {
          latest_start_time = idle.begin_time;
          TRACE_MSG(current_time - latest_start_time);
        }
    }

  TRACE_MSG("count = " << count);
  if ((unsigned int)count != clients.size() + 1)
    {
      latest_start_time = current_time;
    }

  TRACE_MSG((current_time - latest_start_time));
  TRACE_EXIT();

  return current_time - latest_start_time;
}

//! Packs the idle interval to the buffer.
void
IdleLogManager::pack_idle_interval(PacketBuffer &buffer, const IdleInterval &idle) const
{
  int pos = 0;

  buffer.reserve_size(pos);
  buffer.pack_byte(IDLELOG_VERSION);
  buffer.pack_ulong((guint32)idle.begin_time);
  buffer.pack_ulong((guint32)idle.end_idle_time);
  buffer.pack_ulong((guint32)idle.end_time);
  buffer.pack_ushort((guint32)idle.active_time);
  buffer.update_size(pos);
}

//! Unpacks the idle interval from the buffer.
void
IdleLogManager::unpack_idle_interval(PacketBuffer &buffer, IdleInterval &idle, time_t delta_time) const
{
  int pos = 0;
  int size = buffer.read_size(pos);

  if (size > 0 && buffer.bytes_available() >= size)
    {
      /*int version = */ buffer.unpack_byte();

      idle.begin_time = buffer.unpack_ulong() - delta_time;
      idle.end_idle_time = buffer.unpack_ulong() - delta_time;
      idle.end_time = buffer.unpack_ulong() - delta_time;
      idle.active_time = buffer.unpack_ushort();

      buffer.skip_size(pos);
    }
  else
    {
      buffer.clear();
    }
}

//! Packs the idlelog header to the buffer.
void
IdleLogManager::pack_idlelog(PacketBuffer &buffer, const ClientInfo &ci) const
{
  time_t current_time = TimeSource::get_monotonic_time_sec();

  // Add size.
  int pos = 0;
  buffer.reserve_size(pos);

  // Pack
  buffer.pack_ulong((guint32)current_time);
  buffer.pack_string(ci.client_id.c_str());
  buffer.pack_ulong((guint32)ci.total_active_time);
  buffer.pack_byte(ci.master);
  buffer.pack_byte(ci.state);
  buffer.pack_ushort(ci.idlelog.size());

  buffer.update_size(pos);
}

//! Unpacks the idlelog header from the buffer.
void
IdleLogManager::unpack_idlelog(PacketBuffer &buffer, ClientInfo &ci, time_t &pack_time, int &num_intervals) const
{
  int pos = 0;
  int size = buffer.read_size(pos);

  if (size > 0 && buffer.bytes_available() >= size)
    {
      pack_time = buffer.unpack_ulong();

      char *id = buffer.unpack_string();

      if (id != nullptr)
        {
          ci.client_id = id;
        }

      ci.total_active_time = buffer.unpack_ulong();
      ci.master = buffer.unpack_byte() != 0;
      ci.state = (ActivityState)buffer.unpack_byte();

      num_intervals = buffer.unpack_ushort();

      g_free(id);

      buffer.skip_size(pos);
    }
  else
    {
      buffer.clear();
    }
}

//! Removes the idlelog specified in the buffer.
void
IdleLogManager::unlink_idlelog(PacketBuffer &buffer) const
{
  TRACE_ENTER("IdleLogManager::unlink_idlelog()");

  int pos = 0;
  int size = buffer.read_size(pos);

  if (size > 0 && buffer.bytes_available() >= size)
    {
      buffer.unpack_ulong(); // skip pack time.
      char *id = buffer.unpack_string();

      if (id != nullptr)
        {
          stringstream ss;
          ss << Util::get_home_directory();
          ss << "idlelog." << id << ".log" << ends;

#ifdef PLATFORM_OS_WINDOWS
          _unlink(ss.str().c_str());
#else
          unlink(ss.str().c_str());
#endif

          g_free(id);
        }

      buffer.skip_size(pos);
    }
  else
    {
      buffer.clear();
    }
  TRACE_EXIT();
}

//! Saves the idlelog index.
void
IdleLogManager::save_index()
{
  TRACE_ENTER("IdleLogManager::save_index()");

  PacketBuffer buffer;
  buffer.create();

  buffer.pack_ushort(IDLELOG_VERSION);
  buffer.pack_string(myid.c_str());

  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = (*i).second;
      info.update_active_time(TimeSource::get_monotonic_time_sec() );
      TRACE_MSG("Saving " << i->first << " " << info.client_id);

      pack_idlelog(buffer, info);
    }

  stringstream ss;
  ss << Util::get_home_directory();
  ss << "idlelog.idx" << ends;

  ofstream file(ss.str().c_str(), ios::binary);
  file.write(buffer.get_buffer(), buffer.bytes_written());
  file.close();

  TRACE_EXIT();
}

//! Loads the idlelog index.
void
IdleLogManager::load_index()
{
  TRACE_ENTER("IdleLogManager::load()");

  stringstream ss;
  ss << Util::get_home_directory();
  ss << "idlelog.idx" << ends;

  bool exists = Util::file_exists(ss.str());

  if (exists)
    {
      TRACE_MSG("File exists - ok");

      // Open file
      ifstream file(ss.str().c_str(), ios::binary);

      // get file size using buffer's members
      filebuf *pbuf = file.rdbuf();
      int size = pbuf->pubseekoff(0, ios::end, ios::in);
      pbuf->pubseekpos(0, ios::in);

      TRACE_MSG("Size - " << size);

      PacketBuffer buffer;
      buffer.create(size);

      file.read(buffer.get_buffer(), size);
      file.close();
      buffer.write_ptr += size;

      // Read
      int version = buffer.unpack_ushort();
      TRACE_MSG("Version - " << version);

      if (version == IDLELOG_VERSION)
        {
          TRACE_MSG("Version - ok");

          char *id = buffer.unpack_string();
          if (id != nullptr)
            {
              TRACE_MSG("id = " << id);
            }

          while (buffer.bytes_available() > 0)
            {
              ClientInfo info;

              int num_intervals;
              time_t pack_time;

              unpack_idlelog(buffer, info, pack_time, num_intervals);
              info.master = false;
              info.state = ACTIVITY_IDLE;
              info.last_update_time = pack_time;
              TRACE_MSG("Add client " << info.client_id);

              clients[info.client_id] = info;
            }

          g_free(id);
        }
      else
        {
          TRACE_MSG("Old version - deleting logs of old version");

          char *id = buffer.unpack_string();
          if (id != nullptr)
            {
              TRACE_MSG("id = " << id);
            }

          while (buffer.bytes_available() > 0)
            {
              unlink_idlelog(buffer);
            }

          g_free(id);
        }
    }
  TRACE_EXIT();
}

//! Saves the idlelog for the specified client.
void
IdleLogManager::save_idlelog(ClientInfo &info)
{
  info.update_active_time(TimeSource::get_monotonic_time_sec() );

  PacketBuffer buffer;
  buffer.create();

  for (IdleLogRIter i = info.idlelog.rbegin(); i != info.idlelog.rend(); i++)
    {
      IdleInterval &idle = *i;

      pack_idle_interval(buffer, idle);
    }

  stringstream ss;
  ss << Util::get_home_directory();
  ss << "idlelog." << info.client_id << ".log" << ends;

  ofstream file(ss.str().c_str(), ios::binary);
  file.write(buffer.get_buffer(), buffer.bytes_written());
  file.close();
}

//! Loads the idlelog for the specified client.
void
IdleLogManager::load_idlelog(ClientInfo &info)
{
  TRACE_ENTER("IdleLogManager::load_idlelog()");

  time_t current_time = TimeSource::get_monotonic_time_sec();

  stringstream ss;
  ss << Util::get_home_directory();
  ss << "idlelog." << info.client_id << ".log" << ends;

  // Open file
  ifstream file(ss.str().c_str(), ios::binary);

  // get file size using buffer's members
  filebuf *pbuf = file.rdbuf();
  int size = pbuf->pubseekoff(0, ios::end, ios::in);
  pbuf->pubseekpos(0, ios::in);

  // Process it.
  int num_intervals = size / IDLELOG_INTERVAL_SIZE;
  if (num_intervals * IDLELOG_INTERVAL_SIZE == size)
    {
      if (num_intervals > IDLELOG_MAXSIZE)
        {
          TRACE_MSG("Skipping " << (num_intervals - IDLELOG_MAXSIZE) << " intervals");
          int skip = (num_intervals - IDLELOG_MAXSIZE) * IDLELOG_INTERVAL_SIZE;
          file.seekg(skip);
          size -= skip;
          num_intervals = IDLELOG_MAXSIZE;
        }

      // Create buffer and load data.
      PacketBuffer buffer;
      buffer.create(size);
      file.read(buffer.get_buffer(), size);
      file.close();
      buffer.write_ptr += size;

      TRACE_MSG("loading " << num_intervals << " intervals");
      for (int i = 0; i < num_intervals; i++)
        {
          IdleInterval idle;
          unpack_idle_interval(buffer, idle, 0);

          if (idle.end_idle_time >= current_time - IDLELOG_MAXAGE)
            {
              info.idlelog.push_front(idle);
            }
        }

      if (info.idlelog.size() > 0)
        {
          IdleInterval &idle = info.idlelog.back();
          idle.begin_time = 1;
        }
    }

  dump_idlelog(info);
  fix_idlelog(info);
  dump_idlelog(info);
  TRACE_EXIT();
}

//! Loads the entire idlelog.
void
IdleLogManager::load()
{
  load_index();

  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = (*i).second;
      load_idlelog(info);
    }
}

//! Saves the entire idlelog.
void
IdleLogManager::save()
{
  save_index();

  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = (*i).second;
      save_idlelog(info);
    }
}

//! Adds the specified idle interval to persistent storage.
void
IdleLogManager::update_idlelog(ClientInfo &info, const IdleInterval &idle)
{
  info.update_active_time(TimeSource::get_monotonic_time_sec() );

  PacketBuffer buffer;
  buffer.create();

  pack_idle_interval(buffer, idle);

  stringstream ss;
  ss << Util::get_home_directory();
  ss << "idlelog." << info.client_id << ".log" << ends;

  ofstream file(ss.str().c_str(), ios::app | ios::binary);
  file.write(buffer.get_buffer(), buffer.bytes_written());
  file.close();

  save_index();
}

void
IdleLogManager::get_idlelog(PacketBuffer &buffer)
{
  TRACE_ENTER("IdleLogManager::get_idlelog");

  // Information about me.
  ClientInfo &myinfo = clients[myid];

  // First make sure that all data is up-to-date.
  myinfo.update_active_time(TimeSource::get_monotonic_time_sec() );

  // Pack header.
  pack_idlelog(buffer, myinfo);

  for (IdleLogIter i = myinfo.idlelog.begin(); i != myinfo.idlelog.end(); i++)
    {
      pack_idle_interval(buffer, *i);
    }

  TRACE_EXIT();
}

void
IdleLogManager::set_idlelog(PacketBuffer &buffer)
{
  TRACE_ENTER("IdleLogManager::set_idlelog");

  time_t delta_time = 0;
  time_t pack_time = 0;
  int num_intervals = 0;

  ClientInfo info;
  unpack_idlelog(buffer, info, pack_time, num_intervals);

  delta_time = pack_time - TimeSource::get_monotonic_time_sec();
  clients[info.client_id] = info;
  info.last_update_time = 0;

  for (int i = 0; i < num_intervals; i++)
    {
      IdleInterval idle;
      unpack_idle_interval(buffer, idle, delta_time);

      TRACE_MSG(info.client_id << " " << idle.begin_time << " " << idle.end_idle_time << " " << idle.active_time);
      clients[info.client_id].idlelog.push_back(idle);
    }

  fix_idlelog(info);
  save_index();
  save_idlelog(clients[info.client_id]);

  TRACE_EXIT();
}

//! A remote client has signed on.
void
IdleLogManager::signon_remote_client(string client_id)
{
  TRACE_ENTER_MSG("signon_remote_client", client_id);

  time_t current_time = TimeSource::get_monotonic_time_sec();

  ClientInfo &info = clients[client_id];
  info.idlelog.push_front(IdleInterval(1, current_time));
  info.client_id = client_id;

  save_index();
  save_idlelog(info);

  TRACE_EXIT();
}

//! A remote client has signed off.
void
IdleLogManager::signoff_remote_client(string client_id)
{
  TRACE_ENTER_MSG("signoff_remote_client", client_id);

  clients[client_id].state = ACTIVITY_IDLE;
  clients[client_id].master = false;

  TRACE_EXIT();
}

//! Dumps the idle log of the specified client.
void
IdleLogManager::dump_idlelog(ClientInfo &info)
{
  (void)info;
#if 0
#  ifndef PLATFORM_OS_WINDOWS
  TRACE_ENTER("IdleLogManager::dump_idlelog");

  TRACE_MSG("id = " << info.client_id);
  TRACE_MSG("last_active_time = " << info.last_active_time);
  TRACE_MSG("total_active_time = " << info.total_active_time);

  {
      IdleInterval &idle = info.current_interval;
      struct tm begin_time;
      localtime_r(&idle.begin_time, &begin_time);
      struct tm end_idle_time;
      localtime_r(&idle.end_idle_time, &end_idle_time);
      struct tm end_time;
      localtime_r(&idle.end_time, &end_time);

      TRACE_MSG(   begin_time.tm_hour << ":"
                   << begin_time.tm_min << ":"
                   << begin_time.tm_sec << " - "
                   << end_idle_time.tm_hour << ":"
                   << end_idle_time.tm_min << ":"
                   << end_idle_time.tm_sec << " "
                   << idle.active_time << " "
                   << end_time.tm_hour << ":"
                   << end_time.tm_min << ":"
                   << end_time.tm_sec
                   );
  }

  IdleLogIter i = info.idlelog.begin();
  while (i != info.idlelog.end())
    {
      IdleInterval &idle = *i;

      struct tm begin_time;
      localtime_r(&idle.begin_time, &begin_time);
      struct tm end_idle_time;
      localtime_r(&idle.end_idle_time, &end_idle_time);
      struct tm end_time;
      localtime_r(&idle.end_time, &end_time);

      TRACE_MSG(   begin_time.tm_hour << ":"
                   << begin_time.tm_min << ":"
                   << begin_time.tm_sec << " - "
                   << end_idle_time.tm_hour << ":"
                   << end_idle_time.tm_min << ":"
                   << end_idle_time.tm_sec << " "
                   << idle.active_time << " "
                   << end_time.tm_hour << ":"
                   << end_time.tm_min << ":"
                   << end_time.tm_sec
                   );
      i++;
    }
  TRACE_EXIT();
#  endif
#endif
}

void
IdleLogManager::fix_idlelog(ClientInfo &info)
{
  TRACE_ENTER("IdleLogManager::fix_idlelog");

  time_t current_time = TimeSource::get_monotonic_time_sec();
  info.update_active_time(current_time);

  time_t next_time = -1;

  for (IdleLogRIter i = info.idlelog.rbegin(); i != info.idlelog.rend(); i++)
    {
      IdleInterval &idle = *i;

      TRACE_MSG(idle.begin_time << " " << idle.end_time << " " << idle.end_idle_time << " " << idle.active_time);

      if (next_time == -1)
        {
          if (idle.begin_time != 1)
            {
              TRACE_MSG("Fixing first start time. setting to 1");
              idle.begin_time = 1;
            }
        }
      else
        {
          if (idle.begin_time < next_time)
            {
              TRACE_MSG("Fixing start time. setting from " << idle.begin_time << " to " << next_time);
              idle.begin_time = next_time;
            }
        }

      if (idle.end_time != 0)
        {
          next_time = idle.end_time;
        }
      else
        {
          next_time = idle.end_idle_time;
        }
    }

  info.idlelog.push_front(IdleInterval(next_time, current_time));

  TRACE_EXIT();
}
