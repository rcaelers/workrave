// Icon.cpp --- Icon
//
// Copyright (C) 2004, 2005, 2007 Raymond Penners <raymond@dotsphinx.com>
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
// $Id$

#include <stdio.h>

#include "Icon.h"
#include "DeskBand.h"

#define ICON_CLASS_NAME "WorkraveIcon"

Icon::Icon(HWND parent, HINSTANCE hinst, const char *resource, CDeskBand *deskband)
  : deskband(deskband)
{
  init(hinst);
  icon = LoadIcon(hinst, resource);
  hwnd = CreateWindowEx(0, ICON_CLASS_NAME, "",
                        WS_CHILD | WS_CLIPSIBLINGS, 0, 0, 16, 16, parent, NULL, hinst, NULL);
  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG)this);
}

Icon::~Icon()
{
  DestroyWindow(hwnd);
}

LRESULT CALLBACK
Icon::wnd_proc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  Icon  *pThis = (Icon*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

  switch (uMessage)
    {
    case WM_PAINT:
      return pThis->on_paint();

    case WM_LBUTTONUP:
      SendMessage(pThis->deskband->get_command_window(), WM_USER + 1, 0, NULL);
      break;
    }
  return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

void
Icon::get_size(int &w, int &h) const
{
  w = 16;
  h = 16;
}



LRESULT
Icon::on_paint(void)
{
  PAINTSTRUCT ps;

  BeginPaint(hwnd, &ps);
  HDC dc = GetDC(hwnd);
  DrawIconEx(dc, 0, 0, icon, 16, 16, 0, NULL, DI_NORMAL);
  ReleaseDC(hwnd, dc);
  EndPaint(hwnd, &ps);

  return 0;
}

void
Icon::init(HINSTANCE hinst)
{
  //If the window class has not been registered, then do so.
  WNDCLASS wc;
  if(!GetClassInfo(hinst, ICON_CLASS_NAME, &wc))
    {
      ZeroMemory(&wc, sizeof(wc));
      wc.style          = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
      wc.lpfnWndProc    = (WNDPROC)wnd_proc;
      wc.cbClsExtra     = 0;
      wc.cbWndExtra     = 0;
      wc.hInstance      = hinst;
      wc.hIcon          = NULL;
      wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground  = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
      wc.lpszMenuName   = NULL;
      wc.lpszClassName  = ICON_CLASS_NAME;

      RegisterClass(&wc);
    }
}
