// Core.cc --- The main controller
//
// Copyright (C) 2001 - 2012 Rob Caelers & Raymond Penners
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

#include "Core.hh"

#include "config/ConfiguratorFactory.hh"
#include "config/IConfigurator.hh"
#include "utils/TimeSource.hh"
#include "input-monitor/InputMonitorFactory.hh"

#include "Util.hh"
#include "IApp.hh"
#include "ActivityMonitor.hh"
#include "Break.hh"
#include "CoreConfig.hh"
#include "Statistics.hh"

#ifdef HAVE_GCONF
#include <gconf/gconf-client.h>
#endif

#ifdef HAVE_DBUS
#if defined(PLATFORM_OS_WIN32_NATIVE)
#undef interface
#endif
#include "dbus/DBus.hh"
#include "dbus/DBusException.hh"
#endif

#define DBUS_PATH_WORKRAVE         "/org/workrave/Workrave/"
#define DBUS_SERVICE_WORKRAVE      "org.workrave.Workrave"

using namespace workrave::dbus;

ICore::Ptr
ICore::create()
{
  return Ptr(new Core());
}


//! Constructs a new Core.
Core::Core() :
  argc(0),
  argv(NULL),
  application(NULL),
  operation_mode(OPERATION_MODE_NORMAL),
  operation_mode_regular(OPERATION_MODE_NORMAL),
  usage_mode(USAGE_MODE_NORMAL)
{
  TRACE_ENTER("Core::Core");
  current_real_time = g_get_real_time();
  current_monotonic_time = g_get_monotonic_time();
  TRACE_EXIT();
}


//! Destructor.
Core::~Core()
{
  TRACE_ENTER("Core::~Core");

  if (monitor != NULL)
    {
      monitor->terminate();
    }

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

  TimeSource::source = shared_from_this();
  
  init_configurator();
  init_monitor(display_name);
  init_statistics();
  init_breaks();
  init_bus();

  load_config();
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
#if defined(HAVE_GCONF)
      gconf_init(argc, argv, NULL);
      g_type_init();
#endif
      
      configurator = ConfiguratorFactory::create(ConfiguratorFactory::FormatNative);
#if defined(HAVE_GDOME)
      if (configurator == NULL)
        {
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
        }
#endif
      if (configurator == NULL)
        {
          ini_file = Util::get_home_directory() + "workrave.ini";
          configurator = ConfiguratorFactory::create(ConfiguratorFactory::FormatIni);
          configurator->load(ini_file);
          configurator->save(ini_file);
        }
    }
  
  string home;
  if (configurator->get_value(CoreConfig::CFG_KEY_GENERAL_DATADIR, home) &&
      home != "")
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

      extern void init_DBusWorkrave(DBus *dbus);
      init_DBusWorkrave(dbus);

      dbus->connect(DBUS_PATH_WORKRAVE "Core", "org.workrave.CoreInterface", this);
      dbus->connect(DBUS_PATH_WORKRAVE "Core", "org.workrave.ConfigInterface", configurator.get());
      dbus->register_object_path(DBUS_PATH_WORKRAVE "Core");
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
  monitor = ActivityMonitor::create(configurator);
  monitor->init(display_name);
}


//! Initializes all breaks.
void
Core::init_breaks()
{
  breaks_control = BreaksControl::create(application, monitor, statistics, configurator);
  breaks_control->init();
}


//! Initializes the statistics.
void
Core::init_statistics()
{
  statistics = Statistics::create(shared_from_this());
  statistics->init();
}



/********************************************************************************/
/**** TimeSource interface                                                 ******/
/********************************************************************************/

gint64
Core::get_real_time()
{
  return current_real_time;
}

gint64
Core::get_monotonic_time() 
{
  return current_monotonic_time;
}


/********************************************************************************/
/**** Core Interface                                                       ******/
/********************************************************************************/

boost::signals2::signal<void(OperationMode)> &
Core::signal_operation_mode_changed()
{
  return operation_mode_changed_signal;
}


boost::signals2::signal<void(UsageMode)> &
Core::signal_usage_mode_changed()
{
  return usage_mode_changed_signal;
}


//! Forces the start of the specified break.
void
Core::force_break(BreakId id, BreakHint break_hint)
{
  breaks_control->force_break(id, break_hint);
}


//! Returns the specified break controller.
IBreak::Ptr
Core::get_break(BreakId id)
{
  return breaks_control->get_break(id);
}


//! Returns the statistics.
IStatistics::Ptr
Core::get_statistics() const
{
  return statistics;
}


//! Returns the configurator.
IConfigurator::Ptr
Core::get_configurator() const
{
  return configurator;
}


//! Is the user currently active?
bool
Core::is_user_active() const
{
  return monitor->get_current_state() == ACTIVITY_ACTIVE;
}

//! Retrieves the operation mode.
OperationMode
Core::get_operation_mode()
{
    return operation_mode;
}


//! Retrieves the regular operation mode.
OperationMode
Core::get_operation_mode_regular()
{
    /* operation_mode_regular is the same as operation_mode unless there's an 
    override in place, in which case operation_mode is the current override mode and 
    operation_mode_regular is the mode that will be restored once all overrides are removed
    */
    return operation_mode_regular;
}


