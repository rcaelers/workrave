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

const char * const DesktopWindow::WINDOW_CLASS = "WorkraveDesktopWindow";

bool DesktopWindow::initialized = false;

DesktopWindow::DesktopWindow(const HeadInfo &head)
{
  init();
  HINSTANCE hinstance = (HINSTANCE) GetModuleHandle(NULL);
  
  hwnd = CreateWindowEx(0,
                        WINDOW_CLASS,
                        WINDOW_CLASS,
                        WS_POPUP,
                        head.get_x(), head.get_y(),
                        head.get_width(), head.get_height(),
                        (HWND)NULL,
                        (HMENU)NULL,
                        hinstance,
                        (LPSTR)NULL);
  SetWindowLong(hwnd, GWL_USERDATA, (LONG) this);
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
