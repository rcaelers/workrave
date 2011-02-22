// Core.cc --- The main controller
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Rob Caelers & Raymond Penners
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

#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Core.hh"

#include "Util.hh"
#include "IApp.hh"
#include "ICoreEventListener.hh"
#include "ActivityMonitor.hh"
#include "TimerActivityMonitor.hh"
#include "Break.hh"
#include "ConfiguratorFactory.hh"
#include "Configurator.hh"
#include "CoreConfig.hh"
#include "Statistics.hh"
#include "BreakControl.hh"
#include "Timer.hh"
#include "TimePredFactory.hh"
#include "TimePred.hh"
#include "TimeSource.hh"
#include "InputMonitorFactory.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#include "IdleLogManager.hh"
#include "PacketBuffer.hh"
#ifndef NDEBUG
#include "FakeActivityMonitor.hh"
#endif
#endif

#ifdef HAVE_GCONF
#include <gconf/gconf-client.h>
#endif

#ifdef HAVE_DBUS
#if defined(PLATFORM_OS_WIN32_NATIVE)
#undef interface
#endif
#include "DBus.hh"
#include "DBusException.hh"
#ifdef HAVE_TESTS
#include "Test.hh"
#endif
#endif

Core *Core::instance = NULL;

const char *WORKRAVESTATE="WorkRaveState";
const int SAVESTATETIME = 60;

#define DBUS_PATH_WORKRAVE         "/org/workrave/Workrave/Core"
#define DBUS_SERVICE_WORKRAVE      "org.workrave.Workrave"

//! Constructs a new Core.
Core::Core() :
  last_process_time(0),
  master_node(true),
  configurator(NULL),
  monitor(NULL),
  application(NULL),
  statistics(NULL),
  operation_mode(OPERATION_MODE_NORMAL),
  usage_mode(USAGE_MODE_NORMAL),
  core_event_listener(NULL),
  powersave(false),
  powersave_resume_time(0),
  powersave_operation_mode(OPERATION_MODE_NORMAL),
  insist_policy(ICore::INSIST_POLICY_HALT),
  active_insist_policy(ICore::INSIST_POLICY_INVALID),
  resume_break(BREAK_ID_NONE),
  local_state(ACTIVITY_IDLE),
  monitor_state(ACTIVITY_UNKNOWN)
#ifdef HAVE_DISTRIBUTION
  ,
  dist_manager(NULL),
  remote_state(ACTIVITY_IDLE),
  idlelog_manager(NULL)
#  ifndef NDEBUG
  ,
  fake_monitor(NULL)
#  endif
#endif
{
  TRACE_ENTER("Core::Core");
  current_time = time(NULL);

  assert(! instance);
  instance = this;

  TRACE_EXIT();
}


//! Destructor.
Core::~Core()
{
  TRACE_ENTER("Core::~Core");

  save_state();

  if (monitor != NULL)
    {
      monitor->terminate();
    }

  delete statistics;
  delete monitor;
  delete configurator;

#ifdef HAVE_DISTRIBUTION
  if (idlelog_manager != NULL)
    {
      idlelog_manager->terminate();
      delete idlelog_manager;
    }

  delete dist_manager;
#ifndef NDEBUG
  delete fake_monitor;
#endif
#endif

  TRACE_EXIT();
}


/********************************************************************************/
/**** Initialization                                                       ******/
/********************************************************************************/


//! Initializes the core.
void
Core::init(int argc, char **argv, IApp *app, const string &display_name)
{
  application = app;
  this->argc = argc;
  this->argv = argv;

  init_configurator();
  init_monitor(display_name);

#ifdef HAVE_DISTRIBUTION
  init_distribution_manager();
#endif

  init_breaks();
  init_statistics();
  init_bus();

  load_state();
  load_misc();
}


//! Initializes the configurator.
void
Core::init_configurator()
{
  string ini_file = Util::complete_directory("workrave.ini", Util::SEARCH_PATH_CONFIG);

  if (Util::file_exists(ini_file))
    {
      configurator = ConfiguratorFactory::create(ConfiguratorFactory::FormatIni);
      configurator->load(ini_file);
    }
  else
    {
#if defined(PLATFORM_OS_WIN32) || defined(PLATFORM_OS_OSX)
      configurator = ConfiguratorFactory::create(ConfiguratorFactory::FormatNative);

#elif defined(HAVE_GCONF)
      gconf_init(argc, argv, NULL);
      g_type_init();
      configurator = ConfiguratorFactory::create(ConfiguratorFactory::FormatNative);

#elif defined(HAVE_GDOME)
      string configFile = Util::complete_directory("config.xml", Util::SEARCH_PATH_CONFIG);
      configurator = ConfiguratorFactory::create(ConfiguratorFactory::FormatXml);

#  if defined(PLATFORM_OS_UNIX)
      if (configFile == "" || configFile == "config.xml")
        {
          configFile = Util::get_home_directory() + "config.xml";
        }
#  endif
      if (configFile != "")
        {
          configurator->load(configFile);
        }
#elif defined(HAVE_QT)
      configurator = ConfiguratorFactory::create(ConfiguratorFactory::FormatNative);
#else
      ini_file = Util::get_home_directory() + "workrave.ini";
      configurator = ConfiguratorFactory::create(ConfiguratorFactory::FormatIni);
      configurator->load(ini_file);
      configurator->save(ini_file);
#endif
    }

  string home;
  if (configurator->get_value(CoreConfig::CFG_KEY_GENERAL_DATADIR, home))
    {
      Util::set_home_directory(home);
    }
}

