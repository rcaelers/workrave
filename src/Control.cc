// Control.cc --- The main controller
//
// Copyright (C) 2001, 2002 Rob Caelers & Raymond Penners
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

#include "Util.hh"
#include "Control.hh"
#include "GUIFactory.hh"

#include "ActivityMonitor.hh"
#include "TimerActivityMonitor.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#include "PacketBuffer.hh"
#endif

#ifdef HAVE_GCONF
#include "GConfConfigurator.hh"
#endif

#ifndef NDEBUG
#include "FakeActivityMonitor.hh"
#endif

#ifdef ENABLE_NLS
// I know, this is not very nice, but the documentation says
// gtk_set_locale & co need to be invoked at the start of the program,
// which is here.
#include <gtk/gtk.h>
#endif

const char *WORKRAVESTATE="WorkRaveState";
const int SAVESTATETIME = 15;

//! Constructor
Control::Control()
{
  current_time = time(NULL);
#ifdef HAVE_DISTRIBUTION
  dist_manager = NULL;
#endif  
  master_node = true;
  configurator = NULL;
  monitor = NULL;
}


//! Destructor.
Control::~Control()
{
  TRACE_ENTER("Control::~Control");
  
  monitor->terminate();

  TimerCIter ti = timers.begin();
  while (ti != timers.end())
    {
      delete *ti;
      ti++;
    }

#ifdef HAVE_DISTRIBUTION
  if (dist_manager != NULL)
    {
      delete dist_manager;
    }
#endif
  
  delete monitor;
  
  TRACE_EXIT();
}


void
Control::init(char *display_name)
{
#ifdef HAVE_DISTRIBUTION
  create_distribution_manager();
#endif  
  create_monitor(display_name);
  create_timers();
}


//! The main entry point.
int
Control::main(int argc, char **argv)
{
  // I know, this is not very nice, but the documentation says
  // gtk_set_locale & co need to be invoked at the start of the program,
  // which is here.
#ifdef ENABLE_NLS
#  ifndef HAVE_GNOME 
  gtk_set_locale();
#  endif
  const char *locale_dir;
#ifdef WIN32
  string dir = Util::get_application_directory() + "\\lib\\locale";
  locale_dir = dir.c_str();
#else
  locale_dir = GNOMELOCALEDIR;
#endif
  bindtextdomain(PACKAGE, locale_dir);
  textdomain(PACKAGE);
#endif
  
#ifdef HAVE_GCONF
  bool x =  gconf_init(argc, argv, NULL);
  g_type_init();
#endif

  GUIInterface *gui = GUIFactory::create_gui("gtkmm", this, argc, argv);

  configurator = gui->get_configurator();

  gui->run();

  //FIXME: untested, solves memleak, gui not freed.
  delete gui;
  
  return 0;
}


time_t
Control::get_time() const
{
  return current_time;
}


list<string>
Control::get_timer_ids() const
{
  list<string> ret;

  for (TimerCIter i = timers.begin(); i != timers.end(); i++)
    {
      ret.push_back((*i)->get_id());
    }
  return ret;
}
  

TimerInterface *
Control::get_timer(string id)
{
  TimerInterface *timer_itf = NULL;
  
  for (TimerCIter i = timers.begin(); i != timers.end(); i++)
    {
      if ((*i)->get_id() == id)
        {
          timer_itf = *i;
          break;
        }
    }
  return timer_itf;
}


