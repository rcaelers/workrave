// Control.cc --- The main controller
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"

#include "debug.hh"
#include <assert.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "Control.hh"

#include "Util.hh"
#include "ActivityMonitor.hh"
#include "TimerActivityMonitor.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#include "IdleLogManager.hh"
#include "PacketBuffer.hh"
#endif

#ifndef NDEBUG
#include "FakeActivityMonitor.hh"
#endif

const char *WORKRAVESTATE="WorkRaveState";
const int SAVESTATETIME = 15;


//! Constructor
Control::Control() :
#ifdef HAVE_DISTRIBUTION
  dist_manager(NULL),
  monitor_state(ACTIVITY_UNKNOWN),
  remote_state(ACTIVITY_IDLE),
#  ifndef NDEBUG
  script_count(-1),
  script_start_time(-1),
#  endif
#endif
#ifndef NDEBUG
  fake_monitor(NULL),
#endif  
  timer_count(0),
  timers(NULL),
  configurator(NULL),
  monitor(NULL),
  last_process_time(0),
  master_node(true)
{
  current_time = time(NULL);
}


//! Destructor.
Control::~Control()
{
  TRACE_ENTER("Control::~Control");

  if (monitor != NULL)
    {
      monitor->terminate();
    }

  for (int i = 0; i < timer_count; i++)
    {
      delete timers[i];
    }
  delete [] timers;
  
#ifdef HAVE_DISTRIBUTION
  delete dist_manager;
#endif

  delete monitor;
  
  TRACE_EXIT();
}


void
Control::init(int timer_count, Configurator *config, char *display_name)
{
  this->configurator = config;
  this->timer_count = timer_count;
  this->timers = new Timer*[timer_count];

  for (int i = 0; i < timer_count; i++)
    {
      timers[i] = NULL;
    }
  
#ifdef HAVE_DISTRIBUTION
  create_distribution_manager();
#endif  
  create_monitor(display_name);
}


time_t
Control::get_time() const
{
  return current_time;
}


void
Control::process_timers(TimerInfo *infos)
{
  TRACE_ENTER("Control::process_timers");
  static int count = 0;

#ifndef NDEBUG  
  if (script_start_time != -1 && current_time >= script_start_time)
    {
      do_script();
    }
#endif
  
  // Retrieve State.
  ActivityState state = monitor->get_current_state();
#ifndef NDEBUG
  if (fake_monitor != NULL)
    {
      state = fake_monitor->get_current_state();
    }
#endif  

#ifdef HAVE_DISTRIBUTION  

  // Request master status if we are active.
  bool new_master_node = true;
  if (dist_manager != NULL)
    {
      dist_manager->heartbeart();
      dist_manager->set_lock_master(state == ACTIVITY_ACTIVE);
      new_master_node = dist_manager->is_master();

      if (!new_master_node && state == ACTIVITY_ACTIVE)
        {
          new_master_node = dist_manager->claim();
        }
    }

  
  // Enable or disable timers if we became or lost being master
  if (master_node != new_master_node)
    {
      master_node = new_master_node;
#if 0      
      // Enable/Disable timers.
      for (int i = 0; i < timer_count; i++)
        {
          if (master_node && infos[i].enabled)
            {
              timers[i]->enable();
            }
          else
            {
              timers[i]->disable();
            }
        }
#endif
    }

  // Distribute monitor state if we are master and the
  // state has changed.
  if (master_node && monitor_state != state)
    {
      PacketBuffer state_packet;
      state_packet.create();

      state_packet.pack_ushort(1);
      state_packet.pack_ushort(state);
      
      dist_manager->broadcast_client_message(DCM_MONITOR,
                               (unsigned char *)state_packet.get_buffer(),
                               state_packet.bytes_written());
    }
  
  monitor_state = state;

  // 
  if (!master_node)
    {
      state = remote_state;
    }

  TRACE_MSG(master_node << " " << state << " " << monitor_state << " " << remote_state);

  // Update our idle history.
  idlelog_manager->update_all_idlelogs(dist_manager->get_master_id(), state);
  
#endif // HAVE_DISTRIBUTION
  
  // Timers
  current_time = time(NULL);

  if (last_process_time != 0)
    {
      int gap = current_time - 1 - last_process_time;
      if (abs(gap) > 5)
        {
          TRACE_MSG("Time warp of " << gap << " seconds");
          
          monitor->force_idle();
          monitor->shift_time(gap);
          for (int i = 0; i < timer_count; i++)
            {
              timers[i]->shift_time(gap);
            }
          
          state = ACTIVITY_IDLE;
        }
    }

  
#if 0
  if (master_node)
#endif    
    {
      for (int i = 0; i < timer_count; i++)
        {
          if (infos[i].enabled)
            {
              timers[i]->enable();
              if (!(timers[i]->has_activity_monitor()))
                {
                  timers[i]->process(state, infos[i]);
                }
            }
          else
            {
              timers[i]->disable();
            }
        }

      for (int i = 0; i < timer_count; i++)
        {
          if (infos[i].enabled && timers[i]->has_activity_monitor())
            {
              timers[i]->process(state, infos[i]);
            }
        }
    }
  
  if (count % SAVESTATETIME == 0)
    {
      save_state();
    }

  last_process_time = current_time;
  
  count++;
  TRACE_EXIT();
}


