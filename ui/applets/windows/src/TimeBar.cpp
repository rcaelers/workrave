// TimeBar.cpp --- Time bar
//
// Copyright (C) 2004, 2005, 2006, 2007 Raymond Penners <raymond@dotsphinx.com>
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
#include <map>

#include "TimeBar.h"
#include "DeskBand.h"
#include "Debug.h"
#include "PaintHelper.h"

const int BORDER_SIZE = 2;
const int MARGINX = 4;
const int MARGINY = 0;
const int MINIMAL_HEIGHT = 16;

#define TIME_BAR_CLASS_NAME "WorkraveTimeBar"
#define GDK_TO_COLORREF(r, g, b) (((r) >> 8) | (((g) >> 8) << 8) | (((b) >> 8) << 16))

TimeBar::TimeBar(HWND parent, HINSTANCE hinst, CDeskBand *deskband)
  : deskband(deskband)
  , dpi(::GetDpiForWindow(parent))
{
  init_colors();
  init_font();
  init_window(parent, hinst);
  update_size();
}

TimeBar::~TimeBar()
{
  DestroyWindow(hwnd);
  if (bar_font)
    {
      DeleteObject(bar_font);
    }
  for (auto [k, v]: bar_colors)
    {
      DeleteObject(v);
    }
}

LRESULT CALLBACK
TimeBar::wnd_proc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  auto *pThis = (TimeBar *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

  switch (uMessage)
    {
    case WM_NCCREATE:
      {
        auto *lpcs = (LPCREATESTRUCT)lParam;
        pThis = (TimeBar *)(lpcs->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
        SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
      }
      break;

    case WM_PAINT:
      return pThis->on_paint();

    case WM_LBUTTONUP:
      SendMessage(pThis->deskband->get_command_window(), WM_USER + 1, 0, 0);
      break;
    }
  return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

void
TimeBar::get_size(int &w, int &h) const
{
  w = width;
  h = height;
}

LRESULT
TimeBar::on_paint()
{
  TRACE_ENTER("TimeBar::on_paint");
  RECT rc;

  HDC dc = paint_helper->BeginPaint();

  GetClientRect(hwnd, &rc);

  RECT r;

  int winx = rc.left;
  int winy = rc.top;
  int winw = rc.right - rc.left;
  int winh = rc.bottom - rc.top;

  SelectObject(dc, (HGDIOBJ)bar_font);
  r.left = r.top = r.bottom = r.right = 0;

  TRACE_MSG("1" << winx << " " << winy << " " << winw << " " << winh);
  // Bar
  int bar_width = 0;
  int border_size = BORDER_SIZE;
  if (bar_max_value > 0)
    {
      bar_width = (bar_value * (winw - 2 * border_size)) / bar_max_value;
    }

  // Secondary bar
  int sbar_width = 0;
  if (secondary_bar_max_value > 0)
    {
      sbar_width = (secondary_bar_value * (winw - 2 * border_size)) / secondary_bar_max_value;
    }

  int bar_h = winh - 2 * border_size;

  TRACE_MSG("2");
  if (sbar_width > 0)
    {
      // Overlap
      // assert(secondary_bar_color == COLOR_ID_INACTIVE);
      TimerColorId overlap_color;
      switch (bar_color)
        {
        case TimerColorId::Active:
          overlap_color = TimerColorId::InactiveOverActive;
          break;
        case TimerColorId::Overdue:
          overlap_color = TimerColorId::InactiveOverOverdue;
          break;
        default:
          overlap_color = TimerColorId::Bg;
        }

      if (sbar_width >= bar_width)
        {
          if (bar_width != 0)
            {
              r.left = winx + border_size;
              r.top = winy + border_size;
              r.right = r.left + bar_width;
              r.bottom = r.top + bar_h;
              FillRect(dc, &r, bar_colors[overlap_color]);
            }
          if (sbar_width > bar_width)
            {
              r.left = winx + bar_width + border_size;
              r.top = winy + border_size;
              r.right = r.left + sbar_width - bar_width;
              r.bottom = r.top + bar_h;
              FillRect(dc, &r, bar_colors[secondary_bar_color]);
            }
        }
      else
        {
          if (sbar_width != 0)
            {
              r.left = winx + border_size;
              r.top = winy + border_size;
              r.right = r.left + sbar_width;
              r.bottom = r.top + bar_h;
              FillRect(dc, &r, bar_colors[overlap_color]);
            }
          r.left = winx + border_size + sbar_width;
          r.top = winy + border_size;
          r.right = r.left + bar_width - sbar_width;
          r.bottom = r.top + bar_h;
          FillRect(dc, &r, bar_colors[bar_color]);
        }
    }
  else
    {
      // No overlap
      r.left = winx + border_size;
      r.top = winy + border_size;
      r.right = r.left + bar_width;
      r.bottom = r.top + bar_h;
      FillRect(dc, &r, bar_colors[bar_color]);
    }

  TRACE_MSG("3");
  r.left = winx + border_size + __max(bar_width, sbar_width);
  r.top = winy + border_size;
  r.right = winx + winw - border_size;
  r.bottom = r.top + bar_h;
  FillRect(dc, &r, bar_colors[TimerColorId::Bg]);

  r.left = winx;
  r.top = winy;
  r.bottom = r.top + winh;
  r.right = r.left + winw;
  DrawEdge(dc, &r, BF_ADJUST | EDGE_SUNKEN, BF_RECT);

  SetBkMode(dc, TRANSPARENT);
  r.right -= border_size + MARGINX;
  r.left += border_size + MARGINX;
  DrawText(dc, bar_text, (int)strlen(bar_text), &r, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);

  TRACE_MSG("4");

  paint_helper->EndPaint();

  TRACE_EXIT();
  return 0;
}

void
TimeBar::init_window(HWND parent, HINSTANCE hinst)
{
  TRACE_ENTER("TimeBar::init_window");

  // If the window class has not been registered, then do so.
  WNDCLASS wc;
  if (!GetClassInfo(hinst, TIME_BAR_CLASS_NAME, &wc))
    {
      ZeroMemory(&wc, sizeof(wc));
      wc.style = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
      wc.lpfnWndProc = (WNDPROC)wnd_proc;
      wc.cbClsExtra = 0;
      wc.cbWndExtra = 0;
      wc.hInstance = hinst;
      wc.hIcon = nullptr;
      wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
      wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(192, 0, 0));
      wc.lpszMenuName = nullptr;
      wc.lpszClassName = TIME_BAR_CLASS_NAME;

      RegisterClass(&wc);
    }

  hwnd = CreateWindowEx(0,
                        TIME_BAR_CLASS_NAME,
                        "",
                        WS_CHILD | WS_CLIPSIBLINGS,
                        0,
                        0,
                        56,
                        16,
                        parent,
                        nullptr,
                        hinst,
                        (LPVOID)this);
  paint_helper = std::make_shared<PaintHelper>(hwnd);

  TRACE_EXIT();
}

void
TimeBar::init_colors()
{
  HBRUSH green_mix_blue = CreateSolidBrush(0x00b2d400);
  HBRUSH light_green = CreateSolidBrush(GDK_TO_COLORREF(37008, 61166, 37008));
  HBRUSH orange = CreateSolidBrush(GDK_TO_COLORREF(65535, 42405, 0));
  HBRUSH light_blue = CreateSolidBrush(GDK_TO_COLORREF(44461, 55512, 59110));
  HBRUSH bg = CreateSolidBrush(GetSysColor(COLOR_3DLIGHT));

  bar_colors[TimerColorId::Active] = light_blue;
  bar_colors[TimerColorId::Inactive] = light_green;
  bar_colors[TimerColorId::Overdue] = orange;
  bar_colors[TimerColorId::InactiveOverActive] = green_mix_blue;
  bar_colors[TimerColorId::InactiveOverOverdue] = light_green;
  bar_colors[TimerColorId::Bg] = bg;
}

void
TimeBar::init_font()
{
  TRACE_ENTER("TimeBar::init_font");
  NONCLIENTMETRICSW ncm;
  ncm.cbSize = sizeof(ncm);

  TRACE_MSG("dpi " << dpi);
  if (::SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0, dpi))
    {
      TRACE_MSG("1");
      if (bar_font)
        {
          DeleteObject(bar_font);
        }

      bar_font = ::CreateFontIndirectW(&ncm.lfStatusFont);
    }

  TRACE_EXIT();
}

