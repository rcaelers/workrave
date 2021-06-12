// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef READINGACTIVITYMONITOR_HH
#define READINGACTIVITYMONITOR_HH

#include <memory>

#include "Break.hh"
#include "CoreModes.hh"
#include "IActivityMonitor.hh"
#include "Timer.hh"

#include "utils/Signals.hh"

class ReadingActivityMonitor
  : public IActivityMonitorListener
  , public std::enable_shared_from_this<ReadingActivityMonitor>
  , public workrave::utils::Trackable
{
public:
  using Ptr = std::shared_ptr<ReadingActivityMonitor>;

public:
  ReadingActivityMonitor(IActivityMonitor::Ptr monitor, CoreModes::Ptr modes);
  ~ReadingActivityMonitor() override = default;

  void handle_break_event(workrave::BreakId break_id, workrave::BreakEvent event);

  void init();
  void suspend();
  void resume();
  void force_idle();
  bool is_active();

private:
  bool action_notify() override;
  void on_usage_mode_changed(workrave::UsageMode mode);

private:
  enum State
  {
    Idle,
    Active,
    Prelude,
    Taking
  };

  IActivityMonitor::Ptr monitor;
  CoreModes::Ptr modes;
  bool suspended;
  bool forced_idle;
  State state;
};

#endif // READINGACTIVITYMONITOR_HH