//! Initializes the communication bus.
void
Core::init_bus()
{
#ifdef HAVE_DBUS
  try
    {
      dbus = new DBus();

      dbus->init();

      string name = DBUS_SERVICE_WORKRAVE;
      char *env = getenv("WORKRAVE_DBUS_NAME");
      if (env != NULL)
        {
          name = env;
        }      
      dbus->register_service(name);
      dbus->register_object_path(DBUS_PATH_WORKRAVE);

      extern void init_DBusWorkrave(DBus *dbus);
      init_DBusWorkrave(dbus);
  
      dbus->connect(DBUS_PATH_WORKRAVE, "org.workrave.CoreInterface", this);
      
      dbus->connect(DBUS_PATH_WORKRAVE,
                    "org.workrave.ConfigInterface",
                    configurator);

#ifdef HAVE_TESTS
      dbus->register_object_path("/org/workrave/Workrave/Debug");
      dbus->connect("/org/workrave/Workrave/Debug",
                    "org.workrave.DebugInterface",
                    Test::get_instance());
#endif
    }
  catch (DBusException &)
    {
    }
#endif
}


//! Initializes the activity monitor.
void
Core::init_monitor(const string &display_name)
{
#ifdef HAVE_DISTRIBUTION
#ifndef NDEBUG
  fake_monitor = NULL;
  const char *env = getenv("WORKRAVE_FAKE");
  if (env != NULL)
    {
      fake_monitor = new FakeActivityMonitor();
    }
#endif
#endif

  InputMonitorFactory::init(display_name);
  
  monitor = new ActivityMonitor();
  load_monitor_config();

  configurator->add_listener(CoreConfig::CFG_KEY_MONITOR, this);
}


//! Initializes all breaks.
void
Core::init_breaks()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      breaks[i].init(BreakId(i), application);
    }
  application->set_break_response(this);
}


#ifdef HAVE_DISTRIBUTION
//! Initializes the monitor based on the specified configuration.
void
Core::init_distribution_manager()
{
  dist_manager = new DistributionManager();
  assert(dist_manager != NULL);

  dist_manager->init(configurator);
  dist_manager->register_client_message(DCM_BREAKS, DCMT_MASTER, this);
  dist_manager->register_client_message(DCM_TIMERS, DCMT_MASTER, this);
  dist_manager->register_client_message(DCM_MONITOR, DCMT_MASTER, this);
  dist_manager->register_client_message(DCM_IDLELOG, DCMT_SIGNON, this);
  dist_manager->register_client_message(DCM_BREAKCONTROL, DCMT_PASSIVE, this);

  dist_manager->add_listener(this);

  idlelog_manager = new IdleLogManager(dist_manager->get_my_id(), this);
  idlelog_manager->init();
}
#endif


//! Initializes the statistics.
void
Core::init_statistics()
{
  statistics = new Statistics();
  statistics->init(this);
}


//! Loads the configuration of the monitor.
void
Core::load_monitor_config()
{
  TRACE_ENTER("Core::load_monitor_config");

  int noise;
  int activity;
  int idle;

  assert(configurator != NULL);
  assert(monitor != NULL);

  if (! configurator->get_value(CoreConfig::CFG_KEY_MONITOR_NOISE, noise))
    noise = 9000;
  if (! configurator->get_value(CoreConfig::CFG_KEY_MONITOR_ACTIVITY, activity))
    activity = 1000;
  if (! configurator->get_value(CoreConfig::CFG_KEY_MONITOR_IDLE, idle))
    idle = 5000;

  // Pre 1.0 compatibility...
  if (noise < 50)
    {
      noise *= 1000;
      configurator->set_value(CoreConfig::CFG_KEY_MONITOR_NOISE, noise);
    }

  if (activity < 50)
    {
      activity *= 1000;
      configurator->set_value(CoreConfig::CFG_KEY_MONITOR_ACTIVITY, activity);
    }

  if (idle < 50)
    {
      idle *= 1000;
      configurator->set_value(CoreConfig::CFG_KEY_MONITOR_IDLE, idle);
    }

  TRACE_MSG("Monitor config = " << noise << " " << activity << " " << idle);

  monitor->set_parameters(noise, activity, idle);
  TRACE_EXIT();
}


//! Notification that the configuration has changed.
void
Core::config_changed_notify(const string &key)
{
  TRACE_ENTER_MSG("Core::config_changed_notify", key);
  string::size_type pos = key.find('/');
  string path;

  if (pos != string::npos)
    {
      path = key.substr(0, pos);
    }

  if (path == CoreConfig::CFG_KEY_MONITOR)
    {
      load_monitor_config();
    }

  if (key == CoreConfig::CFG_KEY_OPERATION_MODE)
    {
      int mode;
      if (! get_configurator()->get_value(CoreConfig::CFG_KEY_OPERATION_MODE, mode))
        {
          mode = OPERATION_MODE_NORMAL;
        }
      TRACE_MSG("Setting mode");
      set_operation_mode(OperationMode(mode), false);
    }

  if (key == CoreConfig::CFG_KEY_USAGE_MODE)
    {
      int mode;
      if (! get_configurator()->get_value(CoreConfig::CFG_KEY_USAGE_MODE, mode))
        {
          mode = USAGE_MODE_NORMAL;
        }
      set_usage_mode(UsageMode(mode), false);
    }
  TRACE_EXIT();
}


/********************************************************************************/
/**** TimeSource interface                                                 ******/
/********************************************************************************/

//! Retrieve the current time.
time_t
Core::get_time() const
{
  return current_time;
}


/********************************************************************************/
/**** Core Interface                                                       ******/
/********************************************************************************/

//! Returns the specified timer.
Timer *
Core::get_timer(BreakId id) const
{
  if (id >= 0 && id < BREAK_ID_SIZEOF)
    {
      return breaks[id].get_timer();
    }
  else
    {
      return NULL;
    }
}


//! Returns the specified timer.
Timer *
Core::get_timer(string name) const
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (breaks[i].get_name() == name)
        {
          return breaks[i].get_timer();
        }
    }
  return NULL;
}


//! Returns the configurator.
Configurator *
Core::get_configurator() const
{
  return configurator;
}


