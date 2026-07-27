// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "RpcDBusCoreCompat.hh"

#include <utility>

namespace workrave::core::rpc::dbus_compat
{
  CoreCompat::CoreCompat(::Core &core)
    : core_(core)
  {
  }

  void CoreCompat::set_operation_mode(workrave::OperationMode mode)
  {
    core_.set_operation_mode(mode);
  }
  workrave::OperationMode CoreCompat::get_operation_mode()
  {
    return core_.get_regular_operation_mode();
  }
  void CoreCompat::set_usage_mode(workrave::UsageMode mode)
  {
    core_.set_usage_mode(mode);
  }
  workrave::UsageMode CoreCompat::get_usage_mode()
  {
    return core_.get_usage_mode();
  }
  void CoreCompat::report_activity(std::string who, bool active)
  {
    core_.report_external_activity(std::move(who), active);
  }
  bool CoreCompat::is_active() const
  {
    return core_.is_user_active();
  }

  boost::signals2::signal<void(workrave::OperationMode)> &CoreCompat::signal_operation_mode_changed()
  {
    return core_.signal_operation_mode_changed();
  }

  boost::signals2::signal<void(workrave::UsageMode)> &CoreCompat::signal_usage_mode_changed()
  {
    return core_.signal_usage_mode_changed();
  }
} // namespace workrave::core::rpc::dbus_compat
