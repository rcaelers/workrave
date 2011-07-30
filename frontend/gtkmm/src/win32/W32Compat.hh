// W32Compat.hh --- W32 compatibility
//
// Copyright (C) 2004, 2007, 2010 Rob Caelers & Raymond Penners
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

#ifndef W32_COMPAT_HH
#define W32_COMPAT_HH

#include <windows.h>

#ifndef MONITOR_DEFAULTTONULL
#define MONITOR_DEFAULTTONULL       0x00000000
#endif

#ifndef MONITOR_DEFAULTTOPRIMARY
#define MONITOR_DEFAULTTOPRIMARY    0x00000001
#endif

#ifndef MONITOR_DEFAULTTONEAREST
#define MONITOR_DEFAULTTONEAREST    0x00000002
#endif


class W32Compat
{
public:
  static BOOL EnumDisplayMonitors(HDC hdc,LPCRECT rect,MONITORENUMPROC proc,LPARAM lparam);
  static BOOL GetMonitorInfo(HMONITOR monitor, LPMONITORINFO info);
  static HMONITOR MonitorFromPoint(POINT pt, DWORD dwFlags);
  static VOID SwitchToThisWindow( HWND, BOOL );
  static bool get_force_focus_value();
  static void SetWindowOnTop( HWND, BOOL );
  static bool ForceWindowFocus( HWND );
  static void ResetWindow( HWND, bool );
  static void IMEWindowMagic( HWND );

private:
  static inline void init() { if(run_once) init_once(); }
  static void init_once();

  static bool run_once;
  static bool force_focus;
  static bool ime_magic;
  static bool reset_window_always;
  static bool reset_window_never;

  typedef BOOL (WINAPI *ENUMDISPLAYMONITORSPROC)(HDC,LPCRECT,MONITORENUMPROC,LPARAM);
  typedef BOOL (WINAPI *GETMONITORINFOPROC)(HMONITOR monitor, LPMONITORINFO info);
  typedef HMONITOR (WINAPI *MONITORFROMPOINTPROC)(POINT pt, DWORD dwFlags);
  typedef VOID (WINAPI *SWITCHTOTHISWINDOWPROC)( HWND, BOOL );

  static ENUMDISPLAYMONITORSPROC enum_display_monitors_proc;
  static GETMONITORINFOPROC get_monitor_info_proc;
  static MONITORFROMPOINTPROC monitor_from_point_proc;
  static SWITCHTOTHISWINDOWPROC switch_to_this_window_proc;

};


#endif // W32_COMPAT_HH
