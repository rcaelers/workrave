// Core.cc --- The main controller
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

// FIXME: remove
#if defined(PLATFORM_OS_WIN32)
#define BACKEND
#include "w32debug.hh"
#endif

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

#ifdef HAVE_TESTS
#include "FakeActivityMonitor.hh"
#endif

#ifdef HAVE_GCONF
#include <gconf/gconf-client.h>
#endif

#ifdef HAVE_DBUS
#include "DBus.hh"
#include "DBusException.hh"
#ifdef HAVE_TESTS
#include "Test.hh"
#endif
#endif

#ifdef HAVE_DISTRIBUTION
#include "Network.hh"
#include "BreakLinkEvent.hh"
#include "CoreLinkEvent.hh"
#include "ActivityLinkEvent.hh"
#include "TimerStateLinkEvent.hh"
#endif

Core *Core::instance = NULL;

const char *WORKRAVESTATE="WorkRaveState";
const int SAVESTATETIME = 60;

#define DBUS_PATH_WORKRAVE         "/org/workrave/Workrave/Core"
#define DBUS_SERVICE_WORKRAVE      "org.workrave.Workrave"

//! Constructs a new Core.
Core::Core() :
  last_process_time(0),
  configurator(NULL),
  monitor(NULL),
  application(NULL),
  statistics(NULL),
  operation_mode(OPERATION_MODE_NORMAL),
  core_event_listener(NULL),
  powersave(false),
  powersave_resume_time(0),
  powersave_operation_mode(OPERATION_MODE_NORMAL),
  insist_policy(ICore::INSIST_POLICY_HALT),
  active_insist_policy(ICore::INSIST_POLICY_INVALID),
  resume_break(BREAK_ID_NONE),
  local_state(ACTIVITY_IDLE),
  monitor_state(ACTIVITY_UNKNOWN)
{
  TRACE_ENTER("Core::Core");
  current_time = time(NULL);

  assert(! instance);
  instance = this;

#ifdef HAVE_TESTS
  fake_monitor = NULL;
  manual_clock = false;

  const char *env = getenv("WORKRAVE_TEST");
  if (env != NULL)
    {
      current_time = 1000;
      manual_clock = true;
    }
#endif

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

#ifdef HAVE_TESTS
  delete fake_monitor;
#endif

#ifdef HAVE_DISTRIBUTION
  if (network != NULL)
    {
      network->terminate();
    }
  delete network;
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
  init_breaks();
  init_statistics();
  init_networking();
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
#ifdef HAVE_DISTRIBUTION
      dbus->connect(DBUS_PATH_WORKRAVE, "org.workrave.NetworkInterface", network);
#endif
      
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
  catch (DBusException &e)
    {
    }
#endif
}


//! Initializes the network facility.
void
Core::init_networking()
{
#ifdef HAVE_DISTRIBUTION
  network = new Network();
  network->init();
  network->subscribe("breaklinkevent", this);
  network->subscribe("corelinkevent", this);
  network->subscribe("timerstatelinkevent", this);
  
  network->report_active(false);
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      network->report_timer_state(i, false);
    }
#endif
}


//! Initializes the activity monitor.
void
Core::init_monitor(const string &display_name)
{
#ifdef HAVE_TESTS
  fake_monitor = NULL;
  const char *env = getenv("WORKRAVE_FAKE");
  if (env != NULL)
    {
      fake_monitor = new FakeActivityMonitor();
    }
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

#ifdef PLATFORM_OS_WIN32
// FIXME: debug, remove later
if(mode == OPERATION_MODE_NORMAL)
APPEND_TIME("Core::set_operation_mode()", "OPERATION_MODE_NORMAL ")
else if (mode == OPERATION_MODE_SUSPENDED)
APPEND_TIME("Core::set_operation_mode()", "OPERATION_MODE_SUSPENDED ")
else if (mode == OPERATION_MODE_QUIET)
APPEND_TIME("Core::set_operation_mode()", "OPERATION_MODE_QUIET ")
#endif

  OperationMode previous_mode = operation_mode;
  
  if (operation_mode != mode)
    {
      set_operation_mode_no_event(mode, persistent);

#ifdef HAVE_DISTRIBUTION
      CoreLinkEvent::CoreEvent ce = CoreLinkEvent::CORE_EVENT_NONE;
      
      if (operation_mode == OPERATION_MODE_SUSPENDED)
        {
          ce = CoreLinkEvent::CORE_EVENT_MODE_SUSPENDED;
        }
      else if (operation_mode == OPERATION_MODE_NORMAL)
        {
          ce = CoreLinkEvent::CORE_EVENT_MODE_NORMAL;
        }
      else if(operation_mode == OPERATION_MODE_QUIET)
        {
          ce = CoreLinkEvent::CORE_EVENT_MODE_QUIET;
        }

      if (ce != CoreLinkEvent::CORE_EVENT_NONE)
        {
          CoreLinkEvent event(ce);
          network->send_event(&event);
        }
#endif
    }
  return previous_mode;
}


