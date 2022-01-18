// Copyright (C) 2010, 2011, 2013 Rob Caelers <robc@krandor.org>
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

#include "WindowsStatusIcon.hh"

#include <string>
#include <shellapi.h>

#include "commonui/MenuDefs.hh"

#include "debug.hh"

using namespace std;

const UINT MYWM_TRAY_MESSAGE = WM_USER + 0x100;

static HICON pixbuf_to_hicon(GdkPixbuf *pixbuf);

WindowsStatusIcon::WindowsStatusIcon(std::shared_ptr<IApplication> app)
  : toolkit(app->get_toolkit())
  , menu_model(app->get_menu_model())
  , menu_helper(menu_model)
{
  init();
  menu_helper.setup_event();
}

WindowsStatusIcon::~WindowsStatusIcon()
{
  cleanup();
}

void
WindowsStatusIcon::set(const Glib::RefPtr<Gdk::Pixbuf> &pixbuf)
{
  TRACE_ENTRY();
  gint width = pixbuf->get_width();
  gint height = pixbuf->get_height();

  HICON old_hicon = nid.hIcon;
  int size = 32;

  if (width > size || height > size)
    {
      Glib::RefPtr<Gdk::Pixbuf> scaled = pixbuf->scale_simple(MIN(size, width), MIN(size, height), Gdk::INTERP_BILINEAR);
      nid.hIcon = pixbuf_to_hicon(scaled->gobj());
    }
  else
    {
      nid.hIcon = pixbuf_to_hicon(pixbuf->gobj());
    }

  nid.uFlags |= NIF_ICON;
  if (nid.hWnd != nullptr && visible)
    {
      Shell_NotifyIconW(NIM_MODIFY, &nid);
    }

  if (old_hicon != nullptr)
    {
      DestroyIcon(old_hicon);
    }
}

void
WindowsStatusIcon::set_tooltip(const Glib::ustring &text)
{
  gunichar2 *wtext = g_utf8_to_utf16(text.c_str(), -1, nullptr, nullptr, nullptr);

  if (wtext != nullptr)
    {
      nid.uFlags |= NIF_TIP;
      wcsncpy(nid.szTip, (wchar_t *)wtext, G_N_ELEMENTS(nid.szTip) - 1);
      nid.szTip[G_N_ELEMENTS(nid.szTip) - 1] = 0;
      g_free(wtext);
    }
  else
    {
      nid.uFlags &= ~NIF_TIP;
      nid.szTip[0] = 0;
    }

  if (nid.hWnd != nullptr && visible)
    {
      Shell_NotifyIconW(NIM_MODIFY, &nid);
    }
}

void
WindowsStatusIcon::show_balloon(string id, const Glib::ustring &balloon)
{
  TRACE_ENTRY();
  gunichar2 *winfo = g_utf8_to_utf16(balloon.c_str(), -1, nullptr, nullptr, nullptr);
  gunichar2 *wtitle = g_utf8_to_utf16("Workrave", -1, nullptr, nullptr, nullptr);

  current_id = id;

  if (winfo != nullptr && wtitle != nullptr)
    {
      nid.uFlags |= NIF_INFO;
      nid.uTimeout = 20000;
      nid.dwInfoFlags = NIIF_INFO;

      wcsncpy(nid.szInfo, (wchar_t *)winfo, G_N_ELEMENTS(nid.szInfo) - 1);
      nid.szInfo[G_N_ELEMENTS(nid.szInfo) - 1] = 0;

      wcsncpy(nid.szInfoTitle, (wchar_t *)wtitle, G_N_ELEMENTS(nid.szInfoTitle) - 1);
      nid.szInfoTitle[G_N_ELEMENTS(nid.szInfoTitle) - 1] = 0;

      if (nid.hWnd != nullptr && visible)
        {
          Shell_NotifyIconW(NIM_MODIFY, &nid);
        }

      nid.uFlags &= ~NIF_INFO;
    }

  if (winfo != nullptr)
    {
      g_free(winfo);
    }
  if (wtitle != nullptr)
    {
      g_free(wtitle);
    }
}

