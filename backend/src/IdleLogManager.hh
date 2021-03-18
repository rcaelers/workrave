// IdleLogManager.hh --- Bookkeeping of idle time
//
// Copyright (C) 2003, 2004 Rob Caelers <robc@krandor.org>
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

#ifndef IDLELOGMANAGER_HH
#define IDLELOGMANAGER_HH

#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#if STDC_HEADERS
#  include <cstddef>
#  include <cstdlib>
#else
#  if HAVE_STDLIB_H
#    include <stdlib.h>
#  endif
#endif
#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <iostream>
#include <string>
#include <list>
#include <map>

using namespace std;

#include "ActivityMonitor.hh"

class TimeSource;
class PacketBuffer;

class IdleLogManager
{
private:
  // A Single idle time interval
  struct IdleInterval
  {
    IdleInterval() = default;

    IdleInterval(time_t b, time_t e)
      : begin_time(b)
      , end_idle_time(e)
      , end_time(e)
    {
    }

    //! Start time of idle interval
    time_t begin_time{0};

    //! End time of idle interval (and start of active part)
    time_t end_idle_time{0};

    //! End time of active interval.
    time_t end_time{0};

    //! Elapsed active time AFTER the idle interval.
    time_t active_time{0};

    //! Yet to be saved
    bool to_be_saved{false};
  };

  typedef list<IdleInterval> IdleLog;
  typedef IdleLog::iterator IdleLogIter;
  typedef IdleLog::reverse_iterator IdleLogRIter;

  //! Idle information of a single client.
  struct ClientInfo
  {
    ClientInfo() = default;

    //! ID
    string client_id;

    //! List of idle period of this client.
    IdleLog idlelog;

    //! Current interval
    IdleInterval current_interval;

    //! Last known state
    ActivityState state{ACTIVITY_UNKNOWN};

    //! Last known master status;
    bool master{false};

    //! Total active time since daily reset.
    time_t total_active_time{0};

    //! Start time of last active period.
    time_t last_active_begin_time{0};

    //! Total active time since last_active_begin_time
    time_t last_active_time{0};

    //! Last time this idle log was updated.
    time_t last_update_time{0};

    //! Update the active time of the most recent idle interval.
    void update_active_time(time_t current_time)
    {
      if (last_active_begin_time != 0)
        {
          current_interval.active_time += (current_time - last_active_begin_time);
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
  const TimeSource *time_source{nullptr};

  //! Last time we performed an expiration run.
  time_t last_expiration_time{0};

public:
  IdleLogManager(string myid, const TimeSource *control);

  void update_all_idlelogs(string master_id, ActivityState state);
  void reset();
  void init();
  void terminate();

  void signon_remote_client(string client_id);
  void signoff_remote_client(string client_id);

  void get_idlelog(PacketBuffer &buffer);
  void set_idlelog(PacketBuffer &buffer);

  time_t compute_total_active_time();
  time_t compute_active_time(int length);
  time_t compute_idle_time();

private:
  void update_idlelog(ClientInfo &info, ActivityState state, bool master);
  void expire();
  void expire(ClientInfo &info);

  void pack_idle_interval(PacketBuffer &buffer, const IdleInterval &idle) const;
  void unpack_idle_interval(PacketBuffer &buffer, IdleInterval &idle, time_t delta_time) const;

  void pack_idlelog(PacketBuffer &buffer, const ClientInfo &ci) const;
  void unpack_idlelog(PacketBuffer &buffer, ClientInfo &ci, time_t &pack_time, int &num_intervals) const;
  void unlink_idlelog(PacketBuffer &buffer) const;

  void save_index();
  void load_index();
  void save_idlelog(ClientInfo &info);
  void load_idlelog(ClientInfo &info);

  void save();
  void load();
  void update_idlelog(ClientInfo &info, const IdleInterval &idle);

  void fix_idlelog(ClientInfo &info);
  void dump_idlelog(ClientInfo &info);
};

#endif // IDLELOGMANAGER_HH
