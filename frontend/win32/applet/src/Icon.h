// Icon.h --- Icon
//
// Copyright (C) 2004, 2005 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef ICON_H
#define ICON_H

#include <windows.h>

class CDeskBand;

class Icon 
{
public:
  Icon(HWND hwnd, HINSTANCE hinst, const char *resource, CDeskBand *deskband);
  ~Icon();

  HWND get_handle() const { return hwnd; };
  void get_size(int &w, int &h) const;

private:
  CDeskBand *deskband;
  HWND hwnd;
  HICON icon;
  static void init(HINSTANCE hinst);
  static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT uMessage, WPARAM wParam,
                                   LPARAM lParam);
  LRESULT on_paint(void);
};

#endif // ICON_H