#ifdef HAVE_DISTRIBUTION
// Create the monitor based on the specified configuration.
bool
Control::create_distribution_manager()
{
  dist_manager = DistributionManager::get_instance();

  if (dist_manager != NULL)
    {
      dist_manager->init(configurator);
      dist_manager->register_client_message(DCM_TIMERS, DCMT_MASTER, this);
      dist_manager->register_client_message(DCM_MONITOR, DCMT_MASTER, this);
      dist_manager->register_client_message(DCM_IDLELOG, DCMT_SIGNON, this);
#ifndef NDEBUG
      dist_manager->register_client_message(DCM_SCRIPT, DCMT_PASSIVE, this);
#endif      
      dist_manager->add_listener(this);

      
      idlelog_manager = new IdleLogManager(dist_manager->get_my_id(), this);
    }
  return dist_manager != NULL;
}
#endif


// Create the monitor based on the specified configuration.
bool
Control::create_monitor(char *display_name)
{
#ifndef NDEBUG
  fake_monitor = NULL;
  const char *x = getenv("WORKRAVE_FAKE");
  if (x != NULL)
    {
      fake_monitor = new FakeActivityMonitor();
    }
#endif

  monitor = new ActivityMonitor(display_name);
  load_monitor_config();
  
  configurator->add_listener(CFG_KEY_MONITOR, this);
  
  return true;
}


Timer *
Control::get_timer(string id)
{
  Timer *timer = NULL;
  
  for (int i = 0; i < timer_count; i++)
    {
      if (timers[i]->get_id() == id)
        {
          timer = timers[i];
          break;
        }
    }
  return timer;
}


// Initialize the timers based on the specified configuration. 
void
Control::init_timers()
{
  load_state();

  for (int i = 0; i < timer_count; i++)
    {
      configure_timer_monitor(timers[i]);
      timers[i]->enable();
    }

  configurator->add_listener(CFG_KEY_TIMERS, this);
}

void
Control::daily_reset()
{
#ifdef HAVE_DISTRIBUTION
  idlelog_manager->reset();
#endif
}
  

TimerInterface *
Control::create_timer(int timer_id, string name)
{
  Timer *timer = NULL;
  
  if (timer_id >= 0 && timer_id < timer_count)
    {
      timer = new Timer(this);
      timer->set_id(name);
      
      configure_timer(timer);
      configure_timer_monitor(timer);
      
      timers[timer_id] = timer;
    }
  
  return timer;
}


void
Control::configure_timer(Timer *timer)
{
  string prefix = CFG_KEY_TIMER + timer->get_id();

  int limit = 0;
  int autoreset = 0;
  string resetPred, icon;
  int snooze = 0;
  bool countActivity = 0;
  
  configurator->get_value(prefix + CFG_KEY_TIMER_LIMIT, &limit);
  timer->set_limit(limit);
  timer->set_limit_enabled(limit > 0);

  configurator->get_value(prefix + CFG_KEY_TIMER_AUTO_RESET, &autoreset);
  timer->set_auto_reset(autoreset);
  timer->set_auto_reset_enabled(autoreset > 0);

  if (configurator->get_value(prefix + CFG_KEY_TIMER_RESET_PRED, &resetPred))
    {
      timer->set_auto_reset(resetPred);
    }

  configurator->get_value(prefix + CFG_KEY_TIMER_SNOOZE, &snooze);
  timer->set_snooze_interval(snooze);

  configurator->get_value(prefix + CFG_KEY_TIMER_COUNT_ACTIVITY, &countActivity);
  timer->set_for_activity(countActivity);
}


