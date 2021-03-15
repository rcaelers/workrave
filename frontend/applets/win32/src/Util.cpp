// Util.cpp --- Utils
//
// Copyright (C) 2004, 2007, 2010 Raymond Penners <raymond@dotsphinx.com>
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

#include "Util.h"

static void
MapWindowRect(HWND hwnd, HWND parent, RECT *rect)
{
  GetClientRect(hwnd, rect);
  MapWindowPoints(hwnd, parent, (LPPOINT)rect, 2);
}

static void
TransparentHideWindow(HWND hwnd)
{
  if (IsWindowVisible(hwnd))
    {
      RECT r;
      HWND bg = GetParent(GetParent(hwnd));
      MapWindowRect(hwnd, bg, &r);
      ShowWindow(hwnd, SW_HIDE);
      InvalidateRect(bg, &r, TRUE);
    }
}

static void
TransparentPrepareShowWindow(HWND hwnd, int x, int y, BOOL repaint)
{
  if (IsWindowVisible(hwnd))
    {
      RECT r;
      POINT p;
      GetWindowRect(hwnd, &r);
      p.x = r.left;
      p.y = r.top;
      ScreenToClient(GetParent(hwnd), &p);
      if (repaint || p.x != x || p.y != y)
        {
          TransparentHideWindow(hwnd);
        }
    }
}

void
TransparentDamageControl::BeginPaint(BOOL rp)
{
  hide_windows_num = 0;
  show_windows_num = 0;
  repaint = rp;
}

void
TransparentDamageControl::HideWindow(HWND hwnd)
{
  if (hide_windows_num < TRANSPARENT_DAMAGE_CONTROL_BUF_SIZE)
    {
      hide_windows[hide_windows_num++] = hwnd;
    }
}

void
TransparentDamageControl::ShowWindow(HWND hwnd, int x, int y)
{
  if (hide_windows_num < TRANSPARENT_DAMAGE_CONTROL_BUF_SIZE)
    {
      ShowWindowData *d = &show_windows[show_windows_num++];
      d->hwnd = hwnd;
      d->x = x;
      d->y = y;
    }
}

void
TransparentDamageControl::EndPaint()
{
  for (int h = 0; h < hide_windows_num; h++)
    {
      TransparentHideWindow(hide_windows[h]);
    }
  for (int s = 0; s < show_windows_num; s++)
    {
      ShowWindowData *d = &show_windows[s];
      TransparentPrepareShowWindow(d->hwnd, d->x, d->y, repaint);
    }
  for (int p = 0; p < show_windows_num; p++)
    {
      ShowWindowData *d = &show_windows[p];
      SetWindowPos(d->hwnd, NULL, d->x, d->y, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
    }
}
