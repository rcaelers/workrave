// BreaksControl.hh --- The main controller
//
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

#include <vector>
#include <map>
#include <boost/enable_shared_from_this.hpp>

#include "config/Config.hh"
#include "dbus/IDBus.hh"
#include "utils/ScopedConnections.hh"

#include "Break.hh"
#include "Timer.hh"

#include "ICore.hh"
#include "CoreModes.hh"
#include "CoreHooks.hh"
#include "Statistics.hh"
#include "ReadingActivityMonitor.hh"
#include "TimerActivityMonitor.hh"

using namespace workrave;

class BreaksControl :
  public boost::enable_shared_from_this<BreaksControl>
{
public:
  typedef boost::shared_ptr<BreaksControl> Ptr;

  static Ptr create(IApp *app,
                    IActivityMonitor::Ptr activity_monitor,
                    CoreModes::Ptr modes,
                    Statistics::Ptr statistics,
                    workrave::dbus::IDBus::Ptr dbus,
                    CoreHooks::Ptr hooks);
  
  BreaksControl(IApp *app,
                IActivityMonitor::Ptr activity_monitor,
                CoreModes::Ptr modes,
                Statistics::Ptr statistics,
                workrave::dbus::IDBus::Ptr dbus,
                CoreHooks::Ptr hooks);
  virtual ~BreaksControl();

  void init();
  void heartbeat();
  void save_state() const;

  void force_break(BreakId id, BreakHint break_hint);

  IBreak::Ptr get_break(BreakId id);

  void set_insist_policy(InsistPolicy p);

private:
  void set_freeze_all_breaks(bool freeze);
  void process_timers(bool user_is_active);
  void start_break(BreakId break_id, BreakId resume_this_break = BREAK_ID_NONE);
  void load_state();
  void defrost();
  void freeze();
  void force_idle();
  void stop_all_breaks();

  void on_operation_mode_changed(const OperationMode m);
  void on_break_event(BreakId break_id, BreakEvent event);
  
private:
  IApp *application;

  IActivityMonitor::Ptr activity_monitor;
  ReadingActivityMonitor::Ptr reading_activity_monitor;
  TimerActivityMonitor::Ptr microbreak_activity_monitor;

  CoreModes::Ptr modes;
  Statistics::Ptr statistics;
  workrave::dbus::IDBus::Ptr dbus;
  CoreHooks::Ptr hooks;
  
  Break::Ptr breaks[workrave::BREAK_ID_SIZEOF];
  Timer::Ptr timers[workrave::BREAK_ID_SIZEOF];
  
  InsistPolicy insist_policy;
  InsistPolicy active_insist_policy;

  scoped_connections connections;
};

#endif // BREAKSCONTROL_HH
