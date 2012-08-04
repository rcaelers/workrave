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

#include "utils/TimeSource.hh"
#include "debug.hh"

#include "Networking.hh"
#include "Util.hh"

using namespace std;
using namespace workrave::utils;

Networking::Ptr
Networking::create(ICore::Ptr core)
{
  return Networking::Ptr(new Networking(core));
}


//! Constructs a cloud
Networking::Networking(ICore::Ptr core) : core(core)
{
  TRACE_ENTER("Networking::Networking");
  configurator = core->get_configurator();
  cloud = ICloud::create();

  string user;
  configurator->get_value("plugins/networking/user", user);

  string secret;
  configurator->get_value("plugins/networking/secret", secret);

  int port;
  configurator->get_value("plugins/networking/port", port);
  
  cloud->init(port, user, secret);
  
  configuration_manager = NetworkConfigurationManager::create(cloud, core);
  activity_monitor = NetworkActivityMonitor::create(cloud, core);
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

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      IBreak::Ptr b = core->get_break((BreakId)i);

      break_skipped_connection[i] =
        b->signal_skipped().connect(boost::bind(&Networking::on_break_skipped, this, (BreakId) i));
      break_postponed_connection[i] =
        b->signal_postponed().connect(boost::bind(&Networking::on_break_postponed, this, (BreakId) i));
      break_forced_connection[i] =
        b->signal_break_forced().connect(boost::bind(&Networking::on_break_forced, this, (BreakId) i, _1));
    }
  
  operation_mode_connection =
    core->signal_operation_mode_changed().connect(boost::bind(&Networking::on_operation_mode_changed, this, _1));
  usage_mode_connection =
    core->signal_usage_mode_changed().connect(boost::bind(&Networking::on_usage_mode_changed, this, _1));
  
  cloud->signal_message(1, workrave::networking::Break::kTypeFieldNumber)
    .connect(boost::bind(&Networking::on_break_message, this, _1, _2));
  cloud->signal_message(1, workrave::networking::Timer::kTypeFieldNumber)
    .connect(boost::bind(&Networking::on_timer_message, this, _1, _2));
  cloud->signal_message(1, workrave::networking::OperationMode::kTypeFieldNumber)
    .connect(boost::bind(&Networking::on_operation_mode_message, this, _1, _2));
  cloud->signal_message(1, workrave::networking::UsageMode::kTypeFieldNumber)
    .connect(boost::bind(&Networking::on_usage_mode_message, this, _1, _2));
  
  TRACE_EXIT();
}


void
Networking::start_announce()
{
  boost::dynamic_pointer_cast<ICloudTest>(cloud)->start_announce();
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

  cloud->heartbeat();
  activity_monitor->heartbeat();

  send_timer_state();
  
  TRACE_EXIT();
}


void
Networking::connect(const std::string host, int port)
{
  cloud->connect(host, port);
}

/********************************************************************************/
/**** Networking support                                                   ******/
/********************************************************************************/

void
Networking::send_break_event(BreakId id, workrave::networking::Break::BreakEvent event)
{
  boost::shared_ptr<workrave::networking::Break> e(new workrave::networking::Break());
  e->set_break_id(id);
  e->set_break_event(event);
  cloud->send_message(e, MessageParams::create());
}


void
Networking::on_break_postponed(BreakId id)
{
  send_break_event(id, workrave::networking::Break::BREAK_EVENT_USER_POSTPONE);
}


void
Networking::on_break_skipped(BreakId id)
{
  send_break_event(id, workrave::networking::Break::BREAK_EVENT_USER_SKIP);
}


void
Networking::on_break_forced(BreakId id, BreakHint hint)
{
  boost::shared_ptr<workrave::networking::Break> e(new workrave::networking::Break());
  e->set_break_id(id);
  e->set_break_event(workrave::networking::Break::BREAK_EVENT_FORCE_BREAK);
  e->set_break_hint(hint);
  cloud->send_message(e, MessageParams::create());
}


void
Networking::on_operation_mode_changed(OperationMode mode)
{
  boost::shared_ptr<workrave::networking::OperationMode> e(new workrave::networking::OperationMode());

  switch (mode)
    {
    case OPERATION_MODE_SUSPENDED:
      e->set_mode(workrave::networking::OperationMode::OPERATION_MODE_SUSPENDED);
      break;
      
    case OPERATION_MODE_QUIET:
      e->set_mode(workrave::networking::OperationMode::OPERATION_MODE_QUIET);
      break;
      
    case OPERATION_MODE_NORMAL:
      e->set_mode(workrave::networking::OperationMode::OPERATION_MODE_NORMAL);
      break;

    default:
      break;
    }
  cloud->send_message(e, MessageParams::create());
}


void
Networking::on_usage_mode_changed(UsageMode mode)
{
  boost::shared_ptr<workrave::networking::UsageMode> e(new workrave::networking::UsageMode());
  switch (mode)
    {
    case USAGE_MODE_NORMAL:
      e->set_mode(workrave::networking::UsageMode::USAGE_MODE_NORMAL);
      break;
      
    case USAGE_MODE_READING:
      e->set_mode(workrave::networking::UsageMode::USAGE_MODE_READING);
      break;
    }
  cloud->send_message(e, MessageParams::create());
}


