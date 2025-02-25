// Copyright (C) 2004, 2007, 2010, 2012 Rob Caelers, Raymond Penners, Ray Satiro
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

#include "debug.hh"

#include "ui/windows/WindowsCompat.hh"

#include <windows.h>
#include <tchar.h>
#include <wtsapi32.h>

#if defined(PLATFORM_OS_WINDOWS_NATIVE)
#  undef max
#endif

#include "ui/windows/WindowsForceFocus.hh"
#include "utils/W32CriticalSection.hh"

bool WindowsCompat::ime_magic = false;
bool WindowsCompat::reset_window_always = false;
bool WindowsCompat::reset_window_never = false;

WindowsCompat::SWITCHTOTHISWINDOWPROC WindowsCompat::switch_to_this_window_proc = NULL;

typedef BOOLEAN(WINAPI *PWINSTATIONQUERYINFORMATIONW)(HANDLE, ULONG, INT, PVOID, ULONG, PULONG);
namespace
{
  PWINSTATIONQUERYINFORMATIONW dyn_WinStationQueryInformationW;
}

void
WindowsCompat::init(workrave::config::IConfigurator::Ptr config)
{
  WindowsCompat::config = config;
  if (!_initialized)
    {
      _init();
    }
}

/* WindowsCompat::WinStationQueryInformationW()

Query information about our winstation.
http://msdn.microsoft.com/en-us/library/windows/desktop/aa383827.aspx

This API is deprecated so its use is limited. Some queries are problematic on x64:
http://www.remkoweijnen.nl/blog/2011/01/29/querying-a-user-token-under-64-bit-version-of-2003xp/

Help for WinStationInformationClass info:
http://msdn.microsoft.com/en-us/library/cc248604.aspx
http://msdn.microsoft.com/en-us/library/cc248834.aspx

returns whatever the API returns.
if the API is unavailable this function returns FALSE and last error is set ERROR_INVALID_FUNCTION.
*/
BOOLEAN
WindowsCompat::WinStationQueryInformationW(HANDLE hServer,                 // use WTS_CURRENT_SERVER_HANDLE
                                           ULONG LogonId,                  // use WTS_CURRENT_SESSION
                                           INT WinStationInformationClass, // review msdn links in comment block
                                           PVOID pWinStationInformation,
                                           ULONG WinStationInformationLength,
                                           PULONG pReturnLength)
{
  if (!dyn_WinStationQueryInformationW)
    {
      SetLastError(ERROR_INVALID_FUNCTION);
      return FALSE;
    }

  return dyn_WinStationQueryInformationW(hServer,
                                         LogonId,
                                         WinStationInformationClass,
                                         pWinStationInformation,
                                         WinStationInformationLength,
                                         pReturnLength);
}

VOID
WindowsCompat::SwitchToThisWindow(HWND hwnd, BOOL emulate_alt_tab)
{
  if (switch_to_this_window_proc != NULL)
    {
      (*switch_to_this_window_proc)(hwnd, emulate_alt_tab);
    }
  return;
}

static W32CriticalSection cs__init;
volatile LONG WindowsCompat::_initialized = 0;

/* WindowsCompat::_init()

Initialize the WindowsCompat class. The functions should call init(), which is inline, instead of this.
A call to init() should be the first line in each of the other functions in this class.
*/
void
WindowsCompat::_init()
{
  W32CriticalSection::Guard guard(cs__init);

  if (_initialized)
    return;

  HMODULE user_lib = GetModuleHandleA("user32.dll");
  if (user_lib)
    {
      switch_to_this_window_proc = (SWITCHTOTHISWINDOWPROC)GetProcAddress(user_lib, "SwitchToThisWindow");
    }

  HMODULE winsta_lib = LoadLibraryA("winsta");
  if (winsta_lib)
    {
      dyn_WinStationQueryInformationW = (PWINSTATIONQUERYINFORMATIONW)GetProcAddress(winsta_lib, "WinStationQueryInformationW");
    }

  // Should SetWindowOnTop() call IMEWindowMagic() ?
  if (!config->get_value("advanced/ime_magic", ime_magic))
    {
      ime_magic = false;
    }

  // As of writing SetWindowOnTop() always calls ResetWindow()
  // ResetWindow() determines whether to "reset" when both
  // reset_window_always and reset_window_never are false.
  //
  // If reset_window_always is true, and if ResetWindow() is continually
  // passed the same hwnd, hwnd will flicker as a result of the continual
  // z-order position changes / resetting.
  if (!config->get_value("advanced/reset_window_always", reset_window_always))
    {
      reset_window_always = false;
    }
  // ResetWindow() will always abort when reset_window_never is true.
  if (!config->get_value("advanced/reset_window_never", reset_window_never))
    {
      reset_window_never = false;
    }

  InterlockedExchange(&_initialized, 1);
}