//! Returns the activity monitor.
IActivityMonitor *
Core::get_activity_monitor() const
{
  return monitor;
}


//! Returns the statistics.
Statistics *
Core::get_statistics() const
{
  return statistics;
}


//! Returns the specified break controller.
Break *
Core::get_break(BreakId id)
{
  assert(id >= 0 && id < BREAK_ID_SIZEOF);
  return &breaks[id];
}

Break *
Core::get_break(std::string name)
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (breaks[i].get_name() == name)
        {
          return &breaks[i];
        }
    }
  return NULL;
}

#ifdef HAVE_DISTRIBUTION
//! Returns the distribution manager.
DistributionManager *
Core::get_distribution_manager() const
{
  return dist_manager;
}
#endif


//! Retrieves the operation mode.
OperationMode
Core::get_operation_mode()
{
  return operation_mode;
}


//! Sets the operation mode
OperationMode
Core::set_operation_mode(OperationMode mode, bool persistent)
{
  TRACE_ENTER_MSG("Core::set_operation_mode",
                  (mode == OPERATION_MODE_NORMAL ? "normal" :
                   mode == OPERATION_MODE_SUSPENDED ? "suspended" :
                   mode == OPERATION_MODE_QUIET ? "quiet" : "???") << " " << persistent);
  
  OperationMode previous_mode = operation_mode;

  TRACE_MSG("Previous " << (mode == OPERATION_MODE_NORMAL ? "normal" :
                            mode == OPERATION_MODE_SUSPENDED ? "suspended" :
                            mode == OPERATION_MODE_QUIET ? "quiet" : "???"));
  
  if (operation_mode != mode)
    {
      operation_mode = mode;
      
      if (operation_mode == OPERATION_MODE_SUSPENDED)
        {
          TRACE_MSG("Force idle");
          force_idle();
          monitor->suspend();
          stop_all_breaks();
          
          for (int i = 0; i < BREAK_ID_SIZEOF; ++i)
            {
              if (breaks[i].is_enabled())
                {
                  breaks[i].get_timer()->set_insensitive_mode(INSENSITIVE_MODE_IDLE_ALWAYS);
                }
            }
        }
      else if (previous_mode == OPERATION_MODE_SUSPENDED)
        {
          // stop_all_breaks again will reset insensitive mode (that is good)
          stop_all_breaks();
          monitor->resume();
        }
      
      if (operation_mode == OPERATION_MODE_QUIET)
        {
          stop_all_breaks();
        }

      if (persistent)
        {
          get_configurator()->set_value(CoreConfig::CFG_KEY_OPERATION_MODE, mode);
        }
      
      if (core_event_listener != NULL)
        {
          core_event_listener->core_event_operation_mode_changed(mode);
        }
    }

  TRACE_RETURN(previous_mode);
  return previous_mode;
}


UsageMode
Core::get_usage_mode()
{
  return usage_mode;
}

void
Core::set_usage_mode(UsageMode mode, bool persistent)
{
  if (usage_mode != mode)
    {
      usage_mode = mode;
  
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          breaks[i].set_usage_mode(mode);
        }

      if (persistent)
        {
          get_configurator()->set_value(CoreConfig::CFG_KEY_USAGE_MODE, mode);
        }
    }      
}

//! Sets the listener for core events.
void
Core::set_core_events_listener(ICoreEventListener *l)
{
  core_event_listener = l;
}


//! Forces the start of the specified break.
void
Core::force_break(BreakId id, BreakHint break_hint)
{
  do_force_break(id, break_hint);

#ifdef HAVE_DISTRIBUTION
  send_break_control_message_bool_param(id, BCM_START_BREAK, break_hint);
#endif
}


//! Forces the start of the specified break.
void
Core::do_force_break(BreakId id, BreakHint break_hint)
{
  TRACE_ENTER_MSG("Core::do_force_break", id);
  BreakControl *microbreak_control = breaks[BREAK_ID_MICRO_BREAK].get_break_control();
  BreakControl *breaker = breaks[id].get_break_control();

  if (id == BREAK_ID_REST_BREAK &&
      (microbreak_control->get_break_state() == BreakControl::BREAK_ACTIVE))
    {
      microbreak_control->stop_break(false);
      resume_break = BREAK_ID_MICRO_BREAK;
      TRACE_MSG("Resuming Micro break");
    }

  breaker->force_start_break(break_hint);
  TRACE_EXIT();
}


//! Announces a change in time.
void
Core::time_changed()
{
  TRACE_ENTER("Core::time_changed");

  // In case out timezone changed..
  tzset();

  // A change of system time idle handled by process_timewarp.
  // This is used to handle a change in timezone on windows.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      breaks[i].get_timer()->shift_time(0);
    }
  
  TRACE_EXIT();
}


//! Announces a powersave state.
void
Core::set_powersave(bool down)
{
  TRACE_ENTER_MSG("Core::set_powersave", down);
  if (down)
    {
      // Computer is going down
      powersave_operation_mode = set_operation_mode(OPERATION_MODE_SUSPENDED, false);
      TRACE_MSG("prev mode " << powersave_operation_mode);
      powersave_resume_time = 0;
      powersave = true;

      save_state();
      statistics->update();
    }
  else
    {
      // Computer is coming back
      // leave powersave true until the timewarp is detected
      // or until some time has passed
      if (powersave_resume_time == 0)
        {
          powersave_resume_time = current_time ? current_time : 1;
          TRACE_MSG("set resume time " << powersave_resume_time);
        }

      TRACE_MSG("resume time " << powersave_resume_time);
      set_operation_mode(powersave_operation_mode, false);
    }
  TRACE_EXIT();
}


//! Sets the insist policy.
/*!
 *  The insist policy determines what to do when the user is active while
 *  taking a break.
 */