void
Control::process_timers(map<string, TimerInfo> &infos)
{
  TRACE_ENTER("Control::process_timers");
  static int count = 0;

  // Retrieve State.
  
  ActivityState state = monitor->get_current_state();
#ifndef NDEBUG
  if (fake_monitor != NULL)
    {
      state = fake_monitor->get_current_state();
    }
#endif  


  // Distributed  stuff
#ifdef HAVE_DISTRIBUTION  
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

  if (master_node != new_master_node)
    {
      master_node = new_master_node;
      // Enable/Disable timers.
      for (TimerCIter i = timers.begin(); i != timers.end(); i++)
        {
          if (master_node)
            {
              (*i)->enable();
            }
          else
            {
              (*i)->disable();
            }
        }
    }
#endif
  
  // Stats
  ActivityMonitorStatistics stats;
  monitor->get_statistics(stats);

  int ratio = 0;
  if (stats.total_movement != 0)
    {
      ratio = (stats.total_click_movement * 100) / stats.total_movement;
    }
  
  TRACE_MSG("monitor stats "
            << stats.total_movement << " " 
            << stats.total_click_movement << " "
            << ratio << " "
            << stats.total_movement_time << " "
            << stats.total_clicks << " "
            << stats.total_keystrokes
            );
  
  // Timers
  current_time = time(NULL);

  if (master_node)
    {
      // FIXME: quick solution for dependancy probleem if a timer
      // uses a private activity monitor...
  
      for (TimerCIter i = timers.begin(); i != timers.end(); i++)
        {
          if (!(*i)->has_activity_monitor())
            {
              TimerInfo info;
              (*i)->process(state, info);

              infos[(*i)->get_id()] = info;
            }
        }

      for (TimerCIter i = timers.begin(); i != timers.end(); i++)
        {
          if (((*i)->has_activity_monitor()))
            {
              TimerInfo info;
              (*i)->process(state, info);
          
              infos[(*i)->get_id()] = info;
            }
        }
    }

  
  if (count % SAVESTATETIME == 0)
    {
      save_state();
    }
      
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
      dist_manager->register_state(DISTR_STATE_TIMERS,  this);
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
  store_monitor_config();
  
  configurator->add_listener(CFG_KEY_MONITOR, this);
  
  return true;
}


  // Create the timers based on the specified configuration. 
bool
Control::create_timers()
{
  TRACE_ENTER("Control::create_timers");

  list<string> timer_names = configurator->get_all_dirs(CFG_KEY_TIMERS);
  for (list<string>::iterator i = timer_names.begin(); i != timer_names.end(); i++)
    {
      TRACE_MSG("ID=" << *i);
      Timer *timer = new Timer(this);
      
      timer->set_id(*i);
      configure_timer(*i, timer);

      timers.push_back(timer);
    }
  
  load_state();

  for (TimerCIter i = timers.begin(); i != timers.end(); i++)
    {
      string monitor_name;
      configure_timer_monitor((*i)->get_id(), *i);
      (*i)->enable();
    }

  configurator->add_listener(CFG_KEY_TIMERS, this);
  
  TRACE_EXIT();
  return true;
}


void
Control::configure_timer(string id, Timer *timer)
{
  TRACE_ENTER_MSG("Control::configure_timer", id);
  assert(timer != NULL);
  
  string prefix = CFG_KEY_TIMER + id;

  int limit = 0;
  int autoreset = 0;
  string resetPred, icon;
  int snooze = 0;
  bool countActivity = 0;
  bool restore = 0;
  
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

  // TODO: not used...yet
  configurator->get_value(prefix + CFG_KEY_TIMER_RESTORE, &restore);

  configure_timer_monitor(id, timer);
  TRACE_EXIT();
}


void
Control::configure_timer_monitor(string id, Timer *timer)
{
  TRACE_ENTER_MSG("Control::configure_timer_monitor", id);
  string monitor_name;
  bool ret = configurator->get_value(CFG_KEY_TIMER + id + CFG_KEY_TIMER_MONITOR, &monitor_name);
  if (ret && monitor_name != "")
    {
      TimerInterface *master = get_timer(monitor_name);
      if (master != NULL)
        {
          TRACE_MSG("Setting master activity monitor for " << id << " to " << monitor_name);
          TimerActivityMonitor *am = new TimerActivityMonitor(master);
          timer->set_activity_monitor(am);
        }
    }
  else
    {
      timer->set_activity_monitor(NULL);
    }
  TRACE_EXIT();
}