//! Converts the specified time to a string
void
TimeBar::time_to_string(time_t time, char *buf, int len)
{
  char t[2];

  if (time < 0)
    {
      t[0] = '-';
      t[1] = 0;
      time = -time;
    }
  else
    {
      t[0] = 0;
    }
  int hrs = (int)(time / 3600);
  int min = (int)((time / 60) % 60);
  int sec = (int)(time % 60);

  if (hrs > 0)
    {
      _snprintf_s(buf, len, _TRUNCATE, "%s%d:%02d:%02d", t, hrs, min, sec);
    }
  else
    {
      _snprintf_s(buf, len, _TRUNCATE, "%s%d:%02d", t, min, sec);
    }
}

void
TimeBar::set_progress(int value, int max_value)
{
  bar_value = value;
  bar_max_value = max_value;
}

void
TimeBar::set_secondary_progress(int value, int max_value)
{
  secondary_bar_value = value;
  secondary_bar_max_value = max_value;
}

void
TimeBar::set_text(const char *text)
{
  TRACE_ENTER("TimeBar::set_text");
  strncpy_s(bar_text, APPLET_BAR_TEXT_MAX_LENGTH, text, _TRUNCATE);
  TRACE_EXIT();
}

void
TimeBar::update()
{
  TRACE_ENTER("TimeBar::update");
  InvalidateRect(hwnd, nullptr, FALSE);
  TRACE_EXIT();
}

void
TimeBar::set_bar_color(TimerColorId color)
{
  bar_color = color;
}

void
TimeBar::set_secondary_bar_color(TimerColorId color)
{
  secondary_bar_color = color;
}

void
TimeBar::update_dpi()
{
  dpi = GetDpiForWindow(hwnd);
  init_font();
  update_size();
}

void
TimeBar::update_size()
{
  TRACE_ENTER("TimeBar::update_size");
  HDC dc = GetDC(hwnd);
  SelectObject(dc, (HGDIOBJ)bar_font);
  char buf[80];

  time_to_string(-(59 + 59 * 60 + 9 * 60 * 60), buf, sizeof(buf));

  RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.bottom = 0;
  rect.right = 0;
  height = DrawText(dc, buf, (int)strlen(buf), &rect, DT_CALCRECT);
  if (height == 0)
    {
      width = 32;
      height = 16;
    }
  else
    {
      width = rect.right;
    }

  width += 2 * MARGINX + 2 * BORDER_SIZE;
  height += 2 * MARGINY + 2 * BORDER_SIZE;
  if (height < MINIMAL_HEIGHT)
    {
      height = MINIMAL_HEIGHT;
    }
  ReleaseDC(hwnd, dc);

  SetWindowPos(hwnd, nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
  TRACE_MSG(width << " " << height);
}
