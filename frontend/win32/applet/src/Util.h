// Util.h --- Utils
//
// Copyright (C) 2004 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef UTIL_H
#define UTIL_H

#include <windows.h>

#define TRANSPARENT_DAMAGE_CONTROL_BUF_SIZE 16
class TransparentDamageControl
{
public:
  void BeginPaint(BOOL repaint);
  void HideWindow(HWND hwnd);
  void ShowWindow(HWND hwnd, int x, int y);
  void EndPaint();
private:
  int hide_windows_num;
  HWND hide_windows[TRANSPARENT_DAMAGE_CONTROL_BUF_SIZE];
  struct ShowWindowData 
    {
      HWND hwnd;
      int x, y;
    };
  ShowWindowData show_windows[TRANSPARENT_DAMAGE_CONTROL_BUF_SIZE];
  int show_windows_num;
  BOOL repaint;
};



#endif // UTIL_H

