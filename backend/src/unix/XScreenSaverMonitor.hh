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

#include "Runnable.hh"
#include "Thread.hh"

//! Activity monitor for a local X server.
class XScreenSaverMonitor :
  public InputMonitor,
  public Runnable
{
public:
  //! Constructor.
  XScreenSaverMonitor();

  //! Destructor.
  virtual ~XScreenSaverMonitor();

  //! Initialize
  virtual bool init();

  //! Terminate the monitor.
  virtual void terminate();

private:
  //! The monitor's execution thread.
  virtual void run();


private:
  //! Abort the main loop
  bool abort;

  //! The activity monitor thread.
  Thread *monitor_thread;

  //
  XScreenSaverInfo *screen_saver_info;

  GMutex *mutex;
  GCond *cond;
};

#endif // XSCREENSAVERMONITOR_HH
