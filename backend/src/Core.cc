// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#include <boost/filesystem.hpp>

#include "Core.hh"

#include "config/ConfiguratorFactory.hh"
#include "config/IConfigurator.hh"
#include "utils/TimeSource.hh"
#include "input-monitor/InputMonitorFactory.hh"

#include "utils/AssetPath.hh"
#include "IApp.hh"
#include "Break.hh"
#include "CoreConfig.hh"
#include "Statistics.hh"

#include "dbus/IDBus.hh"
#ifdef HAVE_DBUS
#include "DBusWorkrave.hh"
#define DBUS_PATH_WORKRAVE         "/org/workrave/Workrave/"
#define DBUS_SERVICE_WORKRAVE      "org.workrave.Workrave"
#endif

using namespace workrave::dbus;


using namespace workrave::utils;

ICore::Ptr
ICore::create()
{
  return Ptr(new Core());
}


//! Constructs a new Core.
Core::Core() :
  application(NULL),
  powersave(false)
{
  TRACE_ENTER("Core::Core");
  hooks = CoreHooks::create();
  TimeSource::sync();
  TRACE_EXIT();
}


//! Destructor.
Core::~Core()
{
  TRACE_ENTER("Core::~Core");

  if (monitor)
    {
      monitor->terminate();
    }

  TRACE_EXIT();
}


//! Initializes the core.
void
Core::init(IApp *app, const string &display_name)
{
  application = app;

  dbus = IDBus::create();
  dbus->init();
  
  init_configurator();

#ifdef HAVE_TESTS
  if (!hooks->hook_create_monitor().empty())
    {
      monitor = hooks->hook_create_monitor()();
    }
  else
#endif
    {
      // LCOV_EXCL_START
      monitor = LocalActivityMonitor::create(display_name);
      // LCOV_EXCL_STOP
   }
  
  monitor->init();

  statistics = Statistics::create(monitor);
  statistics->init();
  
  core_modes = CoreModes::create(monitor);

  core_dbus = CoreDBus::create(core_modes, dbus);
  
  breaks_control = BreaksControl::create(application, monitor, core_modes, statistics, dbus, hooks);
  breaks_control->init();

  init_bus();
}



//! Initializes the configurator.
void
Core::init_configurator()
{
  string ini_file = AssetPath::complete_directory("workrave.ini", AssetPath::SEARCH_PATH_CONFIG);

#ifdef HAVE_TESTS
  if (!hooks->hook_create_configurator().empty())
    {
      configurator = hooks->hook_create_configurator()();
    }
  else
#endif

  // LCOV_EXCL_START
  if (boost::filesystem::is_regular_file(ini_file))
    {
      configurator = ConfiguratorFactory::create(ConfiguratorFactory::FormatIni);
      configurator->load(ini_file);
    }
  else
    {
      configurator = ConfiguratorFactory::create(ConfiguratorFactory::FormatNative);

      if (configurator == NULL)
        {
          string configFile = AssetPath::complete_directory("config.xml", AssetPath::SEARCH_PATH_CONFIG);
          configurator = ConfiguratorFactory::create(ConfiguratorFactory::FormatXml);

          if (configurator)
            {
#if defined(PLATFORM_OS_UNIX)
              if (configFile == "" || configFile == "config.xml")
                {
                  configFile = AssetPath::get_home_directory() + "config.xml";
                }
#endif
              if (configFile != "")
                {
                  configurator->load(configFile);
                }
            }
        }

      if (configurator == NULL)
        {
          ini_file = AssetPath::get_home_directory() + "workrave.ini";
          configurator = ConfiguratorFactory::create(ConfiguratorFactory::FormatIni);

          if (configurator)
            {
              configurator->load(ini_file);
              configurator->save(ini_file);
            }
        }
    }
  
  CoreConfig::init(configurator);

  string home = CoreConfig::general_datadir()();
  if (home != "")
    {
      AssetPath::set_home_directory(home);
    }
  // LCOV_EXCL_STOP

}


