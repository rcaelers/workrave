// NetworkActivityMonitor.hh
//
// Copyright (C) 2012 Rob Caelers <robc@krandor.nl>
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

#include "cloud/Cloud.hh"

#include "IActivityMonitor.hh"

using namespace workrave;
using namespace workrave::cloud;

class NetworkActivityMonitor
{
public:
  typedef boost::shared_ptr<NetworkActivityMonitor> Ptr;

public:
  static Ptr create(ICloud::Ptr network);

public:
  NetworkActivityMonitor(ICloud::Ptr network);
  virtual ~NetworkActivityMonitor();

  //! Initializes the monitor
  void init();

  // Internal
  void report_active(bool active);

  bool get_active();
  bool is_active(const UUID &remote, time_t &since);

private:

  struct RemoteState
  {
    UUID id;
    ActivityState state;
    time_t lastupdate;
    time_t since;
  };

  // Known states
  typedef std::map<UUID, RemoteState> States;
  typedef States::iterator StateIter;
  typedef States::const_iterator StateCIter;

private:
  void on_activity_message(Message::Ptr, MessageContext::Ptr);
  
private:
  //! The networking core
  ICloud::Ptr network;

  //! Monitor suspended?
  bool suspended;

  //! Current state
  ActivityState state;

  //! Remote states
  States states;
};

#endif // NETWORKACTIVITYMONITOR_HH
