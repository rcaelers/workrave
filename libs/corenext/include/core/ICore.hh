// Copyright (C) 2001 - 2013 Rob Caelers <robc@krandor.nl>
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

#include <chrono>
#include <memory>
#include <string>
#include <boost/signals2.hpp>

#include "config/IConfigurator.hh"
#include "core/CoreTypes.hh"
#include "core/IBreak.hh"
#include "core/ICoreHooks.hh"
#include "core/IStatistics.hh"
#include "dbus/IDBus.hh"

namespace workrave
{
  class IApp;

  //! Main interface of the backend.
  class ICore
  {
  public:
    using Ptr = std::shared_ptr<ICore>;
    virtual ~ICore() = default;

    virtual boost::signals2::signal<void(workrave::OperationMode)> &signal_operation_mode_changed() = 0;
    virtual boost::signals2::signal<void(workrave::UsageMode)> &signal_usage_mode_changed() = 0;

    //! Initialize the Core. Must be called first.
    virtual void init(IApp *app, const char *display) = 0;

    //! Periodic heartbeat. The GUI *MUST* call this method every second.
    virtual void heartbeat() = 0;

    //! Force a break of the specified type.
    virtual void force_break(BreakId id, workrave::utils::Flags<BreakHint> break_hint) = 0;

    //! Return the break interface of the specified type.
    [[nodiscard]] virtual IBreak::Ptr get_break(BreakId id) const = 0;

    //! Return the statistics interface.
    [[nodiscard]] virtual IStatistics::Ptr get_statistics() const = 0;

    //! Is the user currently active?
    [[nodiscard]] virtual bool is_user_active() const = 0;

    //! Is the user taking a break?
    [[nodiscard]] virtual bool is_taking() const = 0;

    //! Retrieves the operation mode.
    [[nodiscard]] virtual OperationMode get_active_operation_mode() = 0;

    //! Retrieves the regular operation mode.
    [[nodiscard]] virtual OperationMode get_regular_operation_mode() = 0;

    //! Sets the operation mode.
    virtual void set_operation_mode(OperationMode mode) = 0;

    //! Sets the operation mode.
    virtual void set_operation_mode_for(OperationMode mode, std::chrono::minutes duration) = 0;

    //! Temporarily overrides the operation mode.
    virtual void set_operation_mode_override(OperationMode mode, const std::string &id) = 0;

    //! Removes the overridden operation mode.
    virtual void remove_operation_mode_override(const std::string &id) = 0;

    //! Checks if operation_mode is an override.
    [[nodiscard]] virtual bool is_operation_mode_an_override() = 0;

    //! Return the current usage mode.
    [[nodiscard]] virtual UsageMode get_usage_mode() = 0;

    //! Set the usage mode.
    virtual void set_usage_mode(UsageMode mode) = 0;

    //! Notify the core that the computer will enter or leave powersave (suspend/hibernate)
    virtual void set_powersave(bool down) = 0;

    //! Set the break insist policy.
    virtual void set_insist_policy(InsistPolicy p) = 0;

    //! Forces all breaks timers to become idle.
    virtual void force_idle() = 0;

    //! Return the hooks
    [[nodiscard]] virtual ICoreHooks::Ptr get_hooks() const = 0;

    //! Return DBUs remoting interface.
    [[nodiscard]] virtual std::shared_ptr<workrave::dbus::IDBus> get_dbus() const = 0;
  };

  class CoreFactory
  {
  public:
    static ICore::Ptr create(workrave::config::IConfigurator::Ptr configurator);
  };
}; // namespace workrave

#endif // WORKRAVE_BACKEND_ICORE_HH
