// Copyright (C) 2001 - 2013 Ray Satiro & Rob Caelers & Raymond Penners
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

#include "CoreModes.hh"
#include "CoreConfig.hh"

using namespace std;

CoreModes::Ptr
CoreModes::create(ActivityMonitor::Ptr monitor, IConfigurator::Ptr config)
{
  return Ptr(new CoreModes(monitor, config));
}


CoreModes::CoreModes(ActivityMonitor::Ptr monitor, IConfigurator::Ptr config) :
  operation_mode(OperationMode::Normal),
  operation_mode_regular(OperationMode::Normal),
  usage_mode(UsageMode::Normal),
  monitor(monitor),
  config(config)
{
  TRACE_ENTER("CoreModes::CoreModes");
  load_config();
  TRACE_EXIT();
}


CoreModes::~CoreModes()
{
  TRACE_ENTER("CoreModes::~CoreModes");
  TRACE_EXIT();
}


boost::signals2::signal<void(OperationMode)> &
CoreModes::signal_operation_mode_changed()
{
  return operation_mode_changed_signal;
}


boost::signals2::signal<void(UsageMode)> &
CoreModes::signal_usage_mode_changed()
{
  return usage_mode_changed_signal;
}


//! Retrieves the operation mode.
OperationMode
CoreModes::get_operation_mode()
{
    TRACE_ENTER("CoreModes::get_operation_mode");
    TRACE_EXIT();
    return operation_mode;
}


//! Retrieves the regular operation mode.
OperationMode
CoreModes::get_operation_mode_regular()
{
    /* operation_mode_regular is the same as operation_mode unless there's an 
    override in place, in which case operation_mode is the current override mode and 
    operation_mode_regular is the mode that will be restored once all overrides are removed
    */
    return operation_mode_regular;
}


//! Checks if operation_mode is an override.
bool
CoreModes::is_operation_mode_an_override()
{
    return !!operation_mode_overrides.size();
}


//! Sets the operation mode.
void
CoreModes::set_operation_mode(OperationMode mode)
{
    set_operation_mode_internal( mode, true );
}


//! Temporarily overrides the operation mode.
void
CoreModes::set_operation_mode_override( OperationMode mode, const std::string &id )
{
    if( !id.size() )
        return;

    set_operation_mode_internal( mode, false, id );
}