//! Sets the operation mode
OperationMode
Core::set_operation_mode_no_event(OperationMode mode, bool persistent)
{

#ifdef PLATFORM_OS_WIN32
// FIXME: debug, remove later
if(mode == OPERATION_MODE_NORMAL)
APPEND_TIME("Core::set_operation_mode()", "OPERATION_MODE_NORMAL ")
else if (mode == OPERATION_MODE_SUSPENDED)
APPEND_TIME("Core::set_operation_mode()", "OPERATION_MODE_SUSPENDED ")
else if (mode == OPERATION_MODE_QUIET)
APPEND_TIME("Core::set_operation_mode()", "OPERATION_MODE_QUIET ")
#endif

  OperationMode previous_mode = operation_mode;
  
  if (operation_mode != mode)
    {
      operation_mode = mode;
      
      if (operation_mode == OPERATION_MODE_SUSPENDED)
        {
          force_idle();
          monitor->suspend();
          stop_all_breaks();
          
          for( int i = 0; i < BREAK_ID_SIZEOF; ++i )
            {
              if( breaks[ i ].is_enabled() )
                {
                  breaks[ i ].get_timer()->set_insensitive_mode( MODE_IDLE_ALWAYS );
                  //breaks[ i ].get_timer()->freeze_timer( true );
                }
            }
        }
      else if (previous_mode == OPERATION_MODE_SUSPENDED)
        {
          // stop_all_breaks again will reset insensitive mode (that is good)
          stop_all_breaks();
          monitor->resume();
        }
      
      if( operation_mode == OPERATION_MODE_QUIET )
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
  
  return previous_mode;
}


//! Sets the listener for core events.
void
Core::set_core_events_listener(ICoreEventListener *l)
{
  core_event_listener = l;
}


//! Forces the start of the specified break.
void
Core::force_break(BreakId id, bool initiated_by_user)
{
  do_force_break(id, initiated_by_user);

#ifdef HAVE_DISTRIBUTION
  BreakLinkEvent event(id, initiated_by_user ?
                       BreakLinkEvent::BREAK_EVENT_USER_FORCE_BREAK :
                       BreakLinkEvent::BREAK_EVENT_SYST_FORCE_BREAK) ;
  network->send_event(&event);
#endif
}


//! Forces the start of the specified break.
void
Core::do_force_break(BreakId id, bool initiated_by_user)
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

  breaker->force_start_break(initiated_by_user);
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
        }

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
  monitor->force_idle();

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      IActivityMonitor *am = breaks[i].get_timer()->get_activity_monitor();
      if (am != NULL)
        {
          am->force_idle();
        }
    }
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
  BreakLinkEvent event(break_id, BreakLinkEvent::BREAK_EVENT_USER_POSTPONE);
  network->send_event(&event);
#endif

  /* FIXME: move to frontend
  if (resume_break != BREAK_ID_NONE &&
      !breaks[resume_break].get_break_ignorable())
    {
      Timer *timer = breaks[resume_break].get_timer();
      assert(timer != NULL);

      if (timer->get_elapsed_time() > timer->get_limit())
        {
          force_break(resume_break, false);
          resume_break = BREAK_ID_NONE;
        }
    }
  */
}


//! User skips the specified break.
void
Core::skip_break(BreakId break_id)
{
  do_skip_break(break_id);

#ifdef HAVE_DISTRIBUTION
  BreakLinkEvent event(break_id, BreakLinkEvent::BREAK_EVENT_USER_SKIP);
  network->send_event(&event);
#endif

  /* FIXME: move to frontend
  if (resume_break != BREAK_ID_NONE &&
      !breaks[resume_break].get_break_ignorable())
    {
      Timer *timer = breaks[resume_break].get_timer();
      assert(timer != NULL);

      if (timer->get_elapsed_time() > timer->get_limit())
        {
          force_break(resume_break, false);
          resume_break = BREAK_ID_NONE;
        }
    }
  */
}


