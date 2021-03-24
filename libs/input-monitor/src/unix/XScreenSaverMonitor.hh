// XScreenSaverMonitor.hh --- ActivityMonitor for X11
//
// Copyright (C) 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef XSCREENSAVERMONITOR_HH
#define XSCREENSAVERMONITOR_HH

#include <string>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>

#include "InputMonitor.hh"

#include "utils/Runnable.hh"
#include "utils/Thread.hh"

//! Activity monitor for a local X server.
class XScreenSaverMonitor
  : public InputMonitor
  , public Runnable
{
public:
  //! Constructor.
  XScreenSaverMonitor();

  //! Destructor.
  ~XScreenSaverMonitor() override;

  //! Initialize
  bool init() override;

  //! Terminate the monitor.
  void terminate() override;

private:
  //! The monitor's execution thread.
  void run() override;

private:
  //! Abort the main loop
  bool abort{false};

  //! The activity monitor thread.
  Thread *monitor_thread;

  //
  XScreenSaverInfo *screen_saver_info{nullptr};

  GMutex mutex{};
  GCond cond{};
};

#endif // XSCREENSAVERMONITOR_HH
