// W32Compat.hh --- W32 compatibility
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

#ifndef W32_COMPAT_HH
#define W32_COMPAT_HH

#include <windows.h>
#include <winuser.h>

#define MONITOR_DEFAULTTONULL       0x00000000
#define MONITOR_DEFAULTTOPRIMARY    0x00000001
#define MONITOR_DEFAULTTONEAREST    0x00000002

class W32Compat
{
public:
  static BOOL IsWindows95();
  static BOOL EnumDisplayMonitors(HDC hdc,LPCRECT rect,MONITORENUMPROC proc,LPARAM lparam);
  static BOOL GetMonitorInfo(HMONITOR monitor, LPMONITORINFO info);
  static HMONITOR MonitorFromPoint(POINT pt, DWORD dwFlags);

private:
  static void init();
  
  typedef BOOL (WINAPI *ENUMDISPLAYMONITORSPROC)(HDC,LPCRECT,MONITORENUMPROC,LPARAM);
  typedef BOOL (WINAPI *GETMONITORINFOPROC)(HMONITOR monitor, LPMONITORINFO info);
  typedef HMONITOR (WINAPI *MONITORFROMPOINTPROC)(POINT pt, DWORD dwFlags);

  static ENUMDISPLAYMONITORSPROC enum_display_monitors_proc;
  static GETMONITORINFOPROC get_monitor_info_proc;
  static MONITORFROMPOINTPROC monitor_from_point_proc;
  static BOOL initialized;
  static BOOL is_w95;
};


#endif // W32_COMPAT_HH