void
Core::set_insist_policy(ICore::InsistPolicy p)
{
  TRACE_ENTER_MSG("Core::set_insist_policy", p);

  if (active_insist_policy != ICore::INSIST_POLICY_INVALID &&
      insist_policy != p)
    {
      TRACE_MSG("refreeze " << active_insist_policy);
      defrost();
      insist_policy = p;
      freeze();
    }
  else
    {
      insist_policy = p;
    }
  TRACE_EXIT();
}


//! Gets the insist policy.
ICore::InsistPolicy
Core::get_insist_policy() const
{
  return insist_policy;
}



// ! Forces all monitors to be idle.
void
Core::force_idle()
{
  TRACE_ENTER("Core::force_idle");
  monitor->force_idle();

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      IActivityMonitor *am = breaks[i].get_timer()->get_activity_monitor();
      if (am != NULL)
        {
          am->force_idle();
        }

      breaks[i].get_timer()->force_idle();
    }
  TRACE_EXIT();
}


/********************************************************************************/
/**** Break Response                                                        ******/
/********************************************************************************/

//! User postpones the specified break.
void
Core::postpone_break(BreakId break_id)
{
  do_postpone_break(break_id);

#ifdef HAVE_DISTRIBUTION
  send_break_control_message(break_id, BCM_POSTPONE);
#endif
}


//! User skips the specified break.
void
Core::skip_break(BreakId break_id)
{
  do_skip_break(break_id);

#ifdef HAVE_DISTRIBUTION
  send_break_control_message(break_id, BCM_SKIP);
#endif
}


//! User stops the prelude.
void
Core::stop_prelude(BreakId break_id)
{
  TRACE_ENTER_MSG("Core::stop_prelude", break_id);
  do_stop_prelude(break_id);

#ifdef HAVE_DISTRIBUTION
  send_break_control_message(break_id, BCM_ABORT_PRELUDE);
#endif

  TRACE_EXIT();
}


//! User postpones the specified break.
void
Core::do_postpone_break(BreakId break_id)
{
  if (break_id >= 0 && break_id < BREAK_ID_SIZEOF)
    {
      BreakControl *bc = breaks[break_id].get_break_control();
      bc->postpone_break();
    }
}


//! User skips the specified break.
void
Core::do_skip_break(BreakId break_id)
{
  if (break_id >= 0 && break_id < BREAK_ID_SIZEOF)
    {
      BreakControl *bc = breaks[break_id].get_break_control();
      bc->skip_break();
    }
}


//!
void
Core::do_stop_prelude(BreakId break_id)
{
  TRACE_ENTER_MSG("Core::do_stop_prelude", break_id);
  if (break_id >= 0 && break_id < BREAK_ID_SIZEOF)
    {
      BreakControl *bc = breaks[break_id].get_break_control();
      bc->stop_prelude();
    }
  TRACE_EXIT();
}


/********************************************************************************/
/**** Break handling                                                       ******/
/********************************************************************************/

//! Periodic heartbeat.
void
Core::heartbeat()
{
  TRACE_ENTER("Core::heartbeat");
  assert(application != NULL);

  // Set current time.
  current_time = time(NULL);

  // Performs timewarp checking.
  bool warped = process_timewarp();

  // Process configuration
  configurator->heartbeat();

  // Perform distribution processing.
  process_distribution();

  if (!warped)
    {
      // Perform state computation.
      process_state();
    }
  
  // Perform timer processing.
  process_timers();

  // Send heartbeats to other components.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakControl *bc = breaks[i].get_break_control();
      if (bc != NULL && bc->need_heartbeat())
        {
          bc->heartbeat();
        }
    }

  // Make state persistent.
  if (current_time % SAVESTATETIME == 0)
    {
      statistics->update();
      save_state();
    }

  // Done.
  last_process_time = current_time;

  TRACE_EXIT();
}


//! Performs all distribution processing.
void
Core::process_distribution()
{
  bool previous_master_mode = master_node;

  // Default
  master_node = true;

#ifdef HAVE_DISTRIBUTION

  // Retrieve State.
  ActivityState state = monitor->get_current_state();

  if (dist_manager != NULL)
    {
      dist_manager->heartbeart();
      dist_manager->set_lock_master(state == ACTIVITY_ACTIVE);
      master_node = dist_manager->is_master();

      if (!master_node && state == ACTIVITY_ACTIVE)
        {
          master_node = dist_manager->claim();
        }
    }

  if ( (previous_master_mode != master_node) ||
       (master_node && local_state != state) )
    {
      PacketBuffer buffer;
      buffer.create();

      buffer.pack_ushort(1);
      buffer.pack_ushort(state);

      dist_manager->broadcast_client_message(DCM_MONITOR, buffer);

      buffer.clear();
      bool ret = request_timer_state(buffer);
      if (ret)
        {
          dist_manager->broadcast_client_message(DCM_TIMERS, buffer);
        }
    }

#endif
}


//! Computes the current state.
void
Core::process_state()
{
  // Default
  local_state = monitor->get_current_state();

  map<std::string, time_t>::iterator i = external_activity.begin();
  while (i != external_activity.end())
    {
      map<std::string, time_t>::iterator next = i;
      next++;

      if (i->second >= current_time)
        {
          local_state = ACTIVITY_ACTIVE;
        }
      else
        {
          external_activity.erase(i);
        }

      i = next;
    }

  monitor_state = local_state;

#if defined(HAVE_DISTRIBUTION) && !defined(NDEBUG)
  if (fake_monitor != NULL)
    {
      monitor_state = fake_monitor->get_current_state();
    }
#endif

#ifdef HAVE_DISTRIBUTION
  if (!master_node)
    {
      if (active_insist_policy == INSIST_POLICY_IGNORE)
        {
          // Our own monitor is suspended, also ignore
          // activity from remote parties.
          monitor_state = ACTIVITY_IDLE;
        }
      else
        {
          monitor_state = remote_state;
        }
    }

  // Update our idle history.
  idlelog_manager->update_all_idlelogs(dist_manager->get_master_id(), monitor_state);
#endif
}


