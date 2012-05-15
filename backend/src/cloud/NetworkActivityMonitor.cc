// NetworkActivityMonitor.cc --- Network (networked) ActivityMonitor
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

#include "CoreFactory.hh"
#include "ICore.hh"
#include "INetwork.hh"

#include "cloud.pb.h"

using namespace std;

NetworkActivityMonitor::NetworkActivityMonitor(INetwork::Ptr network)
  : network(network),
    suspended(false),
    state(ACTIVITY_IDLE)
{
  core = CoreFactory::get_core();
}


NetworkActivityMonitor::~NetworkActivityMonitor()
{
}


void
NetworkActivityMonitor::init()
{
  network->signal_message(1, cloud::ActivityState::kTypeFieldNumber)
    .connect(sigc::mem_fun(*this, &NetworkActivityMonitor::on_activity_message));
}

void
NetworkActivityMonitor::on_activity_message(NetworkMessageBase::Ptr message)
{
  TRACE_ENTER("NetworkActivityMonitor::on_activity_message");

  //NetworkMessage<cloud::ActivityState> x(message);

  boost::shared_ptr<cloud::ActivityState> as = message->cast<cloud::ActivityState>();
  TRACE_MSG("as " << as);

  boost::shared_ptr<cloud::Break> b = message->cast<cloud::Break>();
  TRACE_MSG("b " << b);
  
  // const UUID &remote_id = message->source;
  // const ActivityState state = message->msg()->state();

  //     StateIter i = states.find(remote_id);
  //     if (i != states.end())
  //       {
  //         RemoteState &rs = i->second;

  //         rs.lastupdate = core->get_time();
          
  //         if (rs.state != state)
  //           {
  //             rs.since = rs.lastupdate;
  //           }
          
  //         rs.state = state;
  //       }
  //     else
  //       {
  //         RemoteState rs;
  //         rs.id == remote_id;
  //         rs.state = state;
  //         rs.lastupdate = core->get_time();
  //         rs.since = rs.lastupdate;
          
  //         states[remote_id] = rs;
  //       }
  //   }

  TRACE_EXIT();
}


//! Returns the current state
bool
NetworkActivityMonitor::get_active()
{
  TRACE_ENTER("NetworkActivityMonitor::get_active");

  time_t current_time = core->get_time();
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
  
  // since = -1;
  
  // StateIter i = states.find(remote_id);
  // if (i != states.end())
  //   {
  //     RemoteState &rs = i->second;

  //     ret = rs.state == ACTIVITY_ACTIVE;
  //     since = rs.since;
  //   }
  
  TRACE_EXIT();
  return ret;
}
