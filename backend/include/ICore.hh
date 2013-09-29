// ICore.hh --- The main controller interface
//
// Copyright (C) 2001 - 2009, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_BACKEND_ICORE_HH
#define WORKRAVE_BACKEND_ICORE_HH

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

#include "CoreTypes.hh"

#include "config/IConfigurator.hh"
#include "dbus/IDBus.hh"

#include "IBreak.hh"
#include "IStatistics.hh"
#include "ICoreHooks.hh"

namespace workrave
{
  // Forward declaratons
  class IApp;

  //! Main interface of the backend.
  class ICore
  {
  public:
    typedef boost::shared_ptr<ICore> Ptr;
    
    virtual ~ICore() {}

    static ICore::Ptr create();

    //! The way a break is insisted.
    enum InsistPolicy
      {
        //! Uninitialized policy
        INSIST_POLICY_INVALID,

        //! Halts the timer on activity.
        INSIST_POLICY_HALT,

        //! Resets the timer on activity.
        INSIST_POLICY_RESET,

        //! Ignores all activity.
        INSIST_POLICY_IGNORE,
      };

    virtual boost::signals2::signal<void(OperationMode)> &signal_operation_mode_changed() = 0;
    virtual boost::signals2::signal<void(UsageMode)> &signal_usage_mode_changed() = 0;
    
    //! Initialize the Core. Must be called first.
    virtual void init(IApp *app, const std::string &display) = 0;

    //! Periodic heartbeat. The GUI *MUST* call this method every second.
    virtual void heartbeat() = 0;

    //! Force a break of the specified type.
    virtual void force_break(BreakId id, BreakHint break_hint) = 0;

    //! Return the break interface of the specified type.
    virtual IBreak::Ptr get_break(BreakId id) = 0;

    //! Return the statistics interface.
    virtual IStatistics::Ptr get_statistics() const = 0;

    //! Is the user currently active?
    virtual bool is_user_active() const = 0;

    //! Retrieves the operation mode.
    virtual OperationMode get_operation_mode() = 0;

    //! Retrieves the regular operation mode.
    virtual OperationMode get_operation_mode_regular() = 0;

    //! Sets the operation mode.
    virtual void set_operation_mode(OperationMode mode) = 0;

    //! Temporarily overrides the operation mode.
    virtual void set_operation_mode_override(OperationMode mode, const std::string &id) = 0;

    //! Removes the overriden operation mode.
    virtual void remove_operation_mode_override(const std::string &id) = 0;

    //! Checks if operation_mode is an override.
    virtual bool is_operation_mode_an_override() = 0;

    //! Return the current usage mode.
    virtual UsageMode get_usage_mode() = 0;

    //! Set the usage mode.
    virtual void set_usage_mode(UsageMode mode) = 0;

    //! Notify the core that the computer will enter or leave powersave (suspend/hibernate)
    virtual void set_powersave(bool down) = 0;

    //! Set the break insist policy.
    virtual void set_insist_policy(InsistPolicy p) = 0;

    //! Forces all breaks timers to become idle.
    virtual void force_idle() = 0;

    //! Return configuration
    virtual config::IConfigurator::Ptr get_configurator() const = 0;

    //! Return the hooks
    virtual ICoreHooks::Ptr get_hooks() const = 0;

    //! Return DBUs remoting interface.
    virtual dbus::IDBus::Ptr get_dbus() const = 0;
  };

  std::string operator%(const std::string &key, BreakId id);
};

#endif // WORKRAVE_BACKEND_ICORE_HH