//! Initializes the communication bus.
void
Core::init_bus()
{
  try
    {

#ifdef HAVE_DBUS
      extern void init_DBusWorkrave(IDBus::Ptr dbus);
      init_DBusWorkrave(dbus);

      dbus->connect(DBUS_PATH_WORKRAVE "Core", "org.workrave.CoreInterface", this);
      dbus->connect(DBUS_PATH_WORKRAVE "Core", "org.workrave.ConfigInterface", configurator.get());
      dbus->register_object_path(DBUS_PATH_WORKRAVE "Core");
#endif
    }
  catch (DBusException &)
    {
    }
}

//! Periodic heartbeat.
void
Core::heartbeat()
{
  TRACE_ENTER("Core::heartbeat");

  TimeSource::sync();
  
  // Process configuration
  configurator->heartbeat();

  // Process breaks
  breaks_control->heartbeat();

  TRACE_EXIT();
}


/********************************************************************************/
/**** ICore Interface                                                      ******/
/********************************************************************************/

boost::signals2::signal<void(OperationMode)> &
Core::signal_operation_mode_changed()
{
  return core_modes->signal_operation_mode_changed();
}


boost::signals2::signal<void(UsageMode)> &
Core::signal_usage_mode_changed()
{
  return core_modes->signal_usage_mode_changed();
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


//!
ICoreHooks::Ptr
Core::get_hooks() const
{
  return hooks;
}


dbus::IDBus::Ptr
Core::get_dbus() const
{
  return dbus;
}


//! Is the user currently active?
bool
Core::is_user_active() const
{
  return monitor->is_active();
}


//! Retrieves the operation mode.
OperationMode
Core::get_operation_mode()
{
  return core_modes->get_operation_mode();
}


//! Retrieves the regular operation mode.
OperationMode
Core::get_operation_mode_regular()
{
  return core_modes->get_operation_mode_regular();
}


//! Checks if operation_mode is an override.
bool
Core::is_operation_mode_an_override()
{
  return core_modes->is_operation_mode_an_override();
}


//! Sets the operation mode.
void
Core::set_operation_mode(OperationMode mode)
{
  core_modes->set_operation_mode(mode);
}


//! Temporarily overrides the operation mode.
void
Core::set_operation_mode_override( OperationMode mode, const std::string &id )
{
  core_modes->set_operation_mode_override(mode, id);
}


//! Removes the overriden operation mode.
void
Core::remove_operation_mode_override( const std::string &id )
{
  core_modes->remove_operation_mode_override(id);
}


//! Retrieves the usage mode.
UsageMode
Core::get_usage_mode()
{
  return core_modes->get_usage_mode();
}


//! Sets the usage mode.
void
Core::set_usage_mode(UsageMode mode)
{
  core_modes->set_usage_mode(mode);
}


//! Sets the insist policy.
/*!
 *  The insist policy determines what to do when the user is active while
 *  taking a break.
 */
void
Core::set_insist_policy(InsistPolicy p)
{
  breaks_control->set_insist_policy(p);
}


//! Forces all monitors to be idle.
void
Core::force_idle()
{
  monitor->force_idle();
}


//! Announces a powersave state.
void
Core::set_powersave(bool down)
{
  TRACE_ENTER_MSG("Core::set_powersave", down);
  TRACE_MSG(powersave << " " << core_modes->get_operation_mode());

  if (down)
    {
      if (!powersave)
        {
          // Computer is going down
          set_operation_mode_override(OperationMode::Suspended, "powersave");
          powersave = true;
        }
      
      breaks_control->save_state();
      statistics->update();
    }
  else
    {
      remove_operation_mode_override("powersave");
      powersave = false;
    }
  TRACE_EXIT();
}

void
Core::report_external_activity(std::string who, bool act)
{
  (void) who;
  (void) act;
  // TODO: fix this
  // monitor->report_external_activity(who, act);
}
