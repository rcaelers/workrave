// Copyright (C) 2002, 2004, 2006, 2007, 2012 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef W32INPUTMONITOR_HH
#define W32INPUTMONITOR_HH

#include <windows.h>

#include "InputMonitor.hh"

#include "config/IConfigurator.hh"
#include "crash/CrashReporter.hh"

typedef union HarpoonEventUnion HarpoonEvent;

using namespace workrave::config;

class W32InputMonitor
  : public InputMonitor
  , public workrave::crash::CrashHandler
{
public:
  W32InputMonitor(workrave::config::IConfigurator::Ptr config);
  virtual ~W32InputMonitor();

  bool init() override;
  void terminate() override;

  void on_crashed() override;

private:
  static W32InputMonitor *singleton;
  static void on_harpoon_event(HarpoonEvent *event);

  workrave::config::IConfigurator::Ptr config;
};

#endif // W32INPUTMONITOR_HH
