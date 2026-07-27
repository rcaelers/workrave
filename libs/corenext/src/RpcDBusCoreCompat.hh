// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKRAVE_CORENEXT_RPC_DBUS_CORE_COMPAT_HH
#define WORKRAVE_CORENEXT_RPC_DBUS_CORE_COMPAT_HH

#include <boost/signals2.hpp>
#include <string>

#include "Core.hh"

namespace workrave::core::rpc::dbus_compat
{
  // Exact C++ facade for the legacy org.workrave.CoreInterface wire API.
  // Keeping this separate from Core's richer gRPC annotations prevents DBus
  // compatibility details from narrowing or hiding the modern RPC surface.
  // @rpc(service="LegacyCoreDBusService")
  // @rpc.dbus(interface="org.workrave.CoreInterface")
  class CoreCompat
  {
  public:
    explicit CoreCompat(::Core &core);

    // @rpc(name="SetOperationMode")
    void set_operation_mode(workrave::OperationMode mode);
    // @rpc(name="GetOperationMode")
    workrave::OperationMode get_operation_mode();
    // @rpc(name="SetUsageMode")
    void set_usage_mode(workrave::UsageMode mode);
    // @rpc(name="GetUsageMode")
    workrave::UsageMode get_usage_mode();
    // @rpc(name="ReportActivity")
    void report_activity(std::string who, bool active);
    // @rpc(name="IsActive")
    bool is_active() const;

    // @rpc.signal(name="OperationModeChanged")
    boost::signals2::signal<void(workrave::OperationMode)> &signal_operation_mode_changed();
    // @rpc.signal(name="UsageModeChanged")
    boost::signals2::signal<void(workrave::UsageMode)> &signal_usage_mode_changed();

  private:
    ::Core &core_;
  };
} // namespace workrave::core::rpc::dbus_compat

#endif // WORKRAVE_CORENEXT_RPC_DBUS_CORE_COMPAT_HH