//! Checks if operation_mode is an override.
bool
Core::is_operation_mode_an_override()
{
    return !!operation_mode_overrides.size();
}


//! Sets the operation mode.
void
Core::set_operation_mode(OperationMode mode)
{
    set_operation_mode_internal( mode, true );
}


//! Temporarily overrides the operation mode.
void
Core::set_operation_mode_override( OperationMode mode, const std::string &id )
{
    if( !id.size() )
        return;

    set_operation_mode_internal( mode, false, id );
}


//! Removes the overriden operation mode.
void
Core::remove_operation_mode_override( const std::string &id )
{
    TRACE_ENTER( "Core::remove_operation_mode_override" );

    if( !id.size() || !operation_mode_overrides.count( id ) )
        return;

    operation_mode_overrides.erase( id );

    /* If there are other overrides still in the queue then pass in the first 
    override in the map. set_operation_mode_internal() will then search the 
    map for the most important override and set it as the active operation mode.
    */
    if( operation_mode_overrides.size() )
    {
        set_operation_mode_internal( 
            operation_mode_overrides.begin()->second, 
            false, 
            operation_mode_overrides.begin()->first
            );
    }
    else
    {
        /* if operation_mode_regular is the same as the active operation mode then just 
        signal the mode has changed. During overrides the signal is not sent so it needs to 
        be sent now. Because the modes are the same it would not be called by 
        set_operation_mode_internal().
        */
        if( operation_mode_regular == operation_mode )
        {
            TRACE_MSG( "Only calling core_event_operation_mode_changed()." );

            operation_mode_changed_signal(operation_mode_regular);
        }
        else
            set_operation_mode_internal( operation_mode_regular, false );
    }

    TRACE_EXIT();
}


//! Set the operation mode.
void
Core::set_operation_mode_internal(
    OperationMode mode, 
    bool persistent, 
    const std::string &override_id /* default param: empty string */
    )
{
  TRACE_ENTER_MSG("Core::set_operation_mode", ( persistent ? "persistent" : "" ) );

  if( override_id.size() )
  {
      TRACE_MSG( "override_id: " << override_id );
  }

  TRACE_MSG( "Incoming/requested mode is "
      << ( mode == OPERATION_MODE_NORMAL ? "OPERATION_MODE_NORMAL" :
            mode == OPERATION_MODE_SUSPENDED ? "OPERATION_MODE_SUSPENDED" :
                mode == OPERATION_MODE_QUIET ? "OPERATION_MODE_QUIET" : "???" )
      << ( override_id.size() ? " (override)" : " (regular)" )
      );

  TRACE_MSG( "Current mode is "
      << ( mode == OPERATION_MODE_NORMAL ? "OPERATION_MODE_NORMAL" :
            mode == OPERATION_MODE_SUSPENDED ? "OPERATION_MODE_SUSPENDED" :
                mode == OPERATION_MODE_QUIET ? "OPERATION_MODE_QUIET" : "???" )
      << ( operation_mode_overrides.size() ? " (override)" : " (regular)" )
      );
  
  if( ( mode != OPERATION_MODE_NORMAL )
      && ( mode != OPERATION_MODE_QUIET )
      && ( mode != OPERATION_MODE_SUSPENDED )
      )
  {
      TRACE_RETURN( "No change: incoming invalid" );
      return;
  }

  /* If the incoming operation mode is regular and the current operation mode is an 
  override then save the incoming operation mode and return.
  */
  if( !override_id.size() && operation_mode_overrides.size() )
  {
      operation_mode_regular = mode;

      int cm;
      if( persistent 
          && ( !get_configurator()->get_value( CoreConfig::CFG_KEY_OPERATION_MODE, cm ) 
              || ( cm != mode ) )
          )
          get_configurator()->set_value( CoreConfig::CFG_KEY_OPERATION_MODE, mode );

      TRACE_RETURN( "No change: current is an override type but incoming is regular" );
      return;
  }
  
  // If the incoming operation mode is tagged as an override
  if( override_id.size() )
  {
      // Add this override to the map
      operation_mode_overrides[ override_id ] = mode;

      /* Find the most important override. Override modes in order of importance:
      OPERATION_MODE_SUSPENDED, OPERATION_MODE_QUIET, OPERATION_MODE_NORMAL
      */
      for( map<std::string, OperationMode>::iterator i = operation_mode_overrides.begin();
          ( i != operation_mode_overrides.end() );
          ++i
          )
      {
          if( i->second == OPERATION_MODE_SUSPENDED )
          {
              mode = OPERATION_MODE_SUSPENDED;
              break;
          }

          if( ( i->second == OPERATION_MODE_QUIET )
              && ( mode == OPERATION_MODE_NORMAL )
              )
          {
              mode = OPERATION_MODE_QUIET;
          }
      }
  }


  if (operation_mode != mode)
  {
      TRACE_MSG( "Changing active operation mode to "
          << ( mode == OPERATION_MODE_NORMAL ? "OPERATION_MODE_NORMAL" :
                mode == OPERATION_MODE_SUSPENDED ? "OPERATION_MODE_SUSPENDED" :
                    mode == OPERATION_MODE_QUIET ? "OPERATION_MODE_QUIET" : "???" )
          );

      OperationMode previous_mode = operation_mode;

      operation_mode = mode;
      breaks_control->set_operation_mode(mode);
      
      if( !operation_mode_overrides.size() )
          operation_mode_regular = operation_mode;

      if (operation_mode == OPERATION_MODE_SUSPENDED)
      {
          TRACE_MSG("Force idle");
          monitor->suspend();
          breaks_control->stop_all_breaks();
          breaks_control->set_insensitive_mode(INSENSITIVE_MODE_IDLE_ALWAYS);
      }
      else if (previous_mode == OPERATION_MODE_SUSPENDED)
      {
          // stop_all_breaks again will reset insensitive mode (that is good)
          breaks_control->stop_all_breaks();
          monitor->resume();
      }

      if (operation_mode == OPERATION_MODE_QUIET)
      {
          breaks_control->stop_all_breaks();
      }

      if( !operation_mode_overrides.size() )
      {
          /* The two functions in this block will trigger signals that can call back into this function.
          Only if there are no overrides in place will that reentrancy be ok from here.
          Otherwise the regular/user mode to restore would be overwritten.
          */

          if( persistent )
              get_configurator()->set_value( CoreConfig::CFG_KEY_OPERATION_MODE, operation_mode );

          operation_mode_changed_signal(operation_mode);
      }
  }

  TRACE_EXIT();
}


