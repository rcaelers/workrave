// IdleLogManager.cc
//
// Copyright (C) 2003 Rob Caelers <robc@krandor.org>
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

#include "nls.h"

#include "debug.hh"
#include <assert.h>

#include "IdleLogManager.hh"
#include "TimeSource.hh"
#include "PacketBuffer.hh"


//! Constructs a new idlelog manager.
IdleLogManager::IdleLogManager(string myid, TimeSource *time_source)
{
  this->myid = myid;
  this->time_source = time_source;

  ClientInfo &myinfo = clients[myid];
  myinfo.idlelog.push_front(IdleInterval(1, time_source->get_time()));
}


//! Update the idlelogs of all clients.
void
IdleLogManager::update_all_idlelogs(string master_id, ActivityState current_state)
{
  TRACE_ENTER_MSG("IdleLogManager::update_all_idlelogs", master_id << " " << current_state);

  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = i->second;

      // Default: remote client is idle and not master
      ActivityState state = ACTIVITY_IDLE;
      bool master = false;

      // Only the master can be active.
      if (i->first == master_id)
        {
          // Correction, remote client IS master.
          state = current_state;
          master = true;
        }

      // Did the state/master status change?
      bool changed = (state != info.state || master != info.master);

      // Update history.
      update_idlelog(info, state, changed);

      // Remember current state/master status.
      info.master = master;
      info.state = state;
    }

  TRACE_EXIT();
}


//! Resets the total active time of all clients.
void
IdleLogManager::reset()
{
  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = (*i).second;
      info.update(time_source->get_time());
      info.total_active_time = 0;
    }
}


//! Update the idle log of a single client.
void
IdleLogManager::update_idlelog(ClientInfo &info, ActivityState state, bool changed)
{
  TRACE_ENTER_MSG("IdleLogManager::update_idlelog", ((int)state) << " " << changed);

  int current_time = time_source->get_time();
  IdleInterval *idle = &(info.idlelog.front());

  if (state == ACTIVITY_IDLE)
    {
      if (changed)
        {
          info.update(current_time);
          info.idlelog.push_front(IdleInterval(current_time, current_time));
        }
      else
        {
          idle->end_time = current_time;
        }
    }
  else if (state == ACTIVITY_ACTIVE)
    {
      if (changed)
        {
          idle->end_time = current_time;

          int total_idle = idle->end_time - idle->begin_time;
          if (total_idle < 1 && info.idlelog.size() > 1)
            {
              // Idle period too short. remove it
              info.idlelog.pop_front();
              idle = &(info.idlelog.front());
            }

          // Update start time of last active period.
          info.last_active_time = 0;
          info.last_active_begin_time = current_time;
        }
      else
        {
          info.last_active_time = current_time - info.last_active_begin_time;
        }
    }

  dump_idlelog(info);
  TRACE_EXIT();
}



int
IdleLogManager::compute_total_active_time()
{
  TRACE_ENTER("IdleLogManager::compute_total_active_time");
  int current_time = time_source->get_time();
  int active_time = 0;
  for (ClientMapIter it = clients.begin(); it != clients.end(); it++)
    {
      ClientInfo &info = (*it).second;
      info.update(current_time);
      active_time += info.total_active_time;
    }
  return active_time;
  TRACE_EXIT();
}