void
Core::report_external_activity(std::string who, bool act)
{
  TRACE_ENTER_MSG("Core::report_external_activity", who << " " << act);
  if (act)
    {
      external_activity[who] = current_time + 10;
    }
  else
    {
      external_activity.erase(who);
    }
  TRACE_EXIT();
}


void
Core::is_timer_running(BreakId id, bool &value)
{
  Timer *timer = get_timer(id);
  value = timer->get_state() == STATE_RUNNING;
}


void
Core::get_timer_idle(BreakId id, int *value)
{
  Timer *timer = get_timer(id);
  *value = (int) timer->get_elapsed_idle_time();
}


void
Core::get_timer_elapsed(BreakId id, int *value)
{
  Timer *timer = get_timer(id);
  *value = (int) timer->get_elapsed_time();
}


void
Core::get_timer_overdue(BreakId id, int *value)
{
  Timer *timer = get_timer(id);
  *value = (int) timer->get_total_overdue_time();
}

//! Processes all timers.
void
Core::process_timers()
{
  TRACE_ENTER("Core::process_timers");

  TimerInfo infos[BREAK_ID_SIZEOF];

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      infos[i].enabled = breaks[i].is_enabled();
      if (infos[i].enabled)
        {
          breaks[i].get_timer()->enable();
        }
      else
        {
          breaks[i].get_timer()->disable();
        }

      // First process only timer that do not have their
      // own activity monitor.
      if (!(breaks[i].get_timer()->has_activity_monitor()))
        {
          breaks[i].get_timer()->process(monitor_state, infos[i]);
        }
    }

  // And process timer with activity monitor.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (breaks[i].get_timer()->has_activity_monitor())
        {
          breaks[i].get_timer()->process(monitor_state, infos[i]);
        }
    }

  
  // Process all timer events.
  for (int i = BREAK_ID_SIZEOF - 1; i >= 0;  i--)
    {
      TimerInfo &info = infos[i];
      if (breaks[i].is_enabled())
        {
          timer_action((BreakId)i, info);
        }

      if (i == BREAK_ID_DAILY_LIMIT &&
          (info.event == TIMER_EVENT_NATURAL_RESET ||
           info.event == TIMER_EVENT_RESET))
        {
          statistics->set_counter(Statistics::STATS_VALUE_TOTAL_ACTIVE_TIME, (int) info.elapsed_time);
          statistics->start_new_day();

          daily_reset();
        }
    }

  TRACE_EXIT();
}

#if defined(PLATFORM_OS_WIN32)

//! Process a possible timewarp on Win32
bool
Core::process_timewarp()
{
  bool ret = false;
  
  TRACE_ENTER("Core::process_timewarp");
  if (last_process_time != 0)
    {
      time_t gap = current_time - 1 - last_process_time;
      if (abs((int)gap) > 5)
        {
          if (!powersave)
            {
              TRACE_MSG("Time warp of " << gap << " seconds. Correcting");

              force_idle();

              monitor->shift_time((int)gap);
              for (int i = 0; i < BREAK_ID_SIZEOF; i++)
                {
                  breaks[i].get_timer()->shift_time((int)gap);
                }

              monitor_state = ACTIVITY_IDLE;
              ret = true;
            }
          else
            {
              TRACE_MSG("Time warp of " << gap << " seconds because of powersave");

              // In case the windows message was lost. some people reported that
              // workrave never restarted the timers...
              set_operation_mode(powersave_operation_mode);
            }
        }
      if (powersave && powersave_resume_time != 0 && current_time > powersave_resume_time + 30)
        {
          TRACE_MSG("End of time warp after powersave");

          powersave = false;
          powersave_resume_time = 0;
        }
    }
  TRACE_EXIT();
  return ret;
}

#else

//! Process a possible timewarp On Non-Windows
bool
Core::process_timewarp()
{
  bool ret = false;
  
  TRACE_ENTER("Core::process_timewarp");
  if (last_process_time != 0)
    {
      int gap = current_time - 1 - last_process_time;

      if (gap >= 30)
        {
          TRACE_MSG("Time warp of " << gap << " seconds. Powersafe");
          
          force_idle();

          int save_current_time = current_time;

          current_time = last_process_time + 1;
          monitor_state = ACTIVITY_IDLE;
          
          process_timers();

          current_time = save_current_time;
          ret = true;
        }
    }
  
  TRACE_EXIT();
  return ret;
}

#endif

//! Notication of a timer action.
/*!
 *  \param timerId ID of the timer that caused the action.
 *  \param action action that is performed by the timer.
*/
void
Core::timer_action(BreakId id, TimerInfo info)
{
  // No breaks when mode is quiet,
  if (operation_mode == OPERATION_MODE_QUIET &&
      info.event == TIMER_EVENT_LIMIT_REACHED)
    {
      return;
    }

  BreakControl *breaker = breaks[id].get_break_control();
  Timer *timer = breaks[id].get_timer();

  assert(breaker != NULL && timer != NULL);

  switch (info.event)
    {
    case TIMER_EVENT_LIMIT_REACHED:
      if (breaker->get_break_state() == BreakControl::BREAK_INACTIVE)
        {
          start_break(id);
        }
      break;

    case TIMER_EVENT_NATURAL_RESET:
      statistics->increment_break_counter(id, Statistics::STATS_BREAKVALUE_NATURAL_TAKEN);
      // FALLTHROUGH
      
    case TIMER_EVENT_RESET:
      if (breaker->get_break_state() == BreakControl::BREAK_ACTIVE)
        {
          breaker->stop_break();
        }
      break;

    default:
      break;
    }
}


//! starts the specified break.
/*!
 *  \param break_id ID of the timer that caused the break.
 */