void
WindowsStatusIcon::set_visible(bool visible)
{
  if (this->visible != visible)
    {
      this->visible = visible;
      if (nid.hWnd != nullptr)
        {
          Shell_NotifyIconW(visible ? NIM_ADD : NIM_DELETE, &nid);
        }
    }
}

bool
WindowsStatusIcon::get_visible() const
{
  return visible;
}

bool
WindowsStatusIcon::is_embedded() const
{
  return true;
}

sigc::signal<void>
WindowsStatusIcon::signal_activate()
{
  return activate_signal;
}

sigc::signal<void, string>
WindowsStatusIcon::signal_balloon_activate()
{
  return balloon_activate_signal;
}

void
WindowsStatusIcon::init()
{
  HINSTANCE hinstance = GetModuleHandle(nullptr);

  WNDCLASSA wclass;
  memset(&wclass, 0, sizeof(WNDCLASS));
  wclass.lpszClassName = "WorkraveTrayObserver";
  wclass.lpfnWndProc = window_proc;
  wclass.hInstance = hinstance;

  ATOM atom = RegisterClassA(&wclass);
  if (atom != 0)
    {
      tray_hwnd = CreateWindow(MAKEINTRESOURCE(atom), nullptr, WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, hinstance, nullptr);
    }

  if (tray_hwnd == nullptr)
    {
      UnregisterClass(MAKEINTRESOURCE(atom), hinstance);
    }
  else
    {
      SetWindowLongPtr(tray_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
      wm_taskbarcreated = RegisterWindowMessageA("TaskbarCreated");
    }

  memset(&nid, 0, sizeof(NOTIFYICONDATA));
  nid.cbSize = NOTIFYICONDATAW_V2_SIZE;
  nid.uID = 1;
  nid.uFlags = NIF_MESSAGE;
  nid.uCallbackMessage = MYWM_TRAY_MESSAGE;
  nid.hWnd = tray_hwnd;

  set_tooltip("Workrave");
}

void
WindowsStatusIcon::cleanup()
{
  if (nid.hWnd != nullptr && visible)
    {
      Shell_NotifyIconW(NIM_DELETE, &nid);
      if (nid.hIcon)
        {
          DestroyIcon(nid.hIcon);
        }
    }
}

void
WindowsStatusIcon::show_menu()
{
  POINT pt = {0};
  GetCursorPos(&pt);

  HMENU menu = CreatePopupMenu();
  init_menu(menu, 0, menu_model->get_root());

  SetForegroundWindow(nid.hWnd);
  UINT command = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, NULL, nid.hWnd, nullptr);
  DestroyMenu(menu);
  auto node = menu_helper.find_node(command);
  if (node)
    {
      node->activate();
    }
}

static std::wstring
ConvertAnsiToWide(const std::string &str)
{
  int count = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), nullptr, 0);
  std::wstring wstr(count, 0);
  MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), &wstr[0], count);
  return wstr;
}