//! Removes the overriden operation mode.
void
CoreModes::remove_operation_mode_override( const std::string &id )
{
    TRACE_ENTER( "CoreModes::remove_operation_mode_override" );

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
CoreModes::set_operation_mode_internal(
    OperationMode mode, 
    bool persistent, 
    const std::string &override_id /* default param: empty string */
    )
{
  TRACE_ENTER_MSG("CoreModes::set_operation_mode", ( persistent ? "persistent" : "" ) );

  if( override_id.size() )
  {
      TRACE_MSG( "override_id: " << override_id );
  }

  TRACE_MSG( "Incoming/requested mode is "
      << ( mode == OperationMode::Normal ? "OperationMode::Normal" :
            mode == OperationMode::Suspended ? "OperationMode::Suspended" :
                mode == OperationMode::Quiet ? "OperationMode::Quiet" : "???" )
      << ( override_id.size() ? " (override)" : " (regular)" )
      );

  TRACE_MSG( "Current mode is "
      << ( operation_mode == OperationMode::Normal ? "OperationMode::Normal" :
            operation_mode == OperationMode::Suspended ? "OperationMode::Suspended" :
                operation_mode == OperationMode::Quiet ? "OperationMode::Quiet" : "???" )
      << ( operation_mode_overrides.size() ? " (override)" : " (regular)" )
      );
  
  if( ( mode != OperationMode::Normal )
      && ( mode != OperationMode::Quiet )
      && ( mode != OperationMode::Suspended )
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

      operation_mode_changed_signal(operation_mode);

      OperationMode cm = CoreConfig::operation_mode()();
      if (persistent  && (cm != mode))
        {
          CoreConfig::operation_mode().set(mode);
        }

      TRACE_RETURN( "No change: current is an override type but incoming is regular" );
      return;
  }
  
  // If the incoming operation mode is tagged as an override
  if( override_id.size() )
  {
      // Add this override to the map
      operation_mode_overrides[ override_id ] = mode;

      /* Find the most important override. Override modes in order of importance:
      OperationMode::Suspended, OperationMode::Quiet, OperationMode::Normal
      */
      for( map<std::string, OperationMode>::iterator i = operation_mode_overrides.begin();
          ( i != operation_mode_overrides.end() );
          ++i
          )
      {
          if( i->second == OperationMode::Suspended )
          {
              mode = OperationMode::Suspended;
              break;
          }

          if( ( i->second == OperationMode::Quiet )
              && ( mode == OperationMode::Normal )
              )
          {
              mode = OperationMode::Quiet;
          }
      }
  }


  if (operation_mode != mode)
  {
      TRACE_MSG( "Changing active operation mode to "
          << ( mode == OperationMode::Normal ? "OperationMode::Normal" :
                mode == OperationMode::Suspended ? "OperationMode::Suspended" :
                    mode == OperationMode::Quiet ? "OperationMode::Quiet" : "???" )
          );

      OperationMode previous_mode = operation_mode;

      operation_mode = mode;
      
      if( !operation_mode_overrides.size() )
          operation_mode_regular = operation_mode;

      if (operation_mode == OperationMode::Suspended)
      {
          monitor->suspend();
      }
      else if (previous_mode == OperationMode::Suspended)
      {
          monitor->resume();
      }

      if( !operation_mode_overrides.size() )
      {
          /* The two functions in this block will trigger signals that can call back into this function.
          Only if there are no overrides in place will that reentrancy be ok from here.
          Otherwise the regular/user mode to restore would be overwritten.
          */

          if( persistent )
            {
              CoreConfig::operation_mode().set(operation_mode);
            }

          TRACE_MSG("Send event");
          operation_mode_changed_signal(operation_mode);
      }
  }

  TRACE_EXIT();
}


//! Retrieves the usage mode.
UsageMode
CoreModes::get_usage_mode()
{
  return usage_mode;
}


//! Sets the usage mode.
void
CoreModes::set_usage_mode(UsageMode mode)
{
  set_usage_mode_internal(mode, true);
}


//! Sets the usage mode.
void
CoreModes::set_usage_mode_internal(UsageMode mode, bool persistent)
{
  if (usage_mode != mode)
    {
      usage_mode = mode;

      if (persistent)
        {
          CoreConfig::usage_mode().set(mode);
        }

      usage_mode_changed_signal(mode);

    }
}

//! Loads miscellaneous
void
CoreModes::load_config()
{
  config->rename_key("gui/operation-mode", CoreConfig::operation_mode().key());
  config->add_listener(CoreConfig::operation_mode().key(), this);
  config->add_listener(CoreConfig::usage_mode().key(), this);

  OperationMode operation_mode = CoreConfig::operation_mode()();
  set_operation_mode(operation_mode);

  UsageMode usage_mode = CoreConfig::usage_mode()();
  set_usage_mode(usage_mode);
}


//! Notification that the configuration has changed.
void
CoreModes::config_changed_notify(const string &key)
{
  TRACE_ENTER_MSG("Core::config_changed_notify", key);
  string::size_type pos = key.find('/');
  string path;

  if (pos != string::npos)
    {
      path = key.substr(0, pos);
    }

  if (key == CoreConfig::operation_mode().key())
    {
      OperationMode operation_mode = CoreConfig::operation_mode()();
      TRACE_MSG("Setting operation mode");
      set_operation_mode_internal(operation_mode, false);
    }

  if (key == CoreConfig::usage_mode().key())
    {
      UsageMode usage_mode = CoreConfig::usage_mode()();
      TRACE_MSG("Setting usage mode");
      set_usage_mode_internal(usage_mode, false);
    }
  TRACE_EXIT();
}