void
Core::start_break(BreakId break_id, BreakId resume_this_break)
{
  // Don't show MB when RB is active, RB when DL is active.
  for (int bi = break_id; bi <= BREAK_ID_DAILY_LIMIT; bi++)
    {
      if (breaks[bi].get_break_control()->get_break_state() == BreakControl::BREAK_ACTIVE)
        {
          return;
        }
    }

  // Advance restbreak if it follows within 30s after the end of a microbreak
  BreakControl *restbreak_control;
  restbreak_control = breaks[BREAK_ID_REST_BREAK].get_break_control();

  if (break_id == BREAK_ID_REST_BREAK && resume_this_break == BREAK_ID_NONE)
    {
      breaks[BREAK_ID_REST_BREAK].override(BREAK_ID_REST_BREAK);
    }
  
  if (break_id == BREAK_ID_MICRO_BREAK && breaks[BREAK_ID_REST_BREAK].is_enabled())
    {
      Timer *rb_timer = breaks[BREAK_ID_REST_BREAK].get_timer();
      assert(rb_timer != NULL);

      bool activity_sensitive = breaks[BREAK_ID_REST_BREAK].get_timer_activity_sensitive();

      // Only advance when
      // 0. It is activity sensitive
      // 1. we have a next limit reached time.
      if (activity_sensitive &&
          rb_timer->get_next_limit_time() > 0)
        {
          Timer *timer = breaks[break_id].get_timer();

          time_t duration = timer->get_auto_reset();
          time_t now = get_time();

          if (now + duration + 30 >= rb_timer->get_next_limit_time())
            {
              breaks[BREAK_ID_REST_BREAK].override(BREAK_ID_MICRO_BREAK);
                
              start_break(BREAK_ID_REST_BREAK, BREAK_ID_MICRO_BREAK);

              // Snooze timer before the limit was reached. Just to make sure
              // that it doesn't reach its limit again when elapsed == limit
              rb_timer->snooze_timer();
              return;
            }
        }
    }

  // Stop microbreak when a restbreak starts. should not happend.
  // restbreak should be advanced.
  for (int bi = BREAK_ID_MICRO_BREAK; bi < break_id; bi++)
    {
      if (breaks[bi].get_break_control()->get_break_state() == BreakControl::BREAK_ACTIVE)
        {
          breaks[bi].get_break_control()->stop_break(false);
        }
    }

  // If break 'break_id' ends, and break 'resume_this_break' is still
  // active, resume it...
  resume_break = resume_this_break;

  BreakControl *breaker = breaks[break_id].get_break_control();
  breaker->start_break();
}


//! Sets the freeze state of all breaks.
void
Core::set_freeze_all_breaks(bool freeze)
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Timer *t = breaks[i].get_timer();
      assert(t != NULL);
      if (!t->has_activity_monitor())
        {
          t->freeze_timer(freeze);
        }
    }
}


//! Stops all breaks.
void
Core::stop_all_breaks()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakControl *bc = breaks[i].get_break_control();
      assert(bc != NULL);
      bc->stop_break(false);
    }
}


/********************************************************************************/
/**** Misc                                                                 ******/
/********************************************************************************/

//! Performs a reset when the daily limit is reached.
void
Core::daily_reset()
{
  TRACE_ENTER("Core::daily_reset");
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Timer *t = breaks[i].get_timer();
      assert(t != NULL);

      time_t overdue = t->get_total_overdue_time();

      statistics->set_break_counter(((BreakId)i),
                                    Statistics::STATS_BREAKVALUE_TOTAL_OVERDUE, (int)overdue);

      t->daily_reset_timer();
    }

  
#ifdef HAVE_DISTRIBUTION
  idlelog_manager->reset();
#endif

  TRACE_EXIT();
}


//! Saves the current state.
void
Core::save_state() const
{
  stringstream ss;
  ss << Util::get_home_directory();
  ss << "state" << ends;

  ofstream stateFile(ss.str().c_str());

  stateFile << "WorkRaveState 3"  << endl
            << get_time() << endl;

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      string stateStr = breaks[i].get_timer()->serialize_state();

      stateFile << stateStr << endl;
    }

  stateFile.close();
}


//! Loads miscellaneous
void
Core::load_misc()
{
  configurator->add_listener(CoreConfig::CFG_KEY_OPERATION_MODE, this);
  configurator->add_listener(CoreConfig::CFG_KEY_USAGE_MODE, this);
  
  int mode;
  if (! get_configurator()->get_value(CoreConfig::CFG_KEY_OPERATION_MODE, mode))
    {
      mode = OPERATION_MODE_NORMAL;
    }
  set_operation_mode(OperationMode(mode), false);

  if (! get_configurator()->get_value(CoreConfig::CFG_KEY_USAGE_MODE, mode))
    {
      mode = USAGE_MODE_NORMAL;
    }
  set_usage_mode(UsageMode(mode), false);
}


//! Loads the current state.
void
Core::load_state()
{
  stringstream ss;
  ss << Util::get_home_directory();
  ss << "state" << ends;

  ifstream stateFile(ss.str().c_str());

  int version = 0;
  bool ok = stateFile.good();

  if (ok)
    {
      string tag;
      stateFile >> tag;

      ok = (tag == WORKRAVESTATE);
    }

  if (ok)
    {
      stateFile >> version;

      ok = (version >= 1 && version <= 3);
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

      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          if (breaks[i].get_timer()->get_id() == id)
            {
              string state;
              getline(stateFile, state);

              breaks[i].get_timer()->deserialize_state(state, version);
              break;
            }
        }
    }
}


//! Post an event.
void
Core::post_event(CoreEvent event)
{
  if (core_event_listener != NULL)
    {
      core_event_listener->core_event_notify(event);
    }
}