void
Control::load_monitor_config()
{
  int noise = 9000;
  int activity = 1000;
  int idle = 5000;

  assert(configurator != NULL);
  assert(monitor != NULL);
  
  configurator->get_value(CFG_KEY_MONITOR_NOISE, &noise);
  configurator->get_value(CFG_KEY_MONITOR_ACTIVITY, &activity);
  configurator->get_value(CFG_KEY_MONITOR_IDLE, &idle);
  
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
  TRACE_ENTER_MSG("Control:config_changed_notify", key);
  
  std::string::size_type pos = key.find('/');

  string path;
  string timerId;
  
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
          timerId = key.substr(0, pos);
          key = key.substr(pos + 1);
        }
 
      if (timerId != "")
        {
          for (TimerCIter i = timers.begin(); i != timers.end(); i++)
            {
              if ((*i)->get_id() == timerId)
                {
                  configure_timer(timerId, (*i));
                }
            }
        }
    }
  else if (path == CFG_KEY_MONITOR)
    {
      load_monitor_config();
    }
  
  TRACE_EXIT();
}

void
Control::save_state() const
{
  stringstream ss;
  ss << Util::get_home_directory();
  ss << "state" << ends;

  ofstream stateFile(ss.str().c_str());

  stateFile << "WorkRaveState 1"  << endl
            << get_time() << endl;
  
  for (TimerCIter ti = timers.begin(); ti != timers.end(); ti++)
    {
      string stateStr = (*ti)->serialize_state();

      stateFile << stateStr << endl;
    }

  stateFile.close();
}


void
Control::load_state()
{
  TRACE_ENTER("Control::load_state");
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

      ok = (version == 1);
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

      for (TimerCIter i = timers.begin(); i != timers.end(); i++)
        {
          if ((*i)->get_id() == id)
            {
              string state;
              getline(stateFile, state);

              (*i)->deserialize_state(state);
              break;
            }
        }
    }
  TRACE_EXIT();
}

#ifndef NDEBUG
void
Control::test_me()
{
  TRACE_ENTER("Control::test_me");

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
  
  TRACE_EXIT();
}
#endif

#ifdef HAVE_DISTRIBUTION
bool
Control::get_state(DistributedStateID id, unsigned char **buffer, int *size)
{
  TRACE_ENTER("Control::get_state");

  PacketBuffer state_packet;

  state_packet.create();

  state_packet.pack_ushort(timers.size());
  
  for (TimerCIter i = timers.begin(); i != timers.end(); i++)
    {
      Timer *t = *i;
      state_packet.pack_string(t->get_id().c_str());

      Timer::TimerStateData state_data;
      
      t->get_state_data(state_data);
      
      int pos = state_packet.bytes_written();

      state_packet.pack_ushort(0);
      state_packet.pack_ulong((guint32)state_data.current_time);
      state_packet.pack_ulong((guint32)state_data.elapsed_time);
      state_packet.pack_ulong((guint32)state_data.elapsed_idle_time);
      state_packet.pack_ulong((guint32)state_data.last_pred_reset_time);

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
Control::set_state(DistributedStateID id, bool master, unsigned char *buffer, int size)
{
  TRACE_ENTER("Control::set_state");

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

      int data_size = state_packet.unpack_ushort();
      
      state_data.current_time = state_packet.unpack_ulong();
      state_data.elapsed_time = state_packet.unpack_ulong();
      state_data.elapsed_idle_time = state_packet.unpack_ulong();
      state_data.last_pred_reset_time = state_packet.unpack_ulong();

      TRACE_MSG("state = "
                << state_data.current_time << " "
                << state_data.elapsed_time << " "
                << state_data.elapsed_idle_time << " "
                << state_data.last_pred_reset_time 
                );

      if (t != NULL)
        {
          t->set_state_data(state_data);
        }
    }
  
  TRACE_EXIT();
  return true;
}
#endif
