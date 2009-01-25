// LinkedActivityMonitor.cc --- Linked (networked) ActivityMonitor
//
// Copyright (C) 2007, 2008, 2009 Rob Caelers <robc@krandor.nl>
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

static const char rcsid[] = "$Id: ActivityMonitor.cc 1087 2006-10-01 18:40:08Z dotsphinx $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include "LinkedActivityMonitor.hh"

#include "ICore.hh"
#include "INetwork.hh"

#include "CoreFactory.hh"
#include "LinkRouter.hh"
#include "LinkEvent.hh"
#include "ActivityLinkEvent.hh"

using namespace std;

LinkedActivityMonitor::LinkedActivityMonitor()
  : suspended(false),
    state(ACTIVITY_IDLE)
{
  core = CoreFactory::get_core();
}


LinkedActivityMonitor::~LinkedActivityMonitor()
{
}


void
LinkedActivityMonitor::init()
{
  INetwork *router = CoreFactory::get_networking();

  router->subscribe("activitylinkevent", this);
  router->subscribe("presenselinkevent", this);
}


void
LinkedActivityMonitor::event_received(LinkEvent *event)
{
  TRACE_ENTER_MSG("LinkedActivityMonitor::event_received", event->str());

  string eventid = event->get_eventid();

  if (eventid == "activitylinkevent")
    {
      handle_activity_event(event);
    }
  else if (eventid == "presenselinkevent")
    {
      handle_presense_event(event);
    }
  else
    {
    }

  TRACE_EXIT();
}

void
LinkedActivityMonitor::handle_activity_event(LinkEvent *event)
{
  TRACE_ENTER_MSG("LinkedActivityMonitor::handle_activity_event", event->str());

  ActivityLinkEvent *act = dynamic_cast<ActivityLinkEvent *>(event);
  if (act != NULL)
    {
      const WRID &remote_id = act->get_source();
      const ActivityState state = act->get_state();

      StateIter i = states.find(remote_id);
      if (i != states.end())
        {
          RemoteState &rs = i->second;

          rs.lastupdate = core->get_time();
          
          if (rs.state != state)
            {
              rs.since = rs.lastupdate;
            }
          
          rs.state = state;
        }
      else
        {
          RemoteState rs;
          rs.id == remote_id;
          rs.state = state;
          rs.lastupdate = core->get_time();
          rs.since = rs.lastupdate;
          
          states[remote_id] = rs;
        }
    }

  TRACE_EXIT();
}


void
LinkedActivityMonitor::handle_presense_event(LinkEvent *event)
{
  TRACE_ENTER_MSG("LinkedActivityMonitor::handle_activity_event", event->str());

  TRACE_EXIT();
}


//! Returns the current state
bool
LinkedActivityMonitor::get_active()
{
  TRACE_ENTER("LinkedActivityMonitor::get_active");

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
LinkedActivityMonitor::is_active(const WRID &remote_id, time_t &since)
{
  TRACE_ENTER("LinkedActivityMonitor::is_active");
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
