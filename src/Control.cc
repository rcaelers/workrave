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

#ifdef HAVE_DISTRIBUTION
  my_info.idle_history.push_front(IdleInterval(1, current_time));
#endif
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

  // 
  if (!master_node)
    {
      state = ACTIVITY_IDLE;
    }

  TRACE_MSG(master_node << " " << state << " " << monitor_state);

  // Update our idle history.
  update_idle_history(my_info, state, monitor_state != state);

  // Update remote idle history.
  update_remote_idle_history();

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
void
Control::update_remote_idle_history()
{
  TRACE_ENTER("Control::update_remote_idle_history");

  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = i->second;

      // Default: remote client is idle and not master
      ActivityState state = ACTIVITY_IDLE;
      bool is_master = false;

      if (i->first == dist_manager->get_master_id())
        {
          // Correction, remote client IS master.
          state = remote_state;
          is_master = true;
        }

      // Did the state/master status change?
      bool changed = (state != info.last_state || is_master != info.is_master);

      // Update history.
      update_idle_history(info, state, changed);

      // Remember current state/master status.
      info.is_master = is_master;
      info.last_state = state;
    }

  TRACE_EXIT();
}

void
Control::update_idle_history(ClientInfo &info, ActivityState state, bool changed)
{
  TRACE_ENTER_MSG("Control::update_idle_history", ((int)state) << " " << changed);

  IdleInterval *idle = &(info.idle_history.front());

  if (state == ACTIVITY_IDLE)
    {
      if (changed)
        {
          info.update(current_time);
          info.idle_history.push_front(IdleInterval(current_time, current_time));
        }
      else
        {
          idle->idle_end = current_time;
        }
    }
  else if (state == ACTIVITY_ACTIVE)
    {
      if (changed)
        {
          idle->idle_end = current_time;

          int total_idle = idle->idle_end - idle->idle_begin;
          if (total_idle < 1 && info.idle_history.size() > 1)
            {
              // Idle period too short. remove it
              info.idle_history.pop_front();
              idle = &(info.idle_history.front());
            }

          // Update start time of last active period.
          info.last_elapsed = 0;
          info.last_active_begin = current_time;
        }
      else
        {
          info.last_elapsed = current_time - info.last_active_begin;
        }
    }

  TRACE_MSG("last_elapsed " << info.last_elapsed);
  TRACE_MSG("total_elapsed " << info.total_elapsed);
#ifndef WIN32  
  IdleHistoryIter i = info.idle_history.begin();
  while (i != info.idle_history.end())
    {
      IdleInterval &idle = *i;

      struct tm begin_time;
      localtime_r(&idle.idle_begin, &begin_time);
      struct tm end_time;
      localtime_r(&idle.idle_end, &end_time);

      TRACE_MSG(   begin_time.tm_hour << ":"
                   << begin_time.tm_min << ":"
                   << begin_time.tm_sec << " - "
                   << end_time.tm_hour << ":"
                   << end_time.tm_min << ":" 
                   << end_time.tm_sec << " "
                   << idle.elapsed
                   );
      i++;
    }
#endif  
  TRACE_EXIT();
}


int
Control::compute_active_time(int length)
{
  TRACE_ENTER("Control::compute_active_time");

  // Number of client, myself included.
  int size = clients.size() + 1;

  // Data for each client.
  IdleHistoryIter *iterators = new IdleHistoryIter[size];
  IdleHistoryIter *end_iterators = new IdleHistoryIter[size];
  bool *at_end = new bool[size];
  int *elapsed = new int[size];

  // Init data for me.
  my_info.update(current_time);
  iterators[0] = my_info.idle_history.begin();
  end_iterators[0] = my_info.idle_history.end();
  at_end[0] = true;
  elapsed[0] = 0;

  // Init data for remote client.
  int count = 1;
  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = (*i).second;
      
      iterators[count] = info.idle_history.begin();
      end_iterators[count] = info.idle_history.end();
      elapsed[count] = 0;
      at_end[count] = true;

      info.update(current_time);
      count++;
    }


  // Number of simultaneous idle periods.
  int idle_count = 0;

  // Time of last unprocessed event.
  time_t last_time = -1;

  // Iterator of last unprocessed event.
  int last_iter = -1;

  // Stop criterium
  bool stop = false;

  // Begin and End time of idle perdiod.
  time_t begin_time = -1;
  time_t end_time = -1;
  
  while (!stop)
    {
      // Find latest event.
      last_time = -1;
      for (int i = 0; i < size; i ++)
        {
          if (iterators[i] != end_iterators[i])
            {
              IdleInterval ii = *(iterators[i]);
              
              time_t t = at_end[i] ? ii.idle_end : ii.idle_begin;
              
              if (last_time == -1 || t > last_time)
                {
                  last_time = t;
                  last_iter = i;
                }
            }
        }

      // Did we found one?
      if (last_time != -1)
        {
          IdleInterval ii = *(iterators[last_iter]);
#ifndef WIN32          
          struct tm begin;
          localtime_r(&ii.idle_begin, &begin);
          struct tm end;
          localtime_r(&ii.idle_end, &end);
#endif          
          if (at_end[last_iter])
            {
              idle_count++;
              
#ifndef WIN32
              TRACE_MSG("New end " << last_iter << " " 
                        << end.tm_hour << ":"
                        << end.tm_min << ":" 
                        << end.tm_sec << " "
                        << idle_count);
#endif              
              at_end[last_iter] = false;

              end_time = ii.idle_end;
              elapsed[last_iter] += ii.elapsed;
            }
          else
            {
#ifndef WIN32
              TRACE_MSG("Begin " << last_iter << " " 
                        << begin.tm_hour << ":"
                        << begin.tm_min << ":"
                        << begin.tm_sec << "  "
                        << idle_count);
#endif              
              at_end[last_iter] = true;
              iterators[last_iter]++;
              begin_time = ii.idle_begin;
                
              if (idle_count == size)
                {
                  TRACE_MSG("Common idle period of " << (end_time - begin_time));
                  if ((end_time - begin_time) > length)
                    {
                      stop = true;
                    }
                }
              idle_count--;
            }
        }
      else
        {
          stop = true;
        }
    }

  int total_elapsed = 0;
  for (int i = 0; i < size; i++)
    {
      TRACE_MSG(i << " " << elapsed[i]);
      total_elapsed += elapsed[i];
    }

  TRACE_MSG("total = " << total_elapsed);
  
  delete [] iterators;
  delete [] end_iterators;
  delete [] at_end;

  TRACE_EXIT();
  return total_elapsed;
}