void
Networking::on_break_message(Message::Ptr message, MessageContext::Ptr context)
{
  TRACE_ENTER("Networking::on_break_message");

  boost::shared_ptr<workrave::networking::Break> e = boost::dynamic_pointer_cast<workrave::networking::Break>(message);

  if (e)
    {
      BreakId break_id = (BreakId) e->break_id();
      workrave::networking::Break::BreakEvent break_event = e->break_event();

      IBreak::Ptr b = core->get_break((BreakId)break_id);

      switch(break_event)
        {
        case workrave::networking::Break::BREAK_EVENT_USER_POSTPONE:
          {
            boost::signals2::shared_connection_block block(break_postponed_connection[break_id]);
            b->postpone_break();
          }
          break;
          
        case workrave::networking::Break::BREAK_EVENT_USER_SKIP:
          {
            boost::signals2::shared_connection_block block(break_skipped_connection[break_id]);
            b->skip_break();
          }
          break;
          
        case workrave::networking::Break::BREAK_EVENT_FORCE_BREAK:
          {
            boost::signals2::shared_connection_block block(break_forced_connection[break_id]);
            core->force_break(break_id, (BreakHint) e->break_hint());
          }
          break;
          
        default:
          break;
        }
    }
  TRACE_EXIT();
}


//! Process OperationMode event
void
Networking::on_operation_mode_message(Message::Ptr message, MessageContext::Ptr context)
{
  TRACE_ENTER("Networking::on_core_message");

  boost::shared_ptr<workrave::networking::OperationMode> e = boost::dynamic_pointer_cast<workrave::networking::OperationMode>(message);

  if (e)
    {
      boost::signals2::shared_connection_block block(operation_mode_connection);
  
      switch(e->mode())
        {
        case workrave::networking::OperationMode::OPERATION_MODE_SUSPENDED:
          core->set_operation_mode(OPERATION_MODE_SUSPENDED);
          break;
      
        case workrave::networking::OperationMode::OPERATION_MODE_QUIET:
          core->set_operation_mode(OPERATION_MODE_QUIET);
          break;
      
        case workrave::networking::OperationMode::OPERATION_MODE_NORMAL:
          core->set_operation_mode(OPERATION_MODE_NORMAL);
          break;

        default:
          break;
        }
    }
  TRACE_EXIT();
}


//! Process OperationMode event
void
Networking::on_usage_mode_message(Message::Ptr message, MessageContext::Ptr context)
{
  TRACE_ENTER("Networking::on_usage_mode_message");
  boost::shared_ptr<workrave::networking::UsageMode> e = boost::dynamic_pointer_cast<workrave::networking::UsageMode>(message);

  if (e)
    {
      boost::signals2::shared_connection_block block(usage_mode_connection);

      switch(e->mode())
        {
        case workrave::networking::UsageMode::USAGE_MODE_NORMAL:
          core->set_usage_mode(USAGE_MODE_NORMAL);
          break;
      
        case workrave::networking::UsageMode::USAGE_MODE_READING:
          core->set_usage_mode(USAGE_MODE_READING);
          break;
          
        default:
          break;
        }
    }
  TRACE_EXIT();
}


//! Process Core event
void
Networking::on_timer_message(Message::Ptr message, MessageContext::Ptr context)
{
  TRACE_ENTER("Networking::on_timer_message");

  boost::shared_ptr<workrave::networking::Timer> b = boost::dynamic_pointer_cast<workrave::networking::Timer>(message);

  if (b)
    {
      const UUID &remote_id = context->source;
      const google::protobuf::RepeatedField<int> idle_times = b->idle_times();
      const google::protobuf::RepeatedField<int> active_times = b->active_times();

      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          IBreak::Ptr b = core->get_break((BreakId)i);

          time_t old_idle = b->get_elapsed_idle_time();
          time_t old_active = b->get_elapsed_time();

          int idle = idle_times.Get(i);
          int active = active_times.Get(i);

          TRACE_MSG("Timer " << i <<
                    " idle " <<  idle << " " << old_idle <<
                    " active" << active << " " << old_active); 

          time_t remote_active_since = 0;
          bool remote_active = activity_monitor->is_remote_active(remote_id, remote_active_since);
      
          if (abs((int)(idle - old_idle)) >= 2 ||
              abs((int)(active - old_active)) >= 2 ||
              /* Remote party is active, and became active after us */
              (remote_active && remote_active_since > activity_monitor->get_local_active_since()))
            {
              //timer->set_state(active, idle);
            }
        }
    }

  TRACE_EXIT();
}


//! Broadcast current timer state.
void
Networking::send_timer_state()
{
  if (activity_monitor->is_local_active() &&
      (TimeSource::get_monotonic_time() % 10 == 0))
    {

      boost::shared_ptr<workrave::networking::Timer> a(new workrave::networking::Timer());
  
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          IBreak::Ptr b = core->get_break((BreakId)i);

          a->add_idle_times((int)b->get_elapsed_idle_time());
          a->add_active_times((int)b->get_elapsed_time());
        }

      cloud->send_message(a, MessageParams::create());
    }
}
