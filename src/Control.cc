// Control.cc --- The main controller
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
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

#ifdef HAVE_GCONF
#include "GConfConfigurator.hh"
#endif

const char *WORKRAVESTATE="WorkRaveState";
const int SAVESTATETIME = 15;

//! Constructor
Control::Control()
{
  current_time = time(NULL);
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

  delete monitor;
  
  TRACE_EXIT();
}


void
Control::init()
{
  create_monitor();
  create_timers();
}


//! The main entry point.
int
Control::main(int argc, char **argv)
{
#ifdef HAVE_GCONF
  bool x =  gconf_init(argc, argv, NULL);
  g_type_init();
#endif

  GUIInterface *gui = GUIFactory::create_gui("gtkmm", this, argc, argv);

  configurator = gui->get_configurator();
  gui->run();

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
  
  ActivityState state = monitor->get_current_state();
          
  current_time = time(NULL);
  TRACE_MSG("run");
          
  for (TimerCIter i = timers.begin(); i != timers.end(); i++)
    {
      TimerInfo info;
      (*i)->process(state, info);

      infos[(*i)->get_id()] = info;
    }

  if (count % SAVESTATETIME == 0)
    {
      save_state();
    }
      
  count++;
}

  
// Create the monitor based on the specified configuration.
bool
Control::create_monitor()
{
  monitor = new ActivityMonitor();
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

  //configurator->set_value(CFG_KEY_TIMER + "micro_pause" + CFG_KEY_TIMER_LIMIT, 30);

  TRACE_EXIT();
}
#endif
