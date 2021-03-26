// Copyright (C) 2007, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef MACOSINPUTMONITOR_HH
#define MACOSINPUTMONITOR_HH

#include <thread>

// #include <CoreFoundation/CoreFoundation.h>
// #include <IOKit/IOKitLib.h>

#include "InputMonitor.hh"
#include "input-monitor/IInputMonitorListener.hh"

class MacOSInputMonitor : public InputMonitor
{
public:
  MacOSInputMonitor() = default;
  ~MacOSInputMonitor() override;

  bool init() override;
  void terminate() override;
  void run();

private:
  uint64_t get_event_count();

private:
  bool terminate_loop = false;
  std::shared_ptr<std::thread> monitor_thread;
  int last_event_count = 0;
};

#endif // MACOSINPUTMONITOR_HH
