// XScreenSaverMonitor.cc --- ActivityMonitor for X11
//
// Copyright (C) 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_GTK
#include <gdk/gdkx.h>
#endif

#include "XScreenSaverMonitor.hh"

#include "input-monitor/IInputMonitorListener.hh"
#include "utils/Platform.hh"

using namespace std;
using namespace workrave::utils;

XScreenSaverMonitor::XScreenSaverMonitor() :
  abort(false),
  screen_saver_info(NULL)
{
}


XScreenSaverMonitor::~XScreenSaverMonitor()
{
  TRACE_ENTER("XScreenSaverMonitor::~XScreenSaverMonitor");
  if (monitor_thread)
    {
      monitor_thread->join();
    }
  TRACE_EXIT();
}


bool
XScreenSaverMonitor::init()
{
  TRACE_ENTER("XScreenSaverMonitor::init");
  int event_base;
  int error_base;

  xdisplay = static_cast<Display *>(Platform::get_default_display());
  root = reinterpret_cast<Drawable>(Platform::get_default_root_window());

  Bool has_extension = False;
  if (xdisplay != NULL)
    {
      TRACE_MSG("xdisplay ok");
      has_extension = XScreenSaverQueryExtension(xdisplay, &event_base, &error_base);
    }

  if (has_extension)
  {
    
    screen_saver_info = XScreenSaverAllocInfo();
    monitor_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&XScreenSaverMonitor::run, this)));
  }
  
  TRACE_RETURN(has_extension);
  return has_extension;
}

void
XScreenSaverMonitor::terminate()
{
  TRACE_ENTER("XScreenSaverMonitor::terminate");

  mutex.lock();
  abort = true;
  cond.notify_all();
  mutex.unlock();  
  
  monitor_thread->join();
  
  TRACE_EXIT();
}


void
XScreenSaverMonitor::run()
{
  TRACE_ENTER("XScreenSaverMonitor::run");


  {
    boost::mutex::scoped_lock lock(mutex);
    while (!abort)
      {
        XScreenSaverQueryInfo(xdisplay, root, screen_saver_info);
          
        if (screen_saver_info->idle < 1000)
          {
            TRACE_MSG("action");
            /* Notify the activity monitor */
            fire_action();
          }

        boost::system_time timeout = boost::get_system_time()+ boost::posix_time::milliseconds(1000);      
        cond.timed_wait(lock, timeout);
      }
  }
  
  TRACE_EXIT();
}
