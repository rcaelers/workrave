// IdleLogManager.hh --- Bookkeeping of idle time
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
// $Id$
//

#ifndef IDLELOGMANAGER_HH
#define IDLELOGMANAGER_HH

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <iostream>
#include <string>
#include <list>
#include <map>

using namespace std;

#include "ActivityMonitor.hh"

class TimeSource;

class IdleLogManager
{
private:
  // A Single idle time interval
  struct IdleInterval
  {
    IdleInterval() :
      begin_time(0),
      end_time(0),
      active_time(0)
    {
    }

    IdleInterval(time_t b, time_t e) :
      begin_time(b),
      end_time(e),
      active_time(0)
    {
    }

    //! Start time of interval
    time_t begin_time;

    //! End time of interface
    time_t end_time;

    //! Elapsed active time AFTER the idle interval.
    time_t active_time;
  };
  

  typedef list<IdleInterval> IdleLog;
  typedef IdleLog::iterator IdleLogIter;

  //! Idle information of a single client.
  struct ClientInfo
  {
    ClientInfo() :
      state(ACTIVITY_UNKNOWN),
      master(false),
      total_active_time(0),
      last_active_begin_time(0),
      last_active_time(0)
    {
    }

    //! List of idle period of this client.
    IdleLog idlelog;

    //! Last known state
    ActivityState state;

    //! Last known master status;
    bool master;

    //! Total active time since daily reset.
    time_t total_active_time;

    //! Start time of last active period.
    time_t last_active_begin_time;

    //! Total active time since last_active_begin_time
    time_t last_active_time;

    //! Update the active time of the most recent idle interval.
    void update(time_t current_time)
    {
      if (last_active_begin_time != 0)
        {
          IdleInterval *idle = &(idlelog.front());
          idle->active_time += (current_time - last_active_begin_time);
          total_active_time += (current_time - last_active_begin_time);
            
          last_active_time = 0;
          last_active_begin_time = 0;
        }
    }
  };
  
  typedef map<string, ClientInfo> ClientMap;
  typedef ClientMap::iterator ClientMapIter;

private:
  // My ID
  string myid;

  //! Info about all clients.
  ClientMap clients;

  //! Time
  TimeSource *time_source;

private:
  void update_idlelog(ClientInfo &info, ActivityState state, bool changed);
  void dump_idlelog(ClientInfo &info);
  
public:
  IdleLogManager(string myid, TimeSource *control);

  void update_all_idlelogs(string master_id, ActivityState state);
  void reset();

  void signon_remote_client(string client_id);
  void signoff_remote_client(string client_id);

  bool get_idlelog(unsigned char **buffer, int *size);
  bool set_idlelog(unsigned char *buffer, int size);

  int compute_total_active_time();
  int compute_active_time(int length);
  int compute_idle_time();
  
  void save_idlelog(ClientInfo &info);
  void load_idlelog(ClientInfo &info, string filename);
  void load_all_idlelogs();
  void update_idlelog(ClientInfo &info,  IdleInterval);


};


#endif // IDLELOGMANAGER_HH