//! Retrieves the usage mode.
UsageMode
Core::get_usage_mode()
{
  return usage_mode;
}


//! Sets the usage mode.
void
Core::set_usage_mode(UsageMode mode)
{
  set_usage_mode_internal(mode, true);
}


//! Sets the usage mode.
void
Core::set_usage_mode_internal(UsageMode mode, bool persistent)
{
  if (usage_mode != mode)
    {
      usage_mode = mode;

      breaks_control->set_usage_mode(mode);

      if (persistent)
        {
          get_configurator()->set_value(CoreConfig::CFG_KEY_USAGE_MODE, mode);
        }

      usage_mode_changed_signal(mode);
    }
}


//! Sets the insist policy.
/*!
 *  The insist policy determines what to do when the user is active while
 *  taking a break.
 */
void
Core::set_insist_policy(ICore::InsistPolicy p)
{
  breaks_control->set_insist_policy(p);
}


//! Forces all monitors to be idle.
void
Core::force_idle()
{
  monitor->force_idle();
}


/********************************************************************************/
/****                                                                      ******/
/********************************************************************************/

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

  if (key == CoreConfig::CFG_KEY_OPERATION_MODE)
    {
      int mode;
      if (! get_configurator()->get_value(CoreConfig::CFG_KEY_OPERATION_MODE, mode))
        {
          mode = OPERATION_MODE_NORMAL;
        }
      TRACE_MSG("Setting operation mode");
      set_operation_mode_internal(OperationMode(mode), false);
    }

  if (key == CoreConfig::CFG_KEY_USAGE_MODE)
    {
      int mode;
      if (! get_configurator()->get_value(CoreConfig::CFG_KEY_USAGE_MODE, mode))
        {
          mode = USAGE_MODE_NORMAL;
        }
      TRACE_MSG("Setting usage mode");
      set_usage_mode_internal(UsageMode(mode), false);
    }
  TRACE_EXIT();
}


//! Periodic heartbeat.
void
Core::heartbeat()
{
  TRACE_ENTER("Core::heartbeat");
  assert(application != NULL);

  // Set current time.
  current_real_time = g_get_real_time();
  current_monotonic_time = g_get_monotonic_time();
  
  TRACE_MSG("Time = " << current_real_time << " " << current_monotonic_time);

  // Process configuration
  configurator->heartbeat();

  // Perform state computation.
  monitor->heartbeat();

  // Process breaks
  breaks_control->heartbeat();

  TRACE_EXIT();
}


//!
void
Core::report_external_activity(std::string who, bool act)
{
  monitor->report_external_activity(who, act);
}


//! Loads miscellaneous
void
Core::load_config()
{
  configurator->rename_key("gui/operation-mode", CoreConfig::CFG_KEY_OPERATION_MODE);
  configurator->add_listener(CoreConfig::CFG_KEY_OPERATION_MODE, this);
  configurator->add_listener(CoreConfig::CFG_KEY_USAGE_MODE, this);

  int mode;
  if (! get_configurator()->get_value(CoreConfig::CFG_KEY_OPERATION_MODE, mode))
    {
      mode = OPERATION_MODE_NORMAL;
    }
  set_operation_mode(OperationMode(mode));

  if (! get_configurator()->get_value(CoreConfig::CFG_KEY_USAGE_MODE, mode))
    {
      mode = USAGE_MODE_NORMAL;
    }
  set_usage_mode(UsageMode(mode));
}