void
Control::configure_timer_monitor(Timer *timer)
{
  string monitor_name;
  bool ret = configurator->get_value(CFG_KEY_TIMER + timer->get_id() + CFG_KEY_TIMER_MONITOR, &monitor_name);
  if (ret && monitor_name != "")
    {
      TimerInterface *master = get_timer(monitor_name);
      if (master != NULL)
        {
          TimerActivityMonitor *am = new TimerActivityMonitor(master);
          timer->set_activity_monitor(am);
        }
    }
  else
    {
      timer->set_activity_monitor(NULL);
    }
}


void
Control::load_monitor_config()
{
  int noise;
  int activity;
  int idle;

  assert(configurator != NULL);
  assert(monitor != NULL);
  
  if (! configurator->get_value(CFG_KEY_MONITOR_NOISE, &noise))
      noise = 9000;
  if (! configurator->get_value(CFG_KEY_MONITOR_ACTIVITY, &activity))
    activity = 1000;
  if (! configurator->get_value(CFG_KEY_MONITOR_IDLE, &idle))
    idle = 5000;
  
  if (noise < 50)
    noise *= 1000;
  
  if (activity < 50)
    activity *= 1000;
  
  if (idle < 50)
    idle *= 1000;

  monitor->set_parameters(noise, activity, idle);
}


void
Control::store_monitor_config()
{
  int noise = 9000;
  int activity = 1000;
  int idle = 5000;

  assert(configurator != NULL);
  assert(monitor != NULL);

  monitor->get_parameters(noise, activity, idle);
  
  configurator->set_value(CFG_KEY_MONITOR_NOISE, noise);
  configurator->set_value(CFG_KEY_MONITOR_ACTIVITY, activity);
  configurator->set_value(CFG_KEY_MONITOR_IDLE, idle);
  
  configurator->save();
}


void
Control::config_changed_notify(string key)
{
  std::string::size_type pos = key.find('/');

  string path;
  string timer_id;
  
  if (pos != std::string::npos)
    {
      path = key.substr(0, pos);
      key = key.substr(pos + 1);
    }

  if (path == CFG_KEY_TIMERS)
    {
      pos = key.find('/');

      if (pos != std::string::npos)
        {
          timer_id = key.substr(0, pos);
          key = key.substr(pos + 1);
        }
 
      if (timer_id != "")
        {
          for (int i = 0; i < timer_count; i++)
            {
              if (timers[i]->get_id() == timer_id)
                {
                  configure_timer(timers[i]);
                }
            }
        }
    }
  else if (path == CFG_KEY_MONITOR)
    {
      load_monitor_config();
    }
}

void
Control::save_state() const
{
  stringstream ss;
  ss << Util::get_home_directory();
  ss << "state" << ends;

  ofstream stateFile(ss.str().c_str());

  stateFile << "WorkRaveState 2"  << endl
            << get_time() << endl;
  
  for (int i = 0; i < timer_count; i++)
    {
      string stateStr = timers[i]->serialize_state();

      stateFile << stateStr << endl;
    }

  stateFile.close();
}


void
Control::load_state()
{
  stringstream ss;
  ss << Util::get_home_directory();
  ss << "state" << ends;

  ifstream stateFile(ss.str().c_str());

  bool ok = stateFile;
  
  if (ok)
    {
      string tag;
      stateFile >> tag;

      ok = (tag == WORKRAVESTATE);
    }

  if (ok)
    {
      int version;
      stateFile >> version;

      ok = (version == 1 || version == 2);
    }

  if (ok)
    {
      time_t saveTime;
      stateFile >> saveTime;
    }
  
  while (ok && !stateFile.eof())
    {
      string id;
      stateFile >> id;

      for (int i = 0; i < timer_count; i++)
        {
          if (timers[i]->get_id() == id)
            {
              string state;
              getline(stateFile, state);

              timers[i]->deserialize_state(state);
              break;
            }
        }
    }
}


