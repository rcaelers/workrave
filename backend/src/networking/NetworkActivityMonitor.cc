// NetworkActivityMonitor.cc ---  (networked) NetworkActivityMonitor
//
// Copyright (C) 2007, 2008, 2009, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#include "fog/Fog.hh"
#include "utils/TimeSource.hh"

#include "workrave.pb.h"

using namespace std;
using namespace workrave::utils;

NetworkActivityMonitor::Ptr
NetworkActivityMonitor::create(IFog::Ptr fog, ICore::Ptr core)
{
  return NetworkActivityMonitor::Ptr(new NetworkActivityMonitor(fog, core));
}


NetworkActivityMonitor::NetworkActivityMonitor(IFog::Ptr fog, ICore::Ptr core)
  : fog(fog),
    core(core),
    local_active(false),
    local_active_since(0)
{
  TRACE_ENTER("Networking::NetworkActivityMonitor");
  TRACE_EXIT();
}


NetworkActivityMonitor::~NetworkActivityMonitor()
{
  TRACE_ENTER("Networking::~NetworkActivityMonitor");
  TRACE_EXIT();
}


void
NetworkActivityMonitor::init()
{
  TRACE_ENTER("NetworkActivityMonitor::init");
  ICoreHooks::Ptr hooks = core->get_hooks();

  hooks->hook_is_active().connect(boost::bind(&NetworkActivityMonitor::on_hook_is_active, this));
  hooks->signal_local_active_changed().connect(boost::bind(&NetworkActivityMonitor::on_local_active_changed, this, _1));
    
  fog->signal_message(1, workrave::networking::Activity::kTypeFieldNumber)
    .connect(boost::bind(&NetworkActivityMonitor::on_activity_message, this, _1, _2));
  TRACE_EXIT()
}


void
NetworkActivityMonitor::heartbeat()
{
  TRACE_ENTER("NetworkActivityMonitor::heartbeat");
  if (local_active &&
      (TimeSource::get_monotonic_time_sec() > local_active_since) &&
      (TimeSource::get_monotonic_time_sec() - local_active_since) % 5 == 0)
    {
      boost::shared_ptr<workrave::networking::Activity> a(new workrave::networking::Activity());
      a->set_active(local_active);
      fog->send_message(a, MessageParams::create());
    }
  TRACE_EXIT()
}


bool
NetworkActivityMonitor::on_hook_is_active()
{
  return is_active();
}


void
NetworkActivityMonitor::on_local_active_changed(bool active)
{
  local_active = active;
  local_active_since_real = active ? TimeSource::get_real_time_sec() : 0;
  local_active_since = active ? TimeSource::get_monotonic_time_sec() : 0;
  
  boost::shared_ptr<workrave::networking::Activity> a(new workrave::networking::Activity());
  a->set_active(active);
  fog->send_message(a, MessageParams::create());
}


void
NetworkActivityMonitor::on_activity_message(Message::Ptr message, MessageContext::Ptr context)
{
  TRACE_ENTER("NetworkActivityMonitor::on_activity_message");

  boost::shared_ptr<workrave::networking::Activity> a = boost::dynamic_pointer_cast<workrave::networking::Activity>(message);

  if (a)
    {
      const UUID &remote_id = context->source;
      bool active = a->active();
      
      TRACE_MSG("Active " << active << " from" << remote_id.str());
      
      RemoteActivityIter i = remote_activities.find(remote_id);
      if (i != remote_activities.end())
        {
          RemoteActivity &ra = i->second;

          ra.active = active;
          ra.lastupdate = TimeSource::get_monotonic_time_sec();
          
          if (ra.active != active)
            {
              ra.since = ra.lastupdate;
            }
        }
      else
        {
          RemoteActivity ra;
          ra.id = remote_id;
          ra.active = active;
          ra.lastupdate = TimeSource::get_monotonic_time_sec();
          ra.since = ra.lastupdate;
          
          remote_activities[remote_id] = ra;
        }
    }

  TRACE_EXIT();
}


//! Returns the current state
bool
NetworkActivityMonitor::is_active()
{
  TRACE_ENTER("NetworkActivityMonitor::is_active");

  gint64 current_time = TimeSource::get_monotonic_time_sec();
  int num_active = 0;

  RemoteActivityIter i = remote_activities.begin();
  while (i != remote_activities.end())
    {
      RemoteActivity &ra = i->second;
      RemoteActivityIter next = i;
      next++;
      
      if (ra.lastupdate + 30 < current_time)
        {
          remote_activities.erase(i);
        }
      else
        {
          if (ra.active)
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
NetworkActivityMonitor::is_remote_active(const UUID &remote_id, time_t &since)
{
  TRACE_ENTER("NetworkActivityMonitor::is_remote_active");
  bool ret = false;
  
  since = -1;
  
  RemoteActivityCIter i = remote_activities.find(remote_id);
  if (i != remote_activities.end())
    {
      const RemoteActivity &ra = i->second;
      ret = ra.active;
      since = ra.since;
    }
  
  TRACE_EXIT();
  return ret;
}

gint64
NetworkActivityMonitor::get_local_active_since() const
{
  return local_active_since;
}

bool
NetworkActivityMonitor::is_local_active() const
{
  return local_active;
}
