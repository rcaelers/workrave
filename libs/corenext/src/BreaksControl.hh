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

#ifndef BREAKSCONTROL_HH
#define BREAKSCONTROL_HH

#include "config/Config.hh"
#include "dbus/IDBus.hh"

#include "Break.hh"
#include "Timer.hh"

#include "core/ICore.hh"
#include "CoreModes.hh"
#include "CoreHooks.hh"
#include "Statistics.hh"
#include "ReadingActivityMonitor.hh"
#include "TimerActivityMonitor.hh"

class BreaksControl
  : public std::enable_shared_from_this<BreaksControl>
  , public workrave::utils::Trackable
{
public:
  using Ptr = std::shared_ptr<BreaksControl>;

  BreaksControl(workrave::IApp *app,
                IActivityMonitor::Ptr activity_monitor,
                CoreModes::Ptr modes,
                Statistics::Ptr statistics,
                std::shared_ptr<workrave::dbus::IDBus> dbus,
                CoreHooks::Ptr hooks);
  virtual ~BreaksControl();

  void init();
  void heartbeat();
  void save_state() const;

  void force_break(workrave::BreakId id, workrave::utils::Flags<workrave::BreakHint> break_hint);

  workrave::IBreak::Ptr get_break(workrave::BreakId id);

  void set_insist_policy(workrave::InsistPolicy p);

private:
  void set_freeze_all_breaks(bool freeze);
  void process_timers(bool user_is_active);
  void start_break(workrave::BreakId break_id, workrave::BreakId resume_this_break = workrave::BREAK_ID_NONE);
  void load_state();
  void defrost();
  void freeze();
  void force_idle();
  void stop_all_breaks();

  void on_operation_mode_changed(workrave::OperationMode operation_mode);
  void on_break_event(workrave::BreakId break_id, workrave::BreakEvent event);

private:
  workrave::IApp *application;

  IActivityMonitor::Ptr activity_monitor;
  ReadingActivityMonitor::Ptr reading_activity_monitor;
  TimerActivityMonitor::Ptr microbreak_activity_monitor;

  CoreModes::Ptr modes;
  Statistics::Ptr statistics;
  std::shared_ptr<workrave::dbus::IDBus> dbus;
  CoreHooks::Ptr hooks;

  Break::Ptr breaks[workrave::BREAK_ID_SIZEOF];
  Timer::Ptr timers[workrave::BREAK_ID_SIZEOF];

  workrave::InsistPolicy insist_policy;
  workrave::InsistPolicy active_insist_policy;
};

#endif // BREAKSCONTROL_HH
