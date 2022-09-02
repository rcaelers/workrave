// Copyright (C) 2001 - 2021 Rob Caelers <robc@krandor.nl>
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

#include "ToolkitWindows.hh"

#ifndef PLATFORM_OS_WINDOWS_NATIVE
#  include <pbt.h>
#endif
#include <wtsapi32.h>
#include <dbt.h>
#include <windows.h>

#include "ui/GUIConfig.hh"
#include "debug.hh"

using namespace workrave;
using namespace workrave::config;

ToolkitWindows::ToolkitWindows(int argc, char **argv)
  : Toolkit(argc, argv)
{
#if defined(HAVE_HARPOON)
  locker = std::make_shared<WindowsHarpoonLocker>();
#else
  locker = std::make_shared<WindowsLocker>();
#endif
}

ToolkitWindows::~ToolkitWindows()
{
}

void
ToolkitWindows::init(std::shared_ptr<IApplicationContext> app)
{
  init_gui();

  Toolkit::init(app);

  init_filter();
}

void
ToolkitWindows::release()
{
  Toolkit::release();

#if 0
if (!main_window->is_visible())
    {
      GUIConfig::trayicon_enabled().set(true);
    }
#endif
  }

boost::signals2::signal<bool(MSG *msg), IToolkitWindows::event_combiner> &
ToolkitWindows::hook_event()
{
  return event_hook;
};

void
ToolkitWindows::init_gui()
{
}

void
ToolkitWindows::init_filter()
{
}

bool
ToolkitWindows::filter_func(MSG *msg)
{
  TRACE_ENTRY();
  switch (msg->message)
    {
    case WM_WTSSESSION_CHANGE:
      {
        if (msg->wParam == WTS_SESSION_LOCK)
          {
            signal_session_idle_changed()(true);
          }
        if (msg->wParam == WTS_SESSION_UNLOCK)
          {
            signal_session_idle_changed()(false);
            signal_session_unlocked()();
          }
      }
      break;

    case WM_POWERBROADCAST:
      {
        TRACE_MSG("WM_POWERBROADCAST {} {}", msg->wParam, msg->lParam);

        switch (msg->wParam)
          {
          case PBT_APMQUERYSUSPEND:
            TRACE_MSG("Query Suspend");
            break;

          case PBT_APMQUERYSUSPENDFAILED:
            TRACE_MSG("Query Suspend Failed");
            break;

          case PBT_APMRESUMESUSPEND:
          case PBT_APMRESUMEAUTOMATIC:
          case PBT_APMRESUMECRITICAL:
            {
              TRACE_MSG("Resume suspend");
              auto core = app->get_core();
              core->set_powersave(false);
            }
            break;

          case PBT_APMSUSPEND:
            {
              TRACE_MSG("Suspend");
              auto core = app->get_core();
              core->set_powersave(true);
            }
            break;
          }
      }
      break;

    case WM_DISPLAYCHANGE:
      {
        TRACE_MSG("WM_DISPLAYCHANGE {} {}", msg->wParam, msg->lParam);
      }
      break;

#ifndef HAVE_CORE_NEXT
    case WM_TIMECHANGE:
      {
        TRACE_MSG("WM_TIMECHANGE {} {}", msg->wParam, msg->lParam);
        auto core = app->get_core();
        core->time_changed();
      }
      break;
#endif

    case WM_DEVICECHANGE:
      {
        TRACE_MSG("WM_DEVICECHANGE {} {}", msg->wParam, msg->lParam);
        switch (msg->wParam)
          {
          case DBT_DEVICEARRIVAL:
          case DBT_DEVICEREMOVECOMPLETE:
            {
              HWND hwnd = FindWindowExA(NULL, NULL, "GdkDisplayChange", NULL);
              if (hwnd)
                {
                  SendMessage(hwnd, WM_DISPLAYCHANGE, 0, 0);
                }
            }
          default:
            break;
          }
        break;
      }
    }

  event_hook(msg);

  return true;
}

HWND
ToolkitWindows::get_event_hwnd() const
{
  return 0;
}

auto
ToolkitWindows::get_desktop_image() -> QPixmap
{
  QPixmap pixmap;
  return pixmap;
}

std::shared_ptr<Locker>
ToolkitWindows::get_locker()
{
  return locker;
}