void
WindowsCompat::SetWindowOnTop(HWND hwnd, BOOL topmost)
{
  SetWindowPos(hwnd, topmost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

  ResetWindow(hwnd, (bool)topmost);

  if (ime_magic && topmost)
    {
      IMEWindowMagic(hwnd);
    }

  if (WindowsForceFocus::GetForceFocusValue() && topmost)
    {
      WindowsForceFocus::ForceWindowFocus(hwnd);
    }

  return;
}

// Bug 587 -  Vista: Workrave not modal / coming to front
// http://issues.workrave.org/show_bug.cgi?id=587
// There is an issue with IME and window z-ordering.
//
// hwnd == window to "reset" in z-order
// topmost == true if window should be topmost, false otherwise
void
WindowsCompat::ResetWindow(HWND hwnd, bool topmost)
{
  if (!IsWindow(hwnd) || reset_window_never)
    return;

  bool reset = false;
  DWORD gwl_exstyle = 0;

  WINDOWINFO gwi;
  ZeroMemory(&gwi, sizeof(gwi));
  gwi.cbSize = sizeof(WINDOWINFO);

  SetLastError(0);
  gwl_exstyle = (DWORD)GetWindowLong(hwnd, GWL_EXSTYLE);
  if (!GetLastError())
    {
      // if desired and actual topmost style differ, plan to reset
      if (topmost != (gwl_exstyle & WS_EX_TOPMOST ? true : false))
        reset = true;
    }

  SetLastError(0);
  GetWindowInfo(hwnd, &gwi);
  if (!GetLastError())
    {
      // if desired and actual topmost style differ, plan to reset
      if (topmost != (gwi.dwExStyle & WS_EX_TOPMOST ? true : false))
        reset = true;
    }

#ifdef BREAKAGE
  const bool DEBUG = false;
  DWORD valid_exstyle_diff = 0;

  // GetWindowInfo() and GetWindowLong() extended style info can differ.
  // Compare the two results but filter valid values only.
  valid_exstyle_diff = (gwl_exstyle ^ gwi.dwExStyle) & ~0xF1A08802;
  if (valid_exstyle_diff || DEBUG)
    {
      // if the extended style info differs, plan to reset.
      // e.g. gwl returned ws_ex_toolwindow but gwi didn't
      reset = true;

      // attempt to sync differences:
      DWORD swl_exstyle = (valid_exstyle_diff | gwl_exstyle) & ~0xF1A08802;

      if ((swl_exstyle & WS_EX_APPWINDOW) && (swl_exstyle & WS_EX_TOOLWINDOW))
        // this hasn't happened and shouldn't happen, but i suppose it could.
        // if both styles are set change to appwindow only.
        // why not toolwindow only? well, why are they both set in the first place?
        // in this case it's better to make hwnd visible on the taskbar.
        {
          swl_exstyle &= ~WS_EX_TOOLWINDOW;
        }

      ShowWindow(hwnd, SW_HIDE);
      SetWindowLong(hwnd, GWL_EXSTYLE, (LONG)swl_exstyle);
      ShowWindow(hwnd, SW_SHOWNA);
    }
#endif

  // "reset" window position in z-order.
  // if the window is supposed to be topmost but is really not:
  // set HWND_NOTOPMOST followed by HWND_TOPMOST
  // the above sequence is key: review test results in 587#c17
  //
  // if the window is not supposed to be topmost but is, reverse:
  // set HWND_TOPMOST followed by HWND_NOTOPMOST
  // the reverse is currently unproven.
  // i don't know of any problems removing the topmost style.
  if (IsWindow(hwnd) && (reset || reset_window_always))
    {
      SetWindowPos(hwnd, !topmost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
      SetWindowPos(hwnd, topmost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }

  return;
}

// Bug 587 -  Vista: Workrave not modal / coming to front
// http://issues.workrave.org/show_bug.cgi?id=587
// There is an issue with IME and window z-ordering.
//
// ResetWindow() tests sufficient. This code is for troubleshooting.
// if all else fails request user enable advanced/ime_magic = "1"
void
WindowsCompat::IMEWindowMagic(HWND hwnd)
{
  if (!IsWindow(hwnd))
    return;

  // This message works to make hwnd topmost without activation or focus.
  // I found it by watching window messages. I don't know its intended use.
  SendMessage(hwnd, 0x287, 0x17 /*0x18*/, (LPARAM)hwnd);

  SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

/* WindowsCompat::IsOurWinStationConnected()

Check if our winstation is connected by querying terminal services.

Note: Terminal services API can cause a harmless exception that it handles and should be ignored:
First-chance exception at 0x7656fc56 in workrave.exe: 0x000006BA: The RPC server is unavailable.

returns true if our winstation is connected
*/
bool
WindowsCompat::IsOurWinStationConnected()
{
  bool func_retval = false;
  DWORD bytes_returned = 0;

  WTS_CONNECTSTATE_CLASS *state = NULL;

  if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                                 WTS_CURRENT_SESSION,
                                 WTSConnectState,
                                 reinterpret_cast<LPTSTR *>(&state),
                                 &bytes_returned)
      && state)
    {
      if ((bytes_returned == sizeof(*state)) && ((*state == WTSActive) || (*state == WTSConnected)))
        {
          func_retval = true;
        }

      WTSFreeMemory(state);
    }

  return func_retval;
}

/* WindowsCompat::IsOurWinStationLocked()

Check if our winstation is locked by querying terminal services.

Note: Terminal services API can cause a harmless exception that it handles and should be ignored:
First-chance exception at 0x7656fc56 in workrave.exe: 0x000006BA: The RPC server is unavailable.

returns true if our winstation is locked
*/
bool
WindowsCompat::IsOurWinStationLocked()
{
  BOOL locked = FALSE;
  DWORD bytes_returned = 0;

  if (!WinStationQueryInformationW(WTS_CURRENT_SERVER_HANDLE, // SERVERNAME_CURRENT
                                   WTS_CURRENT_SESSION,       // LOGONID_CURRENT
                                   28,                        // WinStationLockedState
                                   &locked,
                                   sizeof(locked),
                                   &bytes_returned)
      || (bytes_returned != sizeof(locked)))
    {
      return false;
    }

  return !!locked;
}

/* WindowsCompat::IsOurDesktopVisible()

Check if our desktop can be viewed by the user: Check that our process' window station is connected
and its input desktop is the same as our calling thread's desktop.

If our desktop is visible that implies that this process' workstation is unlocked.

returns true if our desktop is visible
*/
bool
WindowsCompat::IsOurDesktopVisible()
{
  bool func_retval = false;

  HDESK input_desktop_handle = NULL;
  HDESK our_desktop_handle = NULL;

  wchar_t input_desktop_name[MAX_PATH] = {
    L'\0',
  };
  wchar_t our_desktop_name[MAX_PATH] = {
    L'\0',
  };

  BOOL ret = 0;
  DWORD bytes_needed = 0;

  /*
  Get the input desktop name
  */
  input_desktop_handle = OpenInputDesktop(0, false, GENERIC_READ);
  if (!input_desktop_handle)
    goto cleanup;

  bytes_needed = 0;
  ret = GetUserObjectInformationW(input_desktop_handle, UOI_NAME, input_desktop_name, sizeof(input_desktop_name), &bytes_needed);

  if (!ret || (bytes_needed > sizeof(input_desktop_name)))
    goto cleanup;

  /*
  Get our calling thread's desktop name
  */
  our_desktop_handle = GetThreadDesktop(GetCurrentThreadId());
  if (!our_desktop_handle)
    goto cleanup;

  bytes_needed = 0;
  ret = GetUserObjectInformationW(our_desktop_handle, UOI_NAME, our_desktop_name, sizeof(our_desktop_name), &bytes_needed);
  if (!ret || (bytes_needed > sizeof(our_desktop_name)))
    goto cleanup;

  // If the desktop names are different then our thread is not associated with the input desktop
  if (_wcsnicmp(input_desktop_name, our_desktop_name, MAX_PATH))
    goto cleanup;

  // If our winstation is not connected then our desktop is not visible
  if (!IsOurWinStationConnected())
    goto cleanup;

  func_retval = true;
cleanup:
  if (input_desktop_handle)
    CloseHandle(input_desktop_handle);

  if (our_desktop_handle)
    CloseHandle(our_desktop_handle);

  return func_retval;
}
