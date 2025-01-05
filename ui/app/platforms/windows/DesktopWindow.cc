// Copyright (C) 2004, 2007, 2012, 2013 Rob Caelers & Raymond Penners
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

#include "ui/windows/DesktopWindow.hh"
#include "ui/windows/WindowsCompat.hh"
#include "debug.hh"

const char *const DesktopWindow::WINDOW_CLASS = "WorkraveDesktopWindow";

bool DesktopWindow::initialized = false;

DesktopWindow::DesktopWindow(int x, int y, int width, int height)
{
  TRACE_ENTRY();
  init();
  auto *hinstance = (HINSTANCE)GetModuleHandle(nullptr);

  POINT pt = {x, y};
  HMONITOR monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
  if (monitor != nullptr)
    {
      MONITORINFO info = {
        0,
      };
      info.cbSize = sizeof(info);

      if (GetMonitorInfo(monitor, &info))
        {
          x = info.rcMonitor.left;
          y = info.rcMonitor.top;
          width = info.rcMonitor.right - x;
          height = info.rcMonitor.bottom - y;
        }
    }

  hwnd = CreateWindowExA(WS_EX_TOOLWINDOW,
                         WINDOW_CLASS,
                         WINDOW_CLASS,
                         WS_POPUP,
                         x,
                         y,
                         width,
                         height,
                         (HWND) nullptr,
                         (HMENU) nullptr,
                         hinstance,
                         (LPSTR) nullptr);
  SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

DesktopWindow::~DesktopWindow()
{
  DestroyWindow(hwnd);
}

LRESULT CALLBACK
DesktopWindow::window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto *self = (DesktopWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  switch (uMsg)
    {
    case WM_WINDOWPOSCHANGED:
      InvalidateRect(self->hwnd, nullptr, TRUE);
      return 0;

    case WM_ERASEBKGND:
      PaintDesktop((HDC)wParam);
      return 1;

    default:
      break;
    }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void
DesktopWindow::init()
{
  if (initialized)
    {
      return;
    }

  auto *windows_hinstance = (HINSTANCE)GetModuleHandle(nullptr);

  WNDCLASSEXA wclass =
    {sizeof(WNDCLASSEX), 0, window_proc, 0, 0, windows_hinstance, nullptr, nullptr, nullptr, nullptr, WINDOW_CLASS, nullptr};
  RegisterClassExA(&wclass);
  initialized = true;
}

void
DesktopWindow::set_visible(bool visible)
{
  ShowWindow(hwnd, visible ? SW_SHOW : SW_HIDE);
  if (visible)
    {
      WindowsCompat::SetWindowOnTop(hwnd, TRUE);
    }
}