void
WindowsStatusIcon::init_menu(HMENU current_menu, int level, menus::Node::Ptr node)
{
  uint32_t command = menu_helper.allocate_command(node->get_id());

  std::wstring text = ConvertAnsiToWide(node->get_dynamic_text());
  std::replace(text.begin(), text.end(), '_', '&');

  UINT flags = MF_STRING | MF_BYPOSITION;

  if (auto n = std::dynamic_pointer_cast<menus::SubMenuNode>(node); n)
    {
      HMENU popup{nullptr};
      if (level > 0)
        {
          popup = CreatePopupMenu();
          InsertMenuW(current_menu, -1, MF_POPUP | flags, (UINT_PTR)popup, text.c_str());
        }
      else
        {
          popup = current_menu;
        }

      for (auto &menu_to_add: n->get_children())
        {
          init_menu(popup, level + 1, menu_to_add);
        }
    }

  else if (auto n = std::dynamic_pointer_cast<menus::RadioGroupNode>(node); n)
    {
      for (auto &menu_to_add: n->get_children())
        {
          init_menu(current_menu, level + 1, menu_to_add);
        }
    }

  else if (auto n = std::dynamic_pointer_cast<menus::ActionNode>(node); n)
    {
      InsertMenuW(current_menu, -1, flags, (UINT_PTR)(command), text.c_str());
    }

  else if (auto n = std::dynamic_pointer_cast<menus::ToggleNode>(node); n)
    {
      if (n->is_checked())
        {
          flags |= MF_CHECKED;
        }
      InsertMenuW(current_menu, -1, flags, (UINT_PTR)(command), text.c_str());
    }

  else if (auto n = std::dynamic_pointer_cast<menus::RadioNode>(node); n)
    {
      if (n->is_checked())
        {
          flags |= MF_CHECKED;
        }
      InsertMenuW(current_menu, -1, flags, (UINT_PTR)(command), text.c_str());
    }

  else if (auto n = std::dynamic_pointer_cast<menus::SeparatorNode>(node); n)
    {
      InsertMenuW(current_menu, -1, MF_SEPARATOR | flags, (UINT_PTR)(command), text.c_str());
    }
}

LRESULT CALLBACK
WindowsStatusIcon::window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  TRACE_ENTRY_PAR(uMsg, wParam);
  auto *status_icon = (WindowsStatusIcon *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  if (status_icon != nullptr)
    {
      if (uMsg == status_icon->wm_taskbarcreated)
        {
          if (status_icon->visible && status_icon->nid.hWnd != nullptr)
            {
              Shell_NotifyIconW(NIM_ADD, &status_icon->nid);
            }
        }
      else if (uMsg == MYWM_TRAY_MESSAGE)
        {
          switch (lParam)
            {
            case WM_RBUTTONDOWN:
              status_icon->show_menu();
              break;
            case WM_LBUTTONDOWN:
              status_icon->activate_signal.emit();
              break;
            case NIN_BALLOONUSERCLICK:
              status_icon->balloon_activate_signal.emit(status_icon->current_id);
            }
        }
    }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/* This is ganked from GTK+.

 * GDK - The GIMP Drawing Kit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 * Copyright (C) 1998-2002 Tor Lillqvist
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */

static HBITMAP
create_alpha_bitmap(gint size, guchar **outdata)
{
  BITMAPV5HEADER bi;
  HDC hdc;
  HBITMAP hBitmap;

  ZeroMemory(&bi, sizeof(BITMAPV5HEADER));
  bi.bV5Size = sizeof(BITMAPV5HEADER);
  bi.bV5Height = bi.bV5Width = size;
  bi.bV5Planes = 1;
  bi.bV5BitCount = 32;
  bi.bV5Compression = BI_BITFIELDS;
  /* The following mask specification specifies a supported 32 BPP
   * alpha format for Windows XP (BGRA format).
   */
  bi.bV5RedMask = 0x00FF0000;
  bi.bV5GreenMask = 0x0000FF00;
  bi.bV5BlueMask = 0x000000FF;
  bi.bV5AlphaMask = 0xFF000000;

  /* Create the DIB section with an alpha channel. */
  hdc = GetDC(NULL);
  if (!hdc)
    {
      return NULL;
    }
  hBitmap = CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (PVOID *)outdata, NULL, (DWORD)0);
  ReleaseDC(NULL, hdc);

  return hBitmap;
}

