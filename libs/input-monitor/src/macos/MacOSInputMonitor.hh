// MacOSInputMonitor.hh --- ActivityMonitor for MacOS
//
// Copyright (C) 2007 Rob Caelers <robc@krandor.nl>
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

#ifndef MacOSINPUTMONITOR_HH
#define MacOSINPUTMONITOR_HH

#if TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
#else
#  if HAVE_SYS_TIME_H
#    include <sys/time.h>
#  else
#    include <time.h>
#  endif
#endif

#ifdef HAVE_IOKIT
#  include <CoreFoundation/CoreFoundation.h>
#  include <IOKit/IOKitLib.h>
#endif

#include "InputMonitor.hh"
#include "IInputMonitorListener.hh"
#include "Runnable.hh"
#include "Thread.hh"

//! Activity monitor for MacOS.
class MacOSInputMonitor
  : public InputMonitor
  , public Runnable
{
public:
  //! Constructor.
  MacOSInputMonitor();

  //! Destructor.
  virtual ~MacOSInputMonitor();

  bool init();
  void terminate();
  void run();

private:
  //! Terminate driver?
  bool terminate_loop;

  //! The activity monitor thread.
  Thread *monitor_thread;

  // OS X IO Service
  io_service_t io_service;
};

#endif // MacOSINPUTMONITOR_HH