int
IdleLogManager::compute_active_time(int length)
{
  TRACE_ENTER("IdleLogManager::compute_active_time");

  int current_time = time_source->get_time();
  
  // Number of client.
  int size = clients.size();

  // Data for each client.
  IdleLogIter *iterators = new IdleLogIter[size];
  IdleLogIter *end_iterators = new IdleLogIter[size];
  bool *at_end = new bool[size];
  int *active_time = new int[size];

  // Init data for all clients.
  int count = 0;
  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = (*i).second;
      
      iterators[count] = info.idlelog.begin();
      end_iterators[count] = info.idlelog.end();
      active_time[count] = 0;
      at_end[count] = true;

      info.update(current_time);
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
  time_t begin_time = -1;
  time_t end_time = -1;
  
  while (!stop)
    {
      // Find latest event.
      last_time = -1;
      for (int i = 0; i < size; i ++)
        {
          if (iterators[i] != end_iterators[i])
            {
              IdleInterval ii = *(iterators[i]);
              
              time_t t = at_end[i] ? ii.end_time : ii.begin_time;
              
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
          IdleInterval ii = *(iterators[last_iter]);
          if (at_end[last_iter])
            {
              idle_count++;
              
              at_end[last_iter] = false;

              end_time = ii.end_time;
              active_time[last_iter] += ii.active_time;
            }
          else
            {
              at_end[last_iter] = true;
              iterators[last_iter]++;
              begin_time = ii.begin_time;
                
              if (idle_count == size)
                {
                  TRACE_MSG("Common idle period of " << (end_time - begin_time));
                  if ((end_time - begin_time) > length)
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

  int total_active_time = 0;
  for (int i = 0; i < size; i++)
    {
      TRACE_MSG(i << " " << active_time[i]);
      total_active_time += active_time[i];
    }

  TRACE_MSG("total = " << total_active_time);
  
  delete [] iterators;
  delete [] end_iterators;
  delete [] at_end;

  TRACE_EXIT();
  return total_active_time;
}


int
IdleLogManager::compute_idle_time()
{
  TRACE_ENTER("IdleLogManager::compute_idle_time");

  int current_time = time_source->get_time();
  
  int count = 0;
  time_t latest_start_time = 0;

  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = (*i).second;
      IdleInterval &idle = info.idlelog.front();

      info.update(current_time);
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


void
IdleLogManager::save_idlelog(ClientInfo &info)
{
  (void) info;
}


void
IdleLogManager::load_idlelog(ClientInfo &info, string filename)
{
  (void) info;
  (void) filename;
}


void
IdleLogManager::load_all_idlelogs()
{
}


void
IdleLogManager::update_idlelog(ClientInfo &info,  IdleInterval)
{
  (void) info;
}


bool
IdleLogManager::get_idlelog(unsigned char **buffer, int *size)
{
  TRACE_ENTER("IdleLogManager::get_idlelog");

  int current_time = time_source->get_time();
  
  ClientInfo &myinfo = clients[myid];
  
  myinfo.update(current_time);
    
  PacketBuffer state_packet;

  int idle_size = myinfo.idlelog.size();
  state_packet.create(1024 + idle_size * 16); // FIXME: 

  state_packet.pack_ushort(idle_size);
  state_packet.pack_string(myid.c_str());
  state_packet.pack_ulong(myinfo.total_active_time);
  state_packet.pack_ulong(current_time);
  state_packet.pack_byte(myinfo.master);
  state_packet.pack_byte(myinfo.state);

  IdleLogIter i = myinfo.idlelog.begin();
  while (i != myinfo.idlelog.end())
    {
      IdleInterval &idle = *i;

      int active_time = idle.active_time;

      int pos = state_packet.bytes_written();
      state_packet.pack_ushort(0);

      state_packet.pack_ulong((guint32)idle.begin_time);
      state_packet.pack_ulong((guint32)idle.end_time);
      state_packet.pack_ulong((guint32)active_time);
      
      state_packet.poke_ushort(pos, state_packet.bytes_written() - pos);
      i++;
    }


  *size = state_packet.bytes_written();
  *buffer = new unsigned char[*size + 1];
  memcpy(*buffer, state_packet.get_buffer(), *size);

  TRACE_EXIT();
  return true;
}


bool
IdleLogManager::set_idlelog(unsigned char *buffer, int size)
{
  TRACE_ENTER("IdleLogManager::set_idlelog_state");

  int current_time = time_source->get_time();
  
  PacketBuffer state_packet;
  state_packet.create(size);

  state_packet.pack_raw(buffer, size);
  
  int num_idle = state_packet.unpack_ushort();
  gchar *id = state_packet.unpack_string();
  int total_active_time = state_packet.unpack_ulong();
  time_t time_diff = state_packet.unpack_ulong() - current_time;
    
  ClientInfo info;
  info.master = (bool) state_packet.unpack_byte();
  info.state = (ActivityState) state_packet.unpack_byte();
  info.total_active_time = total_active_time;
  clients[id] = info;

  TRACE_MSG("id = " << id <<
            " numidle = " << num_idle <<
            " master  = " << info.master <<
            " state = " << info.state);

  for (int i = 0; i < num_idle; i++)
    {
      state_packet.unpack_ushort();

      IdleInterval idle;
      
      idle.begin_time = state_packet.unpack_ulong() - time_diff;
      idle.end_time = state_packet.unpack_ulong() - time_diff;
      idle.active_time = state_packet.unpack_ulong();

      TRACE_MSG(id << " " << idle.begin_time << " " << idle.end_time << " " << idle.active_time);
      clients[id].idlelog.push_back(idle);
    }

  TRACE_EXIT();
  return true;
}


//! A remote client has signed on.
void
IdleLogManager::signon_remote_client(string client_id)
{
  TRACE_ENTER_MSG("signon_remote_client", client_id);

  int current_time = time_source->get_time();
  
  ClientInfo info;
  clients[client_id] = info;
  clients[client_id].idlelog.push_front(IdleInterval(1, current_time));

  TRACE_EXIT();
}


//! A remote client has signed off.
void
IdleLogManager::signoff_remote_client(string client_id)
{
  TRACE_ENTER_MSG("signon_remote_client", client_id);

  clients[client_id].state = ACTIVITY_IDLE;
  clients[client_id].master = false;

  TRACE_EXIT();
}

void
IdleLogManager::dump_idlelog(ClientInfo &info)
{
#ifndef WIN32  
  TRACE_ENTER("IdleLogManager::dump_idlelog");

  TRACE_MSG("last_active_time " << info.last_active_time);
  TRACE_MSG("total_active_time " << info.total_active_time);
  IdleLogIter i = info.idlelog.begin();
  while (i != info.idlelog.end())
    {
      IdleInterval &idle = *i;

      struct tm begin_time;
      localtime_r(&idle.begin_time, &begin_time);
      struct tm end_time;
      localtime_r(&idle.end_time, &end_time);

      TRACE_MSG(   begin_time.tm_hour << ":"
                   << begin_time.tm_min << ":"
                   << begin_time.tm_sec << " - "
                   << end_time.tm_hour << ":"
                   << end_time.tm_min << ":" 
                   << end_time.tm_sec << " "
                   << idle.active_time
                   );
      i++;
    }
  TRACE_EXIT();
}
#endif
