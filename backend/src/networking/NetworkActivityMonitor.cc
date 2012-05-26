// NetworkActivityMonitor.cc ---  (networked) NetworkActivityMonitor
//
// Copyright (C) 2007, 2008, 2009, 2012 Rob Caelers <robc@krandor.nl>
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

#include "debug.hh"

#include "NetworkActivityMonitor.hh"

#include "Cloud.hh"
#include "TimeSource.hh"

#include "workrave.pb.h"

using namespace std;
using namespace workrave::utils;

NetworkActivityMonitor::Ptr
NetworkActivityMonitor::create(ICloud::Ptr network)
{
  return NetworkActivityMonitor::Ptr(new NetworkActivityMonitor(network));
}


NetworkActivityMonitor::NetworkActivityMonitor(ICloud::Ptr network)
  : network(network),
    suspended(false),
    state(ACTIVITY_IDLE)
{
}


NetworkActivityMonitor::~NetworkActivityMonitor()
{
}


void
NetworkActivityMonitor::init()
{
  TRACE_ENTER("NetworkActivityMonitor::init");
  network->signal_message(1, workrave::networking::ActivityState::kTypeFieldNumber)
    .connect(boost::bind(&NetworkActivityMonitor::on_activity_message, this, _1, _2));
  TRACE_EXIT()
}

void
NetworkActivityMonitor::on_activity_message(Message::Ptr message, MessageContext::Ptr context)
{
  TRACE_ENTER("NetworkActivityMonitor::on_activity_message");

  boost::shared_ptr<workrave::networking::ActivityState> a = boost::dynamic_pointer_cast<workrave::networking::ActivityState>(message);

  if (a)
    {
      const UUID &remote_id = context->source;
      const ActivityState state = (ActivityState) a->state();

      TRACE_MSG("state " << state << " from" << remote_id.str());
      
      StateIter i = states.find(remote_id);
      if (i != states.end())
        {
          RemoteState &rs = i->second;

          rs.lastupdate = TimeSource::get_time();
          
          if (rs.state != state)
            {
              rs.since = rs.lastupdate;
            }
          
          rs.state = state;
        }
      else
        {
          RemoteState rs;
          rs.id = remote_id;
          rs.state = state;
          rs.lastupdate = TimeSource::get_time();
          rs.since = rs.lastupdate;
          
          states[remote_id] = rs;
        }
    }

  TRACE_EXIT();
}


//! Returns the current state
bool
NetworkActivityMonitor::get_active()
{
  TRACE_ENTER("NetworkActivityMonitor::get_active");

  time_t current_time = TimeSource::get_time();
  int num_active = 0;

  StateIter i = states.begin();
  while (i != states.end())
    {
      RemoteState &rs = i->second;
      StateIter next = i;
      next++;
      
      if (rs.lastupdate + 30 < current_time)
        {
          states.erase(i);
        }
      else
        {
          if (rs.state == ACTIVITY_ACTIVE)
            {
              num_active++;
            }
        }

      i = next;
    }

  TRACE_MSG(num_active);
  TRACE_EXIT();
  return num_active > 0;
}


//! Returns the current state
bool
NetworkActivityMonitor::is_active(const UUID &remote_id, time_t &since)
{
  TRACE_ENTER("NetworkActivityMonitor::is_active");
  bool ret = false;
  
  since = -1;
  
  StateIter i = states.find(remote_id);
  if (i != states.end())
    {
      RemoteState &rs = i->second;

      ret = rs.state == ACTIVITY_ACTIVE;
      since = rs.since;
    }
  
  TRACE_EXIT();
  return ret;
}