static HBITMAP
create_color_bitmap(gint size, guchar **outdata, gint bits)
{
  struct
  {
    BITMAPV4HEADER bmiHeader;
    RGBQUAD bmiColors[2];
  } bmi;
  HDC hdc;
  HBITMAP hBitmap;

  ZeroMemory(&bmi, sizeof(bmi));
  bmi.bmiHeader.bV4Size = sizeof(BITMAPV4HEADER);
  bmi.bmiHeader.bV4Height = bmi.bmiHeader.bV4Width = size;
  bmi.bmiHeader.bV4Planes = 1;
  bmi.bmiHeader.bV4BitCount = bits;
  bmi.bmiHeader.bV4V4Compression = BI_RGB;

  /* when bits is 1, these will be used.
   * bmiColors[0] already zeroed from ZeroMemory()
   */
  bmi.bmiColors[1].rgbBlue = 0xFF;
  bmi.bmiColors[1].rgbGreen = 0xFF;
  bmi.bmiColors[1].rgbRed = 0xFF;

  hdc = GetDC(NULL);
  if (!hdc)
    {
      return NULL;
    }
  hBitmap = CreateDIBSection(hdc, (BITMAPINFO *)&bmi, DIB_RGB_COLORS, (PVOID *)outdata, NULL, (DWORD)0);
  ReleaseDC(NULL, hdc);

  return hBitmap;
}

static gboolean
pixbuf_to_hbitmaps_alpha_winxp(GdkPixbuf *pixbuf, HBITMAP *color, HBITMAP *mask)
{
  /* Based on code from
   * http://www.dotnet247.com/247reference/msgs/13/66301.aspx
   */
  HBITMAP hColorBitmap, hMaskBitmap;
  guchar *indata, *inrow;
  guchar *colordata, *colorrow, *maskdata, *maskbyte;
  gint width, height, size, i, i_offset, j, j_offset, rowstride;
  guint maskstride, mask_bit;

  width = gdk_pixbuf_get_width(pixbuf);   /* width of icon */
  height = gdk_pixbuf_get_height(pixbuf); /* height of icon */

  /* The bitmaps are created square */
  size = MAX(width, height);

  hColorBitmap = create_alpha_bitmap(size, &colordata);
  if (!hColorBitmap)
    return FALSE;
  hMaskBitmap = create_color_bitmap(size, &maskdata, 1);
  if (!hMaskBitmap)
    {
      DeleteObject(hColorBitmap);
      return FALSE;
    }

  /* MSDN says mask rows are aligned to "LONG" boundaries */
  maskstride = (((size + 31) & ~31) >> 3);

  indata = gdk_pixbuf_get_pixels(pixbuf);
  rowstride = gdk_pixbuf_get_rowstride(pixbuf);

  if (width > height)
    {
      i_offset = 0;
      j_offset = (width - height) / 2;
    }
  else
    {
      i_offset = (height - width) / 2;
      j_offset = 0;
    }

  for (j = 0; j < height; j++)
    {
      colorrow = colordata + 4 * (j + j_offset) * size + 4 * i_offset;
      maskbyte = maskdata + (j + j_offset) * maskstride + i_offset / 8;
      mask_bit = (0x80 >> (i_offset % 8));
      inrow = indata + (height - j - 1) * rowstride;
      for (i = 0; i < width; i++)
        {
          colorrow[4 * i + 0] = inrow[4 * i + 2];
          colorrow[4 * i + 1] = inrow[4 * i + 1];
          colorrow[4 * i + 2] = inrow[4 * i + 0];
          colorrow[4 * i + 3] = inrow[4 * i + 3];
          if (inrow[4 * i + 3] == 0)
            maskbyte[0] |= mask_bit; /* turn ON bit */
          else
            maskbyte[0] &= ~mask_bit; /* turn OFF bit */
          mask_bit >>= 1;
          if (mask_bit == 0)
            {
              mask_bit = 0x80;
              maskbyte++;
            }
        }
    }

  *color = hColorBitmap;
  *mask = hMaskBitmap;

  return TRUE;
}

