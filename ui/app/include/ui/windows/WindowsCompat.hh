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

#ifndef WINDOWS_COMPAT_HH
#define WINDOWS_COMPAT_HH

#include <windows.h>

#include "config/IConfigurator.hh"

#ifndef MONITOR_DEFAULTTONULL
#  define MONITOR_DEFAULTTONULL 0x00000000
#endif

#ifndef MONITOR_DEFAULTTOPRIMARY
#  define MONITOR_DEFAULTTOPRIMARY 0x00000001
#endif

#ifndef MONITOR_DEFAULTTONEAREST
#  define MONITOR_DEFAULTTONEAREST 0x00000002
#endif

class WindowsCompat
{
public:
  // A call to init() should be the first line in each of the other functions in this class
  static void init(workrave::config::IConfigurator::Ptr config);

  static BOOLEAN WinStationQueryInformationW(
    HANDLE hServer,                 // use WTS_CURRENT_SERVER_HANDLE
    ULONG LogonId,                  // use WTS_CURRENT_SESSION
    INT WinStationInformationClass, // http://msdn.microsoft.com/en-us/library/cc248834.aspx
    PVOID pWinStationInformation,
    ULONG WinStationInformationLength,
    PULONG pReturnLength);
  static VOID SwitchToThisWindow(HWND, BOOL);
  static void SetWindowOnTop(HWND, BOOL);
  static void ResetWindow(HWND, bool);
  static void IMEWindowMagic(HWND);
  static bool IsOurWinStationConnected();
  static bool IsOurWinStationLocked();
  static bool IsOurDesktopVisible();

private:
  static volatile LONG _initialized;
  static void _init();

  static bool run_once;
  static bool ime_magic;
  static bool reset_window_always;
  static bool reset_window_never;

  using SWITCHTOTHISWINDOWPROC = void (*)(HWND, BOOL);

  static SWITCHTOTHISWINDOWPROC switch_to_this_window_proc;

  static inline workrave::config::IConfigurator::Ptr config;
};

#endif // WINDOWS_COMPAT_HH
