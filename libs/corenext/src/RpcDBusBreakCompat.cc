// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "RpcDBusBreakCompat.hh"

#include <utility>

namespace workrave::core::rpc::dbus_compat
{
  BreakCompat::BreakCompat(::Break &break_controller)
    : break_(break_controller)
    , break_stage_connection_(break_.signal_break_stage_changed().connect([this](BreakStage stage) {
      std::string state = ::Break::get_stage_text(stage);
      if (!state.empty())
        {
          break_state_changed_(std::move(state));
        }
    }))
  {
  }

  bool BreakCompat::is_timer_running() const
  {
    return break_.is_running();
  }
  int32_t BreakCompat::get_timer_idle() const
  {
    return static_cast<int32_t>(break_.get_elapsed_idle_time());
  }
  int32_t BreakCompat::get_timer_elapsed() const
  {
    return static_cast<int32_t>(break_.get_elapsed_time());
  }
  int32_t BreakCompat::get_timer_remaining() const
  {
    return static_cast<int32_t>(break_.get_timer_remaining());
  }
  int32_t BreakCompat::get_timer_overdue() const
  {
    return static_cast<int32_t>(break_.get_total_overdue_time());
  }
  void BreakCompat::postpone_break()
  {
    break_.postpone_break();
  }
  void BreakCompat::skip_break()
  {
    break_.skip_break();
  }
  std::string BreakCompat::get_break_state() const
  {
    return break_.get_break_stage();
  }

  boost::signals2::signal<void(std::string)> &BreakCompat::signal_break_state_changed()
  {
    return break_state_changed_;
  }

  boost::signals2::signal<void(workrave::BreakEvent)> &BreakCompat::signal_break_event()
  {
    return break_.signal_break_event();
  }
} // namespace workrave::core::rpc::dbus_compat
