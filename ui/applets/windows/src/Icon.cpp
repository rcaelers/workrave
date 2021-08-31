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
#include "Debug.h"
#include "PaintHelper.h"

#define ICON_CLASS_NAME "WorkraveIcon"

Icon::Icon(HWND parent, HINSTANCE hinst, const char *resource, int size, CDeskBand *deskband)
  : deskband(deskband)
  , size(size)
{
  init(hinst);
  icon = (HICON)LoadImage(hinst, resource, IMAGE_ICON, size, size, LR_DEFAULTCOLOR);
  hwnd = CreateWindowEx(0, ICON_CLASS_NAME, "", WS_CHILD | WS_CLIPSIBLINGS, 0, 0, size, size, parent, NULL, hinst, (LPVOID)this);

  paint_helper = new PaintHelper(hwnd);
}

Icon::~Icon()
{
  DestroyWindow(hwnd);
  delete paint_helper;
}

LRESULT CALLBACK
Icon::wnd_proc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  TRACE_ENTER("Icon::WndProc");
  LRESULT lResult = 0;
  Icon *pThis = (Icon *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

  switch (uMessage)
    {
    case WM_NCCREATE:
      {
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
        pThis = (Icon *)(lpcs->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
        SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
      }
      break;

    case WM_PAINT:
      pThis->on_paint();
      break;

    case WM_ERASEBKGND:
      if (PaintHelper::GetCompositionEnabled())
        {
          lResult = 1;
        }
      break;

    case WM_LBUTTONUP:
      SendMessage(pThis->deskband->get_command_window(), WM_USER + 1, 0, 0);
      break;
    }

  if (uMessage != WM_ERASEBKGND)
    {
      lResult = DefWindowProc(hWnd, uMessage, wParam, lParam);
    }

  TRACE_EXIT();
  return lResult;
}

void
Icon::get_size(int &w, int &h) const
{
  w = size;
  h = size;
}

LRESULT
Icon::on_paint()
{
  TRACE_ENTER("Icon::OnPaint");

  HDC dc = paint_helper->BeginPaint();
  if (dc != NULL)
    {
      paint_helper->DrawIcon(0, 0, icon, size, size);
      paint_helper->EndPaint();
    }
  TRACE_EXIT();
  return 0;
}

void
Icon::init(HINSTANCE hinst)
{
  // If the window class has not been registered, then do so.
  WNDCLASS wc;
  if (!GetClassInfo(hinst, ICON_CLASS_NAME, &wc))
    {
      ZeroMemory(&wc, sizeof(wc));
      wc.style = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
      wc.lpfnWndProc = (WNDPROC)wnd_proc;
      wc.cbClsExtra = 0;
      wc.cbWndExtra = 0;
      wc.hInstance = hinst;
      wc.hIcon = NULL;
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
      wc.lpszMenuName = NULL;
      wc.lpszClassName = ICON_CLASS_NAME;

      RegisterClass(&wc);
    }
}

void
Icon::update()
{
  TRACE_ENTER("Icon::update");
  InvalidateRect(hwnd, NULL, FALSE);
  TRACE_EXIT();
}