static gboolean
pixbuf_to_hbitmaps_normal(GdkPixbuf *pixbuf, HBITMAP *color, HBITMAP *mask)
{
  /* Based on code from
   * http://www.dotnet247.com/247reference/msgs/13/66301.aspx
   */
  HBITMAP hColorBitmap, hMaskBitmap;
  guchar *indata, *inrow;
  guchar *colordata, *colorrow, *maskdata, *maskbyte;
  gint width, height, size, i, i_offset, j, j_offset, rowstride, nc, bmstride;
  gboolean has_alpha;
  guint maskstride, mask_bit;

  width = gdk_pixbuf_get_width(pixbuf);   /* width of icon */
  height = gdk_pixbuf_get_height(pixbuf); /* height of icon */

  /* The bitmaps are created square */
  size = MAX(width, height);

  hColorBitmap = create_color_bitmap(size, &colordata, 24);
  if (!hColorBitmap)
    return FALSE;
  hMaskBitmap = create_color_bitmap(size, &maskdata, 1);
  if (!hMaskBitmap)
    {
      DeleteObject(hColorBitmap);
      return FALSE;
    }

  /* rows are always aligned on 4-byte boundaries */
  bmstride = size * 3;
  if (bmstride % 4 != 0)
    bmstride += 4 - (bmstride % 4);

  /* MSDN says mask rows are aligned to "LONG" boundaries */
  maskstride = (((size + 31) & ~31) >> 3);

  indata = gdk_pixbuf_get_pixels(pixbuf);
  rowstride = gdk_pixbuf_get_rowstride(pixbuf);
  nc = gdk_pixbuf_get_n_channels(pixbuf);
  has_alpha = gdk_pixbuf_get_has_alpha(pixbuf);

  if (width > height)
    {
      i_offset = 0;
      j_offset = (width - height) / 2;
    }
  else
    {
      i_offset = (height - width) / 2;
      j_offset = 0;
    }

  for (j = 0; j < height; j++)
    {
      colorrow = colordata + (j + j_offset) * bmstride + 3 * i_offset;
      maskbyte = maskdata + (j + j_offset) * maskstride + i_offset / 8;
      mask_bit = (0x80 >> (i_offset % 8));
      inrow = indata + (height - j - 1) * rowstride;
      for (i = 0; i < width; i++)
        {
          if (has_alpha && inrow[nc * i + 3] < 128)
            {
              colorrow[3 * i + 0] = colorrow[3 * i + 1] = colorrow[3 * i + 2] = 0;
              maskbyte[0] |= mask_bit; /* turn ON bit */
            }
          else
            {
              colorrow[3 * i + 0] = inrow[nc * i + 2];
              colorrow[3 * i + 1] = inrow[nc * i + 1];
              colorrow[3 * i + 2] = inrow[nc * i + 0];
              maskbyte[0] &= ~mask_bit; /* turn OFF bit */
            }
          mask_bit >>= 1;
          if (mask_bit == 0)
            {
              mask_bit = 0x80;
              maskbyte++;
            }
        }
    }

  *color = hColorBitmap;
  *mask = hMaskBitmap;

  return TRUE;
}

static HICON
pixbuf_to_hicon(GdkPixbuf *pixbuf)
{
  gint x = 0, y = 0;
  gboolean is_icon = TRUE;
  ICONINFO ii;
  HICON icon;
  gboolean success;

  if (pixbuf == NULL)
    return NULL;

  if (gdk_pixbuf_get_has_alpha(pixbuf))
    success = pixbuf_to_hbitmaps_alpha_winxp(pixbuf, &ii.hbmColor, &ii.hbmMask);
  else
    success = pixbuf_to_hbitmaps_normal(pixbuf, &ii.hbmColor, &ii.hbmMask);

  if (!success)
    return NULL;

  ii.fIcon = is_icon;
  ii.xHotspot = x;
  ii.yHotspot = y;
  icon = CreateIconIndirect(&ii);
  DeleteObject(ii.hbmColor);
  DeleteObject(ii.hbmMask);
  return icon;
}
