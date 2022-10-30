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
#  include "config.h"
#endif

#include "XScreenSaverMonitor.hh"

#include <memory>

#if defined(HAVE_GTK)
#  include <gdk/gdkx.h>
#endif

#include "input-monitor/IInputMonitorListener.hh"
#include "utils/Platform.hh"
#include "debug.hh"

using namespace std;
using namespace workrave::utils;

XScreenSaverMonitor::~XScreenSaverMonitor()
{
  TRACE_ENTRY();
  if (monitor_thread)
    {
      monitor_thread->join();
    }
}

bool
XScreenSaverMonitor::init()
{
  TRACE_ENTRY();
  int event_base;
  int error_base;

  xdisplay = static_cast<Display *>(Platform::get_default_display());
  root = reinterpret_cast<Drawable>(Platform::get_default_root_window());

  Bool has_extension = False;
  if (xdisplay != nullptr)
    {
      TRACE_MSG("xdisplay ok");
      has_extension = XScreenSaverQueryExtension(xdisplay, &event_base, &error_base);
    }

  if (has_extension)
    {

      screen_saver_info = XScreenSaverAllocInfo();
      monitor_thread = std::make_shared<std::thread>([this] { run(); });
    }

  TRACE_VAR(has_extension);
  return has_extension;
}

void
XScreenSaverMonitor::terminate()
{
  TRACE_ENTRY();
  mutex.lock();
  abort = true;
  cond.notify_all();
  mutex.unlock();

  monitor_thread->join();
}

void
XScreenSaverMonitor::run()
{
  TRACE_ENTRY();
  {
    std::unique_lock lock(mutex);
    while (!abort)
      {
        XScreenSaverQueryInfo(xdisplay, root, screen_saver_info);

        if (screen_saver_info->idle < 1000)
          {
            TRACE_MSG("action");
            /* Notify the activity monitor */
            fire_action();
          }

        cond.wait_for(lock, std::chrono::milliseconds(1000));
      }
  }
}