//! User stops the prelude.
void
Core::stop_prelude(BreakId break_id)
{
  TRACE_ENTER_MSG("Core::stop_prelude", break_id);
  do_stop_prelude(break_id);

#ifdef HAVE_DISTRIBUTION
  BreakLinkEvent event(break_id, BreakLinkEvent::BREAK_EVENT_SYST_STOP_PRELUDE);
  network->send_event(&event);
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

#ifdef HAVE_TESTS
  if (!manual_clock)
    {
      current_time = time(NULL);
    }
#else
  // Set current time.
  current_time = time(NULL);
#endif
  TRACE_MSG("Time = " << current_time);

  // Process configuration
  configurator->heartbeat();

  // Perform state computation.
  process_state();

  // Performs timewarp checking.
  process_timewarp();

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

#ifdef HAVE_DISTRIBUTION
  network->heartbeat();
#endif

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



//! Computes the current state.
void
Core::process_state()
{
  TRACE_ENTER("Core::process_state");

  ActivityState previous_state = local_state;
  bool previous_active = previous_state == ACTIVITY_ACTIVE;

  // Default
  local_state = monitor->get_current_state();
  TRACE_MSG("local (internal) state: " << local_state);

  map<std::string, int>::iterator i = external_activity.begin();
  while (i != external_activity.end())
    {
      map<std::string, int>::iterator next = i;
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
  
  TRACE_MSG("local (external) state: " << local_state);

  
#ifdef HAVE_TESTS
  if (fake_monitor != NULL)
    {
      local_state = fake_monitor->get_current_state();
      TRACE_MSG("FAKE state: " << local_state);
    }
#endif
  monitor_state = local_state;

#ifdef HAVE_DISTRIBUTION
  bool active = local_state == ACTIVITY_ACTIVE;
  bool remote_active = network->get_remote_active();

  if (previous_active != active)
    {
      network->report_active(active);
    }

  if (previous_active != active || (active && current_time % 10 == 0))
    {
      ActivityLinkEvent event(local_state);
      network->send_event(&event);
    }
 
  if ((!previous_active && active) || current_time % 10 == 0)
    {
      broadcast_state();
    }
  
  TRACE_MSG("remote active" << remote_active);
  if (remote_active)
    {
      monitor_state = ACTIVITY_ACTIVE;
    }
#endif

  if (monitor_state == ACTIVITY_ACTIVE && !previous_active)
    {
      active_since = get_time();      
    }
  else if (monitor_state == ACTIVITY_IDLE)
    {
      active_since = -1;
    }
  TRACE_EXIT();
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
  *value = timer->get_elapsed_idle_time();
}


void
Core::get_timer_elapsed(BreakId id, int *value)
{
  Timer *timer = get_timer(id);
  *value = timer->get_elapsed_time();
}


void
Core::get_timer_overdue(BreakId id, int *value)
{
  Timer *timer = get_timer(id);
  *value = timer->get_total_overdue_time();
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

#ifdef HAVE_DISTRIBUTION
      TimerState timer_state = breaks[i].get_timer()->get_state();
      TRACE_MSG("break" << i << " time" << current_time << " timer state " << timer_state);
      network->report_timer_state(i, timer_state == STATE_RUNNING);
#endif
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
          statistics->set_counter(Statistics::STATS_VALUE_TOTAL_ACTIVE_TIME, info.elapsed_time);
          statistics->start_new_day();

          daily_reset();
        }
    }

  TRACE_EXIT();
}

#ifdef PLATFORM_OS_WIN32

