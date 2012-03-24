// XScreenSaverMonitor.cc --- ActivityMonitor for X11
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include <gdk/gdkx.h>

#include "XScreenSaverMonitor.hh"

#include "Core.hh"
#include "ICore.hh"
#include "ICoreEventListener.hh"
#include "IInputMonitorListener.hh"

#include "Thread.hh"

using namespace std;
using namespace workrave;

XScreenSaverMonitor::XScreenSaverMonitor() :
  abort(false),
  screen_saver_info(NULL)
{
  monitor_thread = new Thread(this);
}


XScreenSaverMonitor::~XScreenSaverMonitor()
{
  TRACE_ENTER("XScreenSaverMonitor::~XScreenSaverMonitor");
  if (monitor_thread != NULL)
    {
      monitor_thread->wait();
      delete monitor_thread;
    }

  TRACE_EXIT();
}


bool
XScreenSaverMonitor::init()
{
  monitor_thread->start();

	int event_base;
  int error_base;

  Bool has_extension = XScreenSaverQueryExtension(gdk_x11_display_get_xdisplay(gdk_display_get_default()), &event_base, &error_base);

	if (has_extension)
	{
    screen_saver_info = XScreenSaverAllocInfo();
	}
  
  return has_extension;
}

void
XScreenSaverMonitor::terminate()
{
  TRACE_ENTER("XScreenSaverMonitor::terminate");

  abort = true;
  monitor_thread->wait();

  TRACE_EXIT();
}


void
XScreenSaverMonitor::run()
{
  TRACE_ENTER("XScreenSaverMonitor::run");

  while (!abort)
    {
      XScreenSaverQueryInfo(gdk_x11_display_get_xdisplay(gdk_display_get_default()), gdk_x11_get_default_root_xwindow(), screen_saver_info);

      if (screen_saver_info->idle < 1000)
        {
          /* Notify the activity monitor */
          fire_action();
        }
    }
  
  TRACE_EXIT();
}
