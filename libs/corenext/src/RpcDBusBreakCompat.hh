// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKRAVE_CORENEXT_RPC_DBUS_BREAK_COMPAT_HH
#define WORKRAVE_CORENEXT_RPC_DBUS_BREAK_COMPAT_HH

#include <boost/signals2.hpp>
#include <cstdint>
#include <string>

#include "Break.hh"

namespace workrave::core::rpc::dbus_compat
{
  // Exact C++ facade for the legacy org.workrave.BreakInterface wire API.
  // In particular, the old DBus API exposed timer values as int32_t even
  // though the native and gRPC APIs use int64_t.
  // @rpc(service="LegacyBreakDBusService")
  // @rpc.dbus(interface="org.workrave.BreakInterface")
  class BreakCompat
  {
  public:
    explicit BreakCompat(::Break &break_controller);

    // @rpc(name="IsTimerRunning")
    bool is_timer_running() const;
    // @rpc(name="GetTimerIdle")
    int32_t get_timer_idle() const;
    // @rpc(name="GetTimerElapsed")
    int32_t get_timer_elapsed() const;
    // @rpc(name="GetTimerRemaining")
    int32_t get_timer_remaining() const;
    // @rpc(name="GetTimerOverdue")
    int32_t get_timer_overdue() const;
    // @rpc(name="PostponeBreak")
    void postpone_break();
    // @rpc(name="SkipBreak")
    void skip_break();
    // @rpc(name="GetBreakState")
    std::string get_break_state() const;

    // @rpc.signal(name="BreakStateChanged")
    boost::signals2::signal<void(std::string)> &signal_break_state_changed();
    // @rpc.signal(name="BreakEvent")
    boost::signals2::signal<void(workrave::BreakEvent)> &signal_break_event();

  private:
    ::Break &break_;
    boost::signals2::signal<void(std::string)> break_state_changed_;
    boost::signals2::scoped_connection break_stage_connection_;
  };
} // namespace workrave::core::rpc::dbus_compat

#endif // WORKRAVE_CORENEXT_RPC_DBUS_BREAK_COMPAT_HH
