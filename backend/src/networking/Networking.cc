// Networking.cc
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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <string>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include <glib.h>

#include "debug.hh"

#include "Networking.hh"
#include "Util.hh"

#include "workrave.pb.h"

using namespace std;

Networking::Ptr
Networking::create(ICloud::Ptr network, IConfigurator::Ptr configurator)
{
  return Networking::Ptr(new Networking(network, configurator));
}

Networking::Ptr
Networking::create(IConfigurator::Ptr configurator)
{
  ICloud::Ptr cloud = ICloud::create();
  // FIXME:
  cloud->init(2701, "rob@workrave", "kjsdapkidszahf");
  return Networking::Ptr(new Networking(cloud, configurator));
}


//! Constructs a cloud
Networking::Networking(ICloud::Ptr network, IConfigurator::Ptr configurator) : network(network), configurator(configurator)
{
  TRACE_ENTER("Networking::Networking");
  configuration_manager = NetworkConfigurationManager::create(network, configurator);
  activity_monitor = NetworkActivityMonitor::create(network);
  TRACE_EXIT();
}


//! Destructs the workrave cloud.
Networking::~Networking()
{
  TRACE_ENTER("Networking::~Networking");
  TRACE_EXIT();
}


//! Initializes the workrave cloud.
void
Networking::init()
{
  TRACE_ENTER("Networking::init");
  configuration_manager->init();
  activity_monitor->init();

  // network->subscribe("breaklinkevent", this);
  // network->subscribe("corelinkevent", this);
  // network->subscribe("timerstatelinkevent", this);
  
  // network->report_active(false);
  // for (int i = 0; i < BREAK_ID_SIZEOF; i++)
  //   {
  //     network->report_timer_state(i, false);
  //   }
  
  TRACE_EXIT();
}


//! Terminates the network announcer.
void
Networking::terminate()
{
  TRACE_ENTER("Networking::terminate");
  TRACE_EXIT();
}


//! Periodic heartbeart from the core.
void
Networking::heartbeat()
{
  TRACE_ENTER("Networking::heartbeat");
  static bool once = false;
  
  // TODO: debugging code.
  if (!once)
    {
      boost::shared_ptr<workrave::networking::ActivityState> a(new workrave::networking::ActivityState());
      a->set_state(1);
      network->send_message(a, MessageParams::create());
      once = true;
    }
  
  TRACE_EXIT();
}


/********************************************************************************/
/**** Networking support                                                   ******/
/********************************************************************************/

#ifdef HAVE_BROKEN_DISTRIBUTION
//! Process Break event
void
Core::break_event_received(const BreakLinkEvent *event)
{
  TRACE_ENTER_MSG("Core::break_event_received", event->str());

  BreakId break_id = event->get_break_id();
  BreakLinkEvent::BreakEvent break_event = event->get_break_event();

  switch(break_event)
    {
    case BreakLinkEvent::BREAK_EVENT_USER_POSTPONE:
      do_postpone_break(break_id);
      break;

    case BreakLinkEvent::BREAK_EVENT_USER_SKIP:
      do_skip_break(break_id);
      break;

    case BreakLinkEvent::BREAK_EVENT_USER_FORCE_BREAK:
      do_force_break(break_id, BREAK_HINT_USER_INITIATED);
      break;

    case BreakLinkEvent::BREAK_EVENT_SYST_FORCE_BREAK:
      do_force_break(break_id, (BreakHint)0);
      break;

    case BreakLinkEvent::BREAK_EVENT_SYST_STOP_PRELUDE:
      do_stop_prelude(break_id);
      break;

    default:
      break;
    }

  TRACE_EXIT();
}


//! Process Core event
void
Core::core_event_received(const CoreLinkEvent *event)
{
  TRACE_ENTER_MSG("Core::core_event_received", event->str());

  CoreLinkEvent::CoreEvent core_event = event->get_core_event();

  switch(core_event)
    {
    case CoreLinkEvent::CORE_EVENT_MODE_SUSPENDED:
      set_operation_mode_no_event(OPERATION_MODE_SUSPENDED);
      break;

    case CoreLinkEvent::CORE_EVENT_MODE_QUIET:
      set_operation_mode_no_event(OPERATION_MODE_QUIET);
      break;

    case CoreLinkEvent::CORE_EVENT_MODE_NORMAL:
      set_operation_mode_no_event(OPERATION_MODE_NORMAL);
      break;

    default:
      break;
    }

  TRACE_EXIT();
}


//! Process Timer state event
void
Core::timer_state_event_received(const TimerStateLinkEvent *event)
{
  TRACE_ENTER_MSG("Core::timer_state_event_received", event->str());

  const std::vector<int> &idle_times = event->get_idle_times();
  const std::vector<int> &active_times = event->get_active_times();
    
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Timer *timer = breaks[i].get_timer();

      time_t old_idle = timer->get_elapsed_idle_time();
      time_t old_active = timer->get_elapsed_time();

      int idle = idle_times[i];
      int active = active_times[i];

      TRACE_MSG("Timer " << i <<
                " idle " <<  idle << " " << old_idle <<
                " active" << active << " " << old_active); 

      time_t remote_active_since = 0;
      bool remote_active = network->is_remote_active(event->get_source(),
                                                        remote_active_since);
      
      if (abs((int)(idle - old_idle)) >= 2 ||
          abs((int)(active - old_active)) >= 2 ||
          /* Remote party is active, and became active after us */
          (remote_active && remote_active_since > active_since))
        {
          timer->set_state(active, idle);
        }
    }
 
  TRACE_EXIT();
}


//! Broadcast current timer state.
void
Core::broadcast_state()
{
  std::vector<int> idle_times;
  std::vector<int> active_times;
  
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Timer *timer = breaks[i].get_timer();

      idle_times.push_back((int)timer->get_elapsed_idle_time());
      active_times.push_back((int)timer->get_elapsed_time());
    }

  TimerStateLinkEvent event(idle_times, active_times);
  network->send_event(&event);
}

#endif

