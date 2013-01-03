// NetworkActivityMonitor.hh
//
// Copyright (C) 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef NETWORKACTIVITYMONITOR_HH
#define NETWORKACTIVITYMONITOR_HH

#include <map>

#include "fog/Fog.hh"

#include "ICore.hh"
#include "IActivityMonitor.hh"

using namespace workrave;
using namespace workrave::fog;

class NetworkActivityMonitor
{
public:
  typedef boost::shared_ptr<NetworkActivityMonitor> Ptr;

public:
  static Ptr create(IFog::Ptr fog, ICore::Ptr core);

public:
  NetworkActivityMonitor(IFog::Ptr fog, ICore::Ptr core);
  virtual ~NetworkActivityMonitor();

  //! Initializes the monitor
  void init();
  void heartbeat();

  // Internal
  void report_active(bool active);

  bool is_active();
  bool is_remote_active(const UUID &remote, time_t &since);
  gint64 get_local_active_since() const;
  bool is_local_active() const;
  
private:

  struct RemoteActivity
  {
    UUID id;
    bool active;
    gint64 lastupdate;
    gint64 since;
  };

  // Known states
  typedef std::map<UUID, RemoteActivity> RemoteActivities;
  typedef RemoteActivities::iterator RemoteActivityIter;
  typedef RemoteActivities::const_iterator RemoteActivityCIter;

private:
  void on_activity_message(Message::Ptr, MessageContext::Ptr);
  void on_local_active_changed(bool active);
  bool on_hook_is_active();
  
private:
  //! The networking core
  IFog::Ptr fog;

  //! The core
  ICore::Ptr core;

  //! Is the user locally active;
  bool local_active;

  //!
  gint64 local_active_since_real;
  
  //!
  gint64 local_active_since;
  
  //! Remote states
  RemoteActivities remote_activities;
};

#endif // NETWORKACTIVITYMONITOR_HH
