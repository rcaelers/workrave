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

#ifndef CORE_HH
#define CORE_HH

#include <string>

#include "config/IConfigurator.hh"
#include "utils/Signals.hh"

#include "core/ICore.hh"
#include "LocalActivityMonitor.hh"
#include "BreaksControl.hh"
#include "Statistics.hh"
#include "CoreHooks.hh"
#include "CoreModes.hh"

// Forward declarion of external interface.
namespace workrave
{
  class IApp;
}

#if defined(HAVE_GRPC) && defined(HAVE_CORE_NEXT)
class RpcCoreServer;
#endif
#if defined(HAVE_CORE_NEXT_DBUS)
class RpcDBusServer;
#endif

// @rpc(service="workrave.CoreService")
// @rpc.dbus(interface="org.workrave.CoreInterface")
class Core : public workrave::ICore
{
public:
  explicit Core(workrave::config::IConfigurator::Ptr configurator);
  ~Core() override;

  // ICore
  // @rpc.signal(name="OperationModeChanged")
  boost::signals2::signal<void(workrave::OperationMode)> &signal_operation_mode_changed() override;
  // @rpc.signal(name="UsageModeChanged")
  boost::signals2::signal<void(workrave::UsageMode)> &signal_usage_mode_changed() override;
  void init(workrave::IApp *application, const char *display_name) override;
  void heartbeat() override;
  // @rpc(name="ForceBreak")
  void force_break(workrave::BreakId id, workrave::utils::Flags<workrave::BreakHint> break_hint) override;
  workrave::IBreak::Ptr get_break(workrave::BreakId id) const override;
  workrave::IStatistics::Ptr get_statistics() const override;
  ICoreHooks::Ptr get_hooks() const override;
  // @rpc(name="IsActive")
  bool is_user_active() const override;
  // @rpc(name="IsTaking")
  bool is_taking() const override;
  // @rpc(name="GetActiveOperationMode")
  workrave::OperationMode get_active_operation_mode() override;
  // @rpc(name="GetOperationMode")
  workrave::OperationMode get_regular_operation_mode() override;
  // @rpc(name="IsOperationModeAnOverride")
  bool is_operation_mode_an_override() override;
  // @rpc(name="SetOperationMode")
  void set_operation_mode(workrave::OperationMode mode) override;
  // @rpc(name="SetOperationModeFor")
  void set_operation_mode_for(workrave::OperationMode mode, std::chrono::minutes duration) override;
  void set_operation_mode_override(workrave::OperationMode mode, const std::string &id) override;
  void remove_operation_mode_override(const std::string &id) override;
  // @rpc(name="GetUsageMode")
  workrave::UsageMode get_usage_mode() override;
  // @rpc(name="SetUsageMode")
  void set_usage_mode(workrave::UsageMode mode) override;
  void set_powersave(bool down) override;
  void set_insist_policy(workrave::InsistPolicy p) override;
  void force_idle() override;

  // DBus/RPC functions.
  // @rpc(name="ReportActivity")
  void report_external_activity(std::string who, bool act) override;

#if defined(HAVE_GRPC) && defined(HAVE_CORE_NEXT)
  // The per-object Break service registry; forwards to BreaksControl so
  // whoever wires up the RpcServer (see init_rpc()) can
  // construct a workrave::core::rpc::BreakServiceServiceImpl without reaching
  // into BreaksControl directly.
  rpc::InstanceRegistry<workrave::BreakId, Break> &get_break_registry();
#endif

private:
#if defined(HAVE_GRPC) && defined(HAVE_CORE_NEXT)
  void init_rpc();
  void update_rpc();
#endif
#if defined(HAVE_CORE_NEXT_DBUS)
  void init_rpc_dbus();
#endif

private:
  //! List of breaks.
  BreaksControl::Ptr breaks_control;

  //! The Configurator.
  workrave::config::IConfigurator::Ptr configurator;

  //! The activity monitor
  LocalActivityMonitor::Ptr monitor;

  //! Hooks to alter the backend behaviour.
  CoreHooks::Ptr hooks;

  //!
  CoreModes::Ptr core_modes;

  //! GUI Widget factory.
  workrave::IApp *application{nullptr};

  //! The statistics collector.
  Statistics::Ptr statistics;

  //! Did the OS announce a powersave?
  bool powersave{false};

#if defined(HAVE_GRPC) && defined(HAVE_CORE_NEXT)
  std::string rpc_listen_address;

  // Holds references into breaks_control/configurator and must stop serving
  // before those are torn down.
  std::unique_ptr<RpcCoreServer> rpc_server;

  // Declared after rpc_server so setting callbacks are disconnected before
  // the server and the objects referenced by those callbacks are destroyed.
  workrave::utils::Trackable rpc_settings_tracker;
#endif
#if defined(HAVE_CORE_NEXT_DBUS)
  // Declared last so signal forwarding and DBus object registrations stop
  // before the Core/Break/Configurator instances they reference disappear.
  std::unique_ptr<RpcDBusServer> rpc_dbus_server;
#endif
};

#endif // CORE_HH
