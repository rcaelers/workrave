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
  mutex = g_mutex_new();
  cond = g_cond_new();
}


XScreenSaverMonitor::~XScreenSaverMonitor()
{
  TRACE_ENTER("XScreenSaverMonitor::~XScreenSaverMonitor");
  if (monitor_thread != NULL)
    {
      monitor_thread->wait();
    }

  g_mutex_free(mutex);
  g_cond_free(cond);
  TRACE_EXIT();
}


bool
XScreenSaverMonitor::init()
{
  int event_base;
  int error_base;

  Bool has_extension = XScreenSaverQueryExtension(gdk_x11_display_get_xdisplay(gdk_display_get_default()), &event_base, &error_base);

  if (has_extension)
  {
    screen_saver_info = XScreenSaverAllocInfo();
    monitor_thread->start();
  }
  
  return has_extension;
}

void
XScreenSaverMonitor::terminate()
{
  TRACE_ENTER("XScreenSaverMonitor::terminate");

  g_mutex_lock(mutex);
  abort = true;
  g_cond_broadcast(cond);
  g_mutex_unlock(mutex);  
  
  monitor_thread->wait();
  monitor_thread = NULL;
  
  TRACE_EXIT();
}


void
XScreenSaverMonitor::run()
{
  TRACE_ENTER("XScreenSaverMonitor::run");

  g_mutex_lock(mutex);
  while (!abort)
    {
      XScreenSaverQueryInfo(gdk_x11_display_get_xdisplay(gdk_display_get_default()), gdk_x11_get_default_root_xwindow(), screen_saver_info);

      if (screen_saver_info->idle < 1000)
        {
          /* Notify the activity monitor */
          fire_action();
        }

#if GLIB_CHECK_VERSION(2, 32, 0)
      gint64 end_time = g_get_monotonic_time() + G_TIME_SPAN_SECOND;
      g_cond_wait_until(cond, mutex, end_time);
#else
      g_usleep(500000);
#endif      
    }
  g_mutex_unlock(mutex);  
  
  TRACE_EXIT();
}