int
Control::compute_idle_time()
{
  TRACE_ENTER("Control::compute_idle_time");

  my_info.update(current_time);
  IdleInterval &idle = my_info.idle_history.front();

  int count = idle.elapsed == 0 ? 1 : 0;
  time_t latest_start_time = idle.idle_begin;

  TRACE_MSG(current_time - latest_start_time);
  TRACE_MSG(latest_start_time);
  
  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = (*i).second;
      IdleInterval &idle = info.idle_history.front();

      info.update(current_time);
      if (idle.elapsed == 0)
        {
          count++;
        }
      if (idle.idle_begin > latest_start_time)
        {
          latest_start_time = idle.idle_begin;
          TRACE_MSG(current_time - latest_start_time);
        }
    }

  TRACE_MSG("count = " << count);
  if (count != clients.size() + 1)
    {
      latest_start_time = current_time;
    }

  TRACE_MSG((current_time - latest_start_time));
  TRACE_EXIT();
  
  return current_time - latest_start_time;
}


void
Control::save_idle_log(ClientInfo &info)
{
}


void
Control::load_idle_log(ClientInfo &info, string filename)
{
}


void
Control::load_all_idle_logs()
{
}


void
Control::update_idle_log(ClientInfo &info,  IdleInterval)
{
}


void
Control::compute_timers()
{
  TRACE_ENTER("Control:compute_timers");
  for (int i = 0; i < timer_count; i++)
    {
      int autoreset = timers[i]->get_auto_reset();

      int idle = compute_idle_time();
          
      if (autoreset != 0)
        {
          int elapsed = compute_active_time(autoreset);

          if (idle > autoreset)
            {
              idle = autoreset;
            }
          
          timers[i]->set_values(elapsed, idle);
        }
      else
        {
          my_info.update(current_time);

          int elapsed = my_info.total_elapsed;

          TRACE_MSG(elapsed << " " << idle);
          for (ClientMapIter it = clients.begin(); it != clients.end(); it++)
            {
              ClientInfo &info = (*it).second;
              info.update(current_time);
              elapsed += info.total_elapsed;
              TRACE_MSG(elapsed);
            }

          timers[i]->set_values(elapsed, idle);
        }
    }
  TRACE_EXIT();
}

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
  my_info.update(current_time);
  my_info.total_elapsed = 0;

  for (ClientMapIter i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &info = (*i).second;
      info.update(current_time);
      info.total_elapsed = 0;
    }
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
      ret = get_monitor_state(buffer, size);
      break;

    case DCM_IDLELOG:
      ret = get_idlelog_state(buffer, size);
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
      ret = set_timer_state(master, client_id, buffer, size);
      break;
        
    case DCM_MONITOR:
      ret = set_monitor_state(master, client_id, buffer, size);
      break;

    case DCM_IDLELOG:
      ret = set_idlelog_state(master, client_id, buffer, size);
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
Control::set_timer_state(bool master, char *client_id, unsigned char *buffer, int size)
{
  (void) client_id;
  (void) master;
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
Control::get_monitor_state(unsigned char **buffer, int *size)
{
  TRACE_ENTER("Control::get_monitor_state");
  (void) buffer;
  (void) size;
  TRACE_EXIT();
  return true;
}


bool
Control::set_monitor_state(bool master, char *client_id, unsigned char *buffer, int size)
{
  TRACE_ENTER_MSG("Control::set_monitor_state", master << " " << master_node);

  (void) client_id;
  (void) buffer;
  (void) size;
  
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


bool
Control::get_idlelog_state(unsigned char **buffer, int *size)
{
  TRACE_ENTER("Control::get_idlelog_state");

  my_info.update(current_time);
    
  PacketBuffer state_packet;

  int idle_size = my_info.idle_history.size();
  state_packet.create(1024 + idle_size * 16); // FIXME: 

  state_packet.pack_ushort(idle_size);
  state_packet.pack_string(dist_manager->get_my_id().c_str());
  state_packet.pack_ulong(my_info.total_elapsed);
  state_packet.pack_ulong(current_time);
  state_packet.pack_byte(master_node);
  state_packet.pack_byte(master_node ? monitor_state : ACTIVITY_IDLE);

  IdleHistoryIter i = my_info.idle_history.begin();
  while (i != my_info.idle_history.end())
    {
      IdleInterval &idle = *i;

      int elapsed = idle.elapsed;

      int pos = state_packet.bytes_written();
      state_packet.pack_ushort(0);

      state_packet.pack_ulong((guint32)idle.idle_begin);
      state_packet.pack_ulong((guint32)idle.idle_end);
      state_packet.pack_ulong((guint32)elapsed);
      
      state_packet.poke_ushort(pos, state_packet.bytes_written() - pos);
      i++;
    }


  *size = state_packet.bytes_written();
  *buffer = new unsigned char[*size + 1];
  memcpy(*buffer, state_packet.get_buffer(), *size);

  TRACE_EXIT();
  return true;
}


bool
Control::set_idlelog_state(bool master, char *client_id, unsigned char *buffer, int size)
{
  TRACE_ENTER("Control::set_idlelog_state");

  (void) master;
  (void) client_id;
  
  PacketBuffer state_packet;
  state_packet.create(size);

  state_packet.pack_raw(buffer, size);
  
  int num_idle = state_packet.unpack_ushort();
  gchar *id = state_packet.unpack_string();
  int total_elapsed = state_packet.unpack_ulong();
  time_t time_diff = state_packet.unpack_ulong() - current_time;
    
  ClientInfo info;
  info.is_master = (bool) state_packet.unpack_byte();
  info.last_state = (ActivityState) state_packet.unpack_byte();
  info.total_elapsed = total_elapsed;
  clients[id] = info;

  TRACE_MSG("id = " << id <<
            " numidle = " << num_idle <<
            " master  = " << info.is_master <<
            " state = " << info.last_state);

  for (int i = 0; i < num_idle; i++)
    {
      state_packet.unpack_ushort();

      IdleInterval idle;
      
      idle.idle_begin = state_packet.unpack_ulong() - time_diff;
      idle.idle_end = state_packet.unpack_ulong() - time_diff;
      idle.elapsed = state_packet.unpack_ulong();

      TRACE_MSG(id << " " << idle.idle_begin << " " << idle.idle_end << " " << idle.elapsed);
      clients[id].idle_history.push_back(idle);
    }

#ifndef WIN32  
  IdleHistoryIter i = clients[id].idle_history.begin();
  while (i != clients[id].idle_history.end())
    {
      IdleInterval &idle = *i;

      struct tm begin_time;
      localtime_r(&idle.idle_begin, &begin_time);
      struct tm end_time;
      localtime_r(&idle.idle_end, &end_time);

      TRACE_MSG(   begin_time.tm_hour << ":"
                   << begin_time.tm_min << ":"
                   << begin_time.tm_sec << " - "
                   << end_time.tm_hour << ":"
                   << end_time.tm_min << ":" 
                   << end_time.tm_sec << " "
                   << idle.elapsed
                   );
      i++;
    }
#endif
  compute_timers();
  TRACE_EXIT();
  return true;
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
  TRACE_MSG("size = " << cmd_size);

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

#endif

//! A remote client has signed on.
void
Control::signon_remote_client(string client_id)
{
  TRACE_ENTER_MSG("signon_remote_client", client_id);

  ClientInfo info;
  clients[client_id] = info;
  clients[client_id].idle_history.push_front(IdleInterval(1, current_time));

  TRACE_EXIT();
}


//! A remote client has signed off.
void
Control::signoff_remote_client(string client_id)
{
  TRACE_ENTER_MSG("signon_remote_client", client_id);

  clients[client_id].last_state = ACTIVITY_IDLE;

  string master_id = dist_manager->get_master_id();
  if (master_id == client_id)
    {
      TRACE_MSG("Master signed off. Setting idle.");
      remote_state = ACTIVITY_IDLE;
      clients[client_id].is_master = false;
    }

  TRACE_EXIT();
}
#endif

