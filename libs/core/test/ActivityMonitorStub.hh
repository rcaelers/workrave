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

#ifndef ACTIVITYMONITORSTUB_HH
#define ACTIVITYMONITORSTUB_HH

#include <memory>

#include "IActivityMonitorListener.hh"
#include "IActivityMonitor.hh"

class ActivityMonitorStub : public IActivityMonitor
{
public:
  using Ptr = std::shared_ptr<ActivityMonitorStub>;

public:
  ActivityMonitorStub();
  ~ActivityMonitorStub() override = default;

  void set_active(bool active);

  // void init() override;
  void terminate() override;
  void suspend() override;
  void resume() override;
  void force_idle() override;
  // bool is_active() override;
  // void set_listener(IActivityMonitorListener::Ptr l) override;
  void set_listener(IActivityMonitorListener *l) override;
  ActivityState get_current_state() override;

  void notify();
  void heartbeat();

private:
  bool active{false};
  bool suspended{false};
  bool forced_idle{false};
  int count{0};
  // IActivityMonitorListener::Ptr listener;
  IActivityMonitorListener *listener{nullptr};
};

#endif // LOCALACTIVITYMONITOR_HH