#ifndef NDEBUG
void
Control::test_me()
{
  TRACE_ENTER("Control::test_me");

#if 0  
  script_count = 0;
  script_start_time = current_time + 5;

  PacketBuffer state_packet;
  state_packet.create();

  state_packet.pack_ushort(1);
  state_packet.pack_ushort(SCRIPT_START);
  state_packet.pack_ushort(script_start_time);

  dist_manager->broadcast_client_message(DCM_SCRIPT,
                                         (unsigned char *)state_packet.get_buffer(),
                                         state_packet.bytes_written());
#else  
  if (fake_monitor != NULL)
    {
      ActivityState state = fake_monitor->get_current_state();

      if (state == ACTIVITY_ACTIVE)
        {
          TRACE_MSG("Setting idle");
          fake_monitor->set_state(ACTIVITY_IDLE);
        }
      else
        {
          TRACE_MSG("Setting master");
          fake_monitor->set_state(ACTIVITY_ACTIVE);
        }
    }
#endif
  TRACE_EXIT();
}
#endif


#ifdef HAVE_DISTRIBUTION
bool
Control::request_client_message(DistributionClientMessageID id, unsigned char **buffer, int *size)
{
  bool ret = false;
  
  switch (id)
    {
    case DCM_TIMERS:
      ret = get_timer_state(buffer, size);
      break;
        
    case DCM_MONITOR:
      ret = true;
      break;

    case DCM_IDLELOG:
      ret = idlelog_manager->get_idlelog(buffer, size);
      break;

    default:
      break;
    }

  return ret;
}

bool
Control::client_message(DistributionClientMessageID id, bool master, char *client_id, unsigned char *buffer, int size)
{
  bool ret = false;
  
  switch (id)
    {
    case DCM_TIMERS:
      ret = set_timer_state(buffer, size);
      break;
        
    case DCM_MONITOR:
      ret = set_monitor_state(master, buffer, size);
      break;

    case DCM_IDLELOG:
      ret = idlelog_manager->set_idlelog(buffer, size);
      compute_timers();
      break;
      
#ifndef NDEBUG
    case DCM_SCRIPT:
      ret = script_message(master, client_id, buffer, size);
      break;
#endif
    default:
      break;
    }

  return ret;
}


bool
Control::get_timer_state(unsigned char **buffer, int *size)
{
  TRACE_ENTER("Control::get_timer_state");

  PacketBuffer state_packet;

  state_packet.create();
  state_packet.pack_ushort(timer_count);
  
  for (int i = 0; i < timer_count; i++)
    {
      Timer *t = timers[i];
      state_packet.pack_string(t->get_id().c_str());

      Timer::TimerStateData state_data;
      
      t->get_state_data(state_data);
      
      int pos = state_packet.bytes_written();

      state_packet.pack_ushort(0);
      state_packet.pack_ulong((guint32)state_data.current_time);
      state_packet.pack_ulong((guint32)state_data.elapsed_time);
      state_packet.pack_ulong((guint32)state_data.elapsed_idle_time);
      state_packet.pack_ulong((guint32)state_data.last_pred_reset_time);
      state_packet.pack_ulong((guint32)state_data.total_overdue_time);

      state_packet.pack_ulong((guint32)state_data.last_limit_time);
      state_packet.pack_ulong((guint32)state_data.last_limit_elapsed);
      state_packet.pack_ushort((guint16)state_data.snooze_inhibited);
      
      state_packet.poke_ushort(pos, state_packet.bytes_written() - pos);
    }
  
  // FIXME: solve in PacketBuffer.
  *size = state_packet.bytes_written();
  *buffer = new unsigned char[*size + 1];
  memcpy(*buffer, state_packet.get_buffer(), *size);

  TRACE_EXIT();
  return true;
}


bool
Control::set_timer_state(unsigned char *buffer, int size)
{
  TRACE_ENTER("Control::set_timer_state");

  PacketBuffer state_packet;
  state_packet.create();

  state_packet.pack_raw(buffer, size);
  
  int num_timers = state_packet.unpack_ushort();

  TRACE_MSG("numtimer = " << num_timers);
  for (int i = 0; i < num_timers; i++)
    {
      gchar *id = state_packet.unpack_string();
      TRACE_MSG("id = " << id);

      if (id == NULL)
        {
          TRACE_EXIT();
          return false;
        }
      
      Timer *t = (Timer *)get_timer(id);

      Timer::TimerStateData state_data;

      state_packet.unpack_ushort();
      
      state_data.current_time = state_packet.unpack_ulong();
      state_data.elapsed_time = state_packet.unpack_ulong();
      state_data.elapsed_idle_time = state_packet.unpack_ulong();
      state_data.last_pred_reset_time = state_packet.unpack_ulong();
      state_data.total_overdue_time = state_packet.unpack_ulong();

      state_data.last_limit_time = state_packet.unpack_ulong();
      state_data.last_limit_elapsed = state_packet.unpack_ulong();
      state_data.snooze_inhibited = state_packet.unpack_ushort();
      
      TRACE_MSG("state = "
                << state_data.current_time << " "
                << state_data.elapsed_time << " "
                << state_data.elapsed_idle_time << " "
                << state_data.last_pred_reset_time << " "
                << state_data.total_overdue_time 
                );

      if (t != NULL)
        {
          t->set_state_data(state_data);
        }
    }
  
  TRACE_EXIT();
  return true;
}


