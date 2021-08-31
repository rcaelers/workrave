// Copyright (C) 2004, 2007 Rob Caelers & Raymond Penners
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

#ifndef DESKTOP_WINDOW_HH
#define DESKTOP_WINDOW_HH

#include <windows.h>

class DesktopWindow
{
public:
  DesktopWindow(int x, int y, int width, int height);
  ~DesktopWindow();

  void set_visible(bool visible);

private:
  void init();

private:
  HWND hwnd;
  static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  static bool initialized;
  static const char *const WINDOW_CLASS;
};

#endif // DESKTOP_WINDOW_HH