//! Excecute the insist policy.
void
Core::freeze()
{
  TRACE_ENTER_MSG("Core::freeze", insist_policy);
  ICore::InsistPolicy policy = insist_policy;

  switch (policy)
    {
    case ICore::INSIST_POLICY_IGNORE:
      {
        // Ignore all activity during break by suspending the activity monitor.
        monitor->suspend();
      }
      break;
    case ICore::INSIST_POLICY_HALT:
      {
        // Halt timer when the user is active.
        set_freeze_all_breaks(true);
      }
      break;
    case ICore::INSIST_POLICY_RESET:
      // reset the timer when the user becomes active.
      // default.
      break;

    default:
      break;
    }

  active_insist_policy = policy;
  TRACE_EXIT();
}


//! Undo the insist policy.
void
Core::defrost()
{
  TRACE_ENTER_MSG("Core::defrost", active_insist_policy);

  switch (active_insist_policy)
    {
    case ICore::INSIST_POLICY_IGNORE:
      {
        // Resumes the activity monitor, if not suspended.
        if (operation_mode != OPERATION_MODE_SUSPENDED)
          {
            monitor->resume();
          }
      }
      break;
    case ICore::INSIST_POLICY_HALT:
      {
        // Desfrost timers.
        set_freeze_all_breaks(false);
      }
      break;

    default:
      break;
    }

  active_insist_policy = ICore::INSIST_POLICY_INVALID;
  TRACE_EXIT();
}


//! Is the user currently active?
bool
Core::is_user_active() const
{
  return monitor_state == ACTIVITY_ACTIVE;
}


/********************************************************************************/
/**** Distribution                                                         ******/
/********************************************************************************/

#ifdef HAVE_DISTRIBUTION
//! The distribution manager requests a client message.
bool
Core::request_client_message(DistributionClientMessageID id, PacketBuffer &buffer)
{
  bool ret = false;

  switch (id)
    {
    case DCM_BREAKS:
      ret = request_break_state(buffer);
      break;

    case DCM_TIMERS:
      ret = request_timer_state(buffer);
      break;

    case DCM_CONFIG:
      break;

    case DCM_MONITOR:
      ret = true;
      break;

    case DCM_BREAKCONTROL:
      ret = true;
      break;

    case DCM_IDLELOG:
      idlelog_manager->get_idlelog(buffer);
      ret = true;
      break;

    default:
      break;
    }

  return ret;
}


//! The distribution manager delivers a client message.
bool
Core::client_message(DistributionClientMessageID id, bool master, const char *client_id,
                     PacketBuffer &buffer)
{
  bool ret = false;

  (void) client_id;
  
  switch (id)
    {
    case DCM_BREAKS:
      ret = set_break_state(master, buffer);
      break;

    case DCM_TIMERS:
      ret = set_timer_state(buffer);
      break;

    case DCM_MONITOR:
      ret = set_monitor_state(master, buffer);
      break;

    case DCM_BREAKCONTROL:
      ret = set_break_control(buffer);
      break;

    case DCM_CONFIG:
      break;

    case DCM_IDLELOG:
      idlelog_manager->set_idlelog(buffer);
      compute_timers();
      ret = true;
      break;

    default:
      break;
    }

  return ret;
}


bool
Core::request_break_state(PacketBuffer &buffer)
{
  buffer.pack_ushort(BREAK_ID_SIZEOF);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakControl *bi = breaks[i].get_break_control();

      if (bi != NULL)
        {
          BreakControl::BreakStateData state_data;
          bi->get_state_data(state_data);

          int pos = buffer.bytes_written();

          buffer.pack_ushort(0);
          buffer.pack_byte((guint8)state_data.forced_break);
          buffer.pack_byte((guint8)state_data.reached_max_prelude);
          buffer.pack_ulong((guint32)state_data.prelude_count);
          buffer.pack_ulong((guint32)state_data.break_stage);
          buffer.pack_ulong((guint32)state_data.prelude_time);

          buffer.poke_ushort(pos, buffer.bytes_written() - pos);
        }
      else
        {
          buffer.pack_ushort(0);
        }
    }

  return true;
}


bool
Core::set_break_state(bool master, PacketBuffer &buffer)
{
  int num_breaks = buffer.unpack_ushort();

  if (num_breaks > BREAK_ID_SIZEOF)
    {
      num_breaks = BREAK_ID_SIZEOF;
    }

  for (int i = 0; i < num_breaks; i++)
    {
      BreakControl *bi = breaks[i].get_break_control();

      BreakControl::BreakStateData state_data;

      int data_size = buffer.unpack_ushort();

      if (data_size > 0)
        {
          state_data.forced_break = buffer.unpack_byte();
          state_data.reached_max_prelude = buffer.unpack_byte();
          state_data.prelude_count = buffer.unpack_ulong();
          state_data.break_stage = buffer.unpack_ulong();
          state_data.prelude_time = buffer.unpack_ulong();

          bi->set_state_data(master, state_data);
        }
    }

  return true;
}

bool
Core::request_timer_state(PacketBuffer &buffer) const
{
  TRACE_ENTER("Core::get_timer_state");

  buffer.pack_ushort(BREAK_ID_SIZEOF);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Timer *t = breaks[i].get_timer();
      buffer.pack_string(t->get_id().c_str());

      Timer::TimerStateData state_data;

      t->get_state_data(state_data);

      int pos = buffer.bytes_written();

      buffer.pack_ushort(0);
      buffer.pack_ulong((guint32)state_data.current_time);
      buffer.pack_ulong((guint32)state_data.elapsed_time);
      buffer.pack_ulong((guint32)state_data.elapsed_idle_time);
      buffer.pack_ulong((guint32)state_data.last_pred_reset_time);
      buffer.pack_ulong((guint32)state_data.total_overdue_time);

      buffer.pack_ulong((guint32)state_data.last_limit_time);
      buffer.pack_ulong((guint32)state_data.last_limit_elapsed);
      buffer.pack_ushort((guint16)state_data.snooze_inhibited);

      buffer.poke_ushort(pos, buffer.bytes_written() - pos);
    }

  TRACE_EXIT();
  return true;
}


