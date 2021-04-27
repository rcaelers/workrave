// Icon.h --- Icon
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

#ifndef ICON_H
#define ICON_H

#include <windows.h>

class CDeskBand;
class PaintHelper;

class Icon
{
public:
  Icon(HWND hwnd, HINSTANCE hinst, const char *resource, int size, CDeskBand *deskband);
  ~Icon();

  HWND get_handle() const
  {
    return hwnd;
  };
  void get_size(int &w, int &h) const;
  void update();

private:
  CDeskBand *deskband{nullptr};
  HWND hwnd{};
  HICON icon{};
  PaintHelper *paint_helper{nullptr};
  int size{16};
  static void init(HINSTANCE hinst);
  static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
  LRESULT on_paint();
};

#endif // ICON_H
