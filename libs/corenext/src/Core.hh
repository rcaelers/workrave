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

#include "dbus/IDBus.hh"
#include "config/IConfigurator.hh"

#include "core/ICore.hh"
#include "LocalActivityMonitor.hh"
#include "BreaksControl.hh"
#include "Statistics.hh"
#include "CoreHooks.hh"
#include "CoreModes.hh"
#include "CoreDBus.hh"

// Forward declarion of external interface.
namespace workrave
{
  class IApp;
}

#if defined(HAVE_RPC)
class RpcCoreServer;
#endif

// @rpc(service="CoreService")
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
  std::shared_ptr<workrave::dbus::IDBus> get_dbus() const override;
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
  void report_external_activity(std::string who, bool act);

#if defined(HAVE_RPC)
  // The gRPC analog of BreakDBus's per-object-path registration; forwards to
  // BreaksControl so whoever wires up the RpcServer (see init_rpc()) can
  // construct a BreakServiceServiceImpl without reaching into BreaksControl
  // directly.
  rpc::InstanceRegistry<workrave::BreakId, Break> &get_break_registry();
#endif

private:
  void init_bus();
#if defined(HAVE_RPC)
  void init_rpc();
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

  //!
  CoreDBus::Ptr core_dbus;

  //! GUI Widget factory.
  workrave::IApp *application{nullptr};

  //! The statistics collector.
  Statistics::Ptr statistics;

  //! Did the OS announce a powersave?
  bool powersave{false};

  //! DBUS bridge
  std::shared_ptr<workrave::dbus::IDBus> dbus;

#if defined(HAVE_RPC)
  // Declared last so it is destroyed first: it holds references into
  // breaks_control/configurator and must stop serving before those are torn
  // down.
  std::unique_ptr<RpcCoreServer> rpc_server;
#endif
};

#endif // CORE_HH