bool
Core::set_timer_state(PacketBuffer &buffer)
{
  TRACE_ENTER("Core::set_timer_state");

  int num_breaks = buffer.unpack_ushort();

  TRACE_MSG("numtimer = " << num_breaks);
  for (int i = 0; i < num_breaks; i++)
    {
      gchar *id = buffer.unpack_string();
      TRACE_MSG("id = " << id);

      if (id == NULL)
        {
          TRACE_EXIT();
          return false;
        }

      Timer *t = (Timer *)get_timer(id);

      Timer::TimerStateData state_data;

      buffer.unpack_ushort();

      state_data.current_time = buffer.unpack_ulong();
      state_data.elapsed_time = buffer.unpack_ulong();
      state_data.elapsed_idle_time = buffer.unpack_ulong();
      state_data.last_pred_reset_time = buffer.unpack_ulong();
      state_data.total_overdue_time = buffer.unpack_ulong();

      state_data.last_limit_time = buffer.unpack_ulong();
      state_data.last_limit_elapsed = buffer.unpack_ulong();
      state_data.snooze_inhibited = buffer.unpack_ushort();

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

      g_free(id);
    }

  TRACE_EXIT();
  return true;
}


bool
Core::set_monitor_state(bool master, PacketBuffer &buffer)
{
  TRACE_ENTER_MSG("Core::set_monitor_state", master << " " << master_node);

  if (!master_node)
    {
      buffer.unpack_ushort();
      remote_state = (ActivityState) buffer.unpack_ushort();
      TRACE_MSG(remote_state);
    }

  TRACE_EXIT();
  return true;
}


//! A remote client has signed on.
void
Core::signon_remote_client(string client_id)
{
  idlelog_manager->signon_remote_client(client_id);

  if (master_node)
    {
      PacketBuffer buffer;
      buffer.create();

      ActivityState state = monitor->get_current_state();

      buffer.pack_ushort(1);
      buffer.pack_ushort(state);

      dist_manager->broadcast_client_message(DCM_MONITOR, buffer);

      buffer.clear();
    }
}


//! A remote client has signed off.
void
Core::signoff_remote_client(string client_id)
{
  TRACE_ENTER_MSG("Core::signoff_remote_client", client_id);
  TRACE_MSG("Master = " << dist_manager->get_master_id());
  if (client_id == dist_manager->get_master_id())
    {
      TRACE_MSG("Idle");
      remote_state = ACTIVITY_IDLE;
    }

  idlelog_manager->signoff_remote_client(client_id);
  TRACE_EXIT();
}


void
Core::compute_timers()
{
  TRACE_ENTER("IdleLogManager:compute_timers");

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      int autoreset = breaks[i].get_timer()->get_auto_reset();
      int idle = idlelog_manager->compute_idle_time();

      if (autoreset != 0)
        {
          int active_time = idlelog_manager->compute_active_time(autoreset);

          if (idle > autoreset)
            {
              idle = autoreset;
            }

          breaks[i].get_timer()->set_values(active_time, idle);
        }
      else
        {
          int active_time = idlelog_manager->compute_total_active_time();
          breaks[i].get_timer()->set_values(active_time, idle);
        }
    }

  TRACE_EXIT();
}


//! Sends a break control message to all workrave clients.
void
Core::send_break_control_message(BreakId break_id, BreakControlMessage message)
{
  PacketBuffer buffer;
  buffer.create();

  buffer.pack_ushort(4);
  buffer.pack_ushort(break_id);
  buffer.pack_ushort(message);

  dist_manager->broadcast_client_message(DCM_BREAKCONTROL, buffer);
}

//! Sends a break control message with boolean parameter to all workrave clients.
void
Core::send_break_control_message_bool_param(BreakId break_id, BreakControlMessage message,
                                            bool param)
{
  PacketBuffer buffer;
  buffer.create();

  buffer.pack_ushort(5);
  buffer.pack_ushort(break_id);
  buffer.pack_ushort(message);
  buffer.pack_byte(param);

  dist_manager->broadcast_client_message(DCM_BREAKCONTROL, buffer);
}


bool
Core::set_break_control(PacketBuffer &buffer)
{
  int data_size = buffer.unpack_ushort();

  if (data_size >= 4)
    {
      BreakId break_id = (BreakId) buffer.unpack_ushort();
      BreakControlMessage message = (BreakControlMessage) buffer.unpack_ushort();

      switch (message)
        {
        case BCM_POSTPONE:
          do_postpone_break(break_id);
          break;

        case BCM_SKIP:
          do_skip_break(break_id);
          break;

        case BCM_ABORT_PRELUDE:
          do_stop_prelude(break_id);
          break;

        case BCM_START_BREAK:
          if (data_size >= 6)
            {
              // Only for post 1.9.1 workrave...
              int break_hint = (int) buffer.unpack_ushort();
              do_force_break(break_id, (BreakHint) break_hint);
            }
          else if (data_size >= 5)
            {
              // Only for post 1.6.2 workrave...
              bool initiated_by_user = (bool) buffer.unpack_byte();
              do_force_break(break_id, initiated_by_user ? BREAK_HINT_USER_INITIATED : BREAK_HINT_NONE);
            }
          else
            {
              do_force_break(break_id, BREAK_HINT_USER_INITIATED);
            }
          break;
        }
    }

  return true;
}

#endif // HAVE_DISTRIBUTION

namespace workrave
{
  std::string operator%(const string &key, BreakId id)
  {
    IBreak *b = Core::get_instance()->get_break(id);
  
    string str = key;
    string::size_type pos = 0;
    string name = b->get_name();
  
    while ((pos = str.find("%b", pos)) != string::npos)
      {
        str.replace(pos, 2, name);
        pos++;
      }
  
    return str;
  }
}