bool
Control::set_monitor_state(bool master, unsigned char *buffer, int size)
{
  TRACE_ENTER_MSG("Control::set_monitor_state", master << " " << master_node);

  if (!master_node)
    {
      PacketBuffer state_packet;
      state_packet.create();
      
      state_packet.pack_raw(buffer, size);
  
      state_packet.unpack_ushort();
      remote_state = (ActivityState) state_packet.unpack_ushort();
    }
  
  TRACE_EXIT();
  return true;
}


//! A remote client has signed on.
void
Control::signon_remote_client(string client_id)
{
  idlelog_manager->signon_remote_client(client_id);
}


//! A remote client has signed off.
void
Control::signoff_remote_client(string client_id)
{
  if (client_id == dist_manager->get_master_id())
    {
      remote_state = ACTIVITY_IDLE;
    }

  idlelog_manager->signoff_remote_client(client_id);
}

void
Control::compute_timers()
{
  TRACE_ENTER("IdleLogManager:compute_timers");

  for (int i = 0; i < timer_count; i++)
    {
      int autoreset = timers[i]->get_auto_reset();
      int idle = idlelog_manager->compute_idle_time();
          
      if (autoreset != 0)
        {
          int active_time = idlelog_manager->compute_active_time(autoreset);

          if (idle > autoreset)
            {
              idle = autoreset;
            }
          
          timers[i]->set_values(active_time, idle);
        }
      else
        {
          int active_time = idlelog_manager->compute_total_active_time();
          timers[i]->set_values(active_time, idle);
        }
    }
  
  TRACE_EXIT();
}


#ifndef NDEBUG
bool
Control::script_message(bool master, char *client_id, unsigned char *buffer, int size)
{
  TRACE_ENTER("Control::script_message");

  (void) master;
  (void) client_id;
  
  PacketBuffer packet;
  packet.create(size);

  packet.pack_raw(buffer, size);
  
  int cmd_size = packet.unpack_ushort();
  for (int i = 0; i < cmd_size; i++)
    {
      ScriptCommand type = (ScriptCommand) packet.unpack_ushort();

      switch (type)
        {
        case SCRIPT_START:
          {
            script_start_time = packet.unpack_ulong();            
            script_count = 0;
          }
          break;
        }
    }

  TRACE_EXIT();
  return true;
}

void
Control::do_script()
{
  string id = dist_manager->get_my_id();
  PacketBuffer packet;

  if (id == "192.168.0.42:2701")
    {
      if (script_count == 0)
        {
          fake_monitor->set_state(ACTIVITY_ACTIVE);      
        }
      else if (script_count == 5)
        {
          fake_monitor->set_state(ACTIVITY_IDLE);
        }
      else if (script_count == 40)
        {
          fake_monitor->set_state(ACTIVITY_ACTIVE);      
        }
      else if (script_count == 50)
        {
          fake_monitor->set_state(ACTIVITY_IDLE);
          dist_manager->disconnect("192.168.0.42:2702");
        }
      else if (script_count == 85)
        {
          dist_manager->connect("tcp://192.168.0.42:2702");
        }
    }
  else if (id == "192.168.0.42:2702")
    {
      if (script_count == 51)
        {
          fake_monitor->set_state(ACTIVITY_ACTIVE);
        }
      else if (script_count == 61)
        {
          fake_monitor->set_state(ACTIVITY_IDLE);
        }
      else if (script_count == 71)
        {
          fake_monitor->set_state(ACTIVITY_ACTIVE);
        }
      else if (script_count == 81)
        {
          fake_monitor->set_state(ACTIVITY_IDLE);
        }
    }

  script_count++;
}

#endif // NDEBUG
#endif // HAVE_DISTRIBUTION