//! Process a possible timewarp on Win32
void
Core::process_timewarp()
{
  TRACE_ENTER("Core::process_timewarp");
#ifdef HAVE_TESTS
  if (manual_clock)
    {
      // Don't to warping is manual clock mode
      TRACE_EXIT();
      return;
    }
#endif

  if (last_process_time != 0)
    {
      int gap = current_time - 1 - last_process_time;
      if (abs(gap) > 5)
        {
          if (!powersave)
            {
              TRACE_MSG("Time warp of " << gap << " seconds. Correcting");


              force_idle();

              monitor->shift_time(gap);
              for (int i = 0; i < BREAK_ID_SIZEOF; i++)
                {
                  breaks[i].get_timer()->shift_time(gap);
                }

              monitor_state = ACTIVITY_IDLE;
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
}

#else

//! Process a possible timewarp On Non-Windows
void
Core::process_timewarp()
{
  TRACE_ENTER("Core::process_timewarp");
  if (last_process_time != 0)
    {
      int gap = current_time - 1 - last_process_time;
      bool do_powersafe = false;
      bool do_timeshift = false;

      if (gap < 0 || (gap > 5 && gap < 600))
        {
          // Negative gap are always intrepreted as timechanges.
          // Positive gaps between 5..600 are also considered timechanges
          do_timeshift = true;
        }
      else if (gap >= 600)
        {
          // Gaps > 600 are considered powersaves.
          do_powersafe = true;
        }
      
      if (do_powersafe)
        {
          TRACE_MSG("Time warp of " << gap << " seconds. Powersafe");
          
          force_idle();

          /* Assume it is a powersave... */
          int save_current_time = current_time;

          current_time = last_process_time + 1;
          monitor_state = ACTIVITY_IDLE;
          
          process_timers();

          current_time = save_current_time;
        }

      if (do_timeshift)
        {
          TRACE_MSG("Time warp of " << gap << " seconds. Timeshift");

          monitor->shift_time(gap);
          for (int i = 0; i < BREAK_ID_SIZEOF; i++)
            {
              breaks[i].get_timer()->shift_time(gap);
            }
        }
    }
  
  TRACE_EXIT();
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

    case TIMER_EVENT_RESET:
      if (breaker->get_break_state() == BreakControl::BREAK_ACTIVE)
        {
          breaker->stop_break();
        }
      break;

    case TIMER_EVENT_NATURAL_RESET:
      if (breaker->get_break_state() == BreakControl::BREAK_ACTIVE)
        {
          breaker->stop_break();
        }
      statistics->increment_break_counter(id, Statistics::STATS_BREAKVALUE_NATURAL_TAKEN);
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

      int overdue = t->get_total_overdue_time();

      statistics->set_break_counter(((BreakId)i),
                                    Statistics::STATS_BREAKVALUE_TOTAL_OVERDUE, overdue);

      t->daily_reset_timer();
    }

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
  int mode;
  if (! get_configurator()->get_value(CoreConfig::CFG_KEY_OPERATION_MODE, mode))
    {
      mode = OPERATION_MODE_NORMAL;
    }
  set_operation_mode(OperationMode(mode), false);
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
  bool ok = stateFile;

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
  TRACE_ENTER_MSG("BreakControl::freeze", insist_policy);
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
  TRACE_ENTER_MSG("BreakControl::defrost", active_insist_policy);

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
/**** Networking support                                                   ******/
/********************************************************************************/

#ifdef HAVE_DISTRIBUTION
//! Process event from remote Workrave.
void
Core::event_received(LinkEvent *event)
{
  TRACE_ENTER_MSG("Core::event_received", event->str());

  const BreakLinkEvent *ble = dynamic_cast<const BreakLinkEvent *>(event);
  if (ble != NULL)
    {
      break_event_received(ble);
    }

  const CoreLinkEvent *cle = dynamic_cast<const CoreLinkEvent *>(event);
  if (cle != NULL)
    {
      core_event_received(cle);
    }

  const TimerStateLinkEvent *tsle = dynamic_cast<const TimerStateLinkEvent *>(event);
  if (tsle != NULL)
    {
      timer_state_event_received(tsle);
    }
  
  TRACE_EXIT();
}


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
      do_force_break(break_id, true);
      break;

    case BreakLinkEvent::BREAK_EVENT_SYST_FORCE_BREAK:
      do_force_break(break_id, false);
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

      int old_idle = timer->get_elapsed_idle_time();
      int old_active = timer->get_elapsed_time();

      int idle = idle_times[i];
      int active = active_times[i];

      TRACE_MSG("Timer " << i <<
                " idle " <<  idle << " " << old_idle <<
                " active" << active << " " << old_active); 

      time_t remote_active_since = 0;
      bool remote_active = network->is_remote_active(event->get_source(),
                                                        remote_active_since);
      
      if (abs(idle - old_idle) >=2 ||
          abs(active - old_active) >= 2 ||
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

      idle_times.push_back(timer->get_elapsed_idle_time());
      active_times.push_back(timer->get_elapsed_time());
    }

  TimerStateLinkEvent event(idle_times, active_times);
  network->send_event(&event);
}

#endif

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
