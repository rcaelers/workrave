// DesktopWindow.cc --- Desktop window
//
// Copyright (C) 2004 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#include "DesktopWindow.hh"
#include "W32Compat.hh"
#include "debug.hh"

const char * const DesktopWindow::WINDOW_CLASS = "WorkraveDesktopWindow";

bool DesktopWindow::initialized = false;

DesktopWindow::DesktopWindow(const HeadInfo &head)
{
  TRACE_ENTER("DesktopWindow::DesktopWindow");
  init();
  HINSTANCE hinstance = (HINSTANCE) GetModuleHandle(NULL);

  int x = head.get_x(), y = head.get_y();
  int w = head.get_width(), h = head.get_height();

  if (! W32Compat::IsWindows95())
    {
      POINT pt = { x, y };
      HMONITOR monitor = W32Compat::MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
      if (monitor)
        {
          TRACE_MSG("Monitor found");
          MONITORINFO info;
          info.cbSize = sizeof(MONITORINFO);
          
          if (W32Compat::GetMonitorInfo(monitor, &info))
            {
              x = info.rcMonitor.left;
              y = info.rcMonitor.top;
              w = info.rcMonitor.right - x;
              h = info.rcMonitor.bottom - y;
              TRACE_MSG("Monitor dimensions: " << x << ", " << y << ", " << w << ", " << h);
            }
        }
    }

  
  hwnd = CreateWindowEx(0,
                        WINDOW_CLASS,
                        WINDOW_CLASS,
                        WS_POPUP,
                        x, y, w, h,
                        (HWND)NULL,
                        (HMENU)NULL,
                        hinstance,
                        (LPSTR)NULL);
  SetWindowLong(hwnd, GWL_USERDATA, (LONG) this);

  TRACE_EXIT();
}

DesktopWindow::~DesktopWindow()
{
  DestroyWindow(hwnd);
}

LRESULT CALLBACK
DesktopWindow::window_proc(HWND hwnd, UINT uMsg, WPARAM wParam,
                           LPARAM lParam)
{
  DesktopWindow *self = (DesktopWindow *) GetWindowLong(hwnd, GWL_USERDATA);
  switch (uMsg)
    {
    case WM_WINDOWPOSCHANGED:
      InvalidateRect(self->hwnd, NULL, TRUE);
      return 0;
      
    case WM_ERASEBKGND:
      PaintDesktop((HDC) wParam);
      return 1;
    }
  
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void
DesktopWindow::init()
{
  if (initialized)
    return;
  
  HINSTANCE win32_hinstance = (HINSTANCE) GetModuleHandle(NULL);
  
  WNDCLASSEX wclass =
    {
      sizeof(WNDCLASSEX),
      0,
      window_proc,
      0,
      0,
      win32_hinstance,
      NULL,
      NULL,
      NULL,
      NULL,
      WINDOW_CLASS,
      NULL
    };
  RegisterClassEx(&wclass);
  initialized = true;
}

void
DesktopWindow::set_visible(bool visible)
{
  ShowWindow(hwnd, visible ? SW_SHOW : SW_HIDE);
}
