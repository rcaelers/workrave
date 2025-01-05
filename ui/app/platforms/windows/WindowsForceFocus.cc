// WindowsForceFocus.cc --- Collection of hacks to force win32 window focus
//
// Copyright (C) 2012 Ray Satiro <raysatiro@yahoo.com>
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

#include "ui/windows/WindowsForceFocus.hh"

#include <windows.h>
#include <tchar.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include <cstdlib>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>

#include "config/IConfigurator.hh"
#include "ui/windows/WindowsCompat.hh"
#include "utils/W32CriticalSection.hh"

using namespace std;
using namespace workrave;

#if defined(_MSC_VER)
#  pragma warning(disable : 4102) // warning C4102: unreferenced label
#endif

HANDLE WindowsForceFocus::thread_handle;
struct WindowsForceFocus::thread_info WindowsForceFocus::ti;
bool WindowsForceFocus::force_focus;

// Critical section objects for each function that needs one
static W32CriticalSection cs_GetForceFocusValue;
static W32CriticalSection cs_GetFunctions;

/* Each of these flags will represent a function that can be used to force the window focus.
The flags of functions that we want to use to try to force the window focus will be OR'd together
and then passed to a worker thread which will try each of the functions specified.
*/
enum functions
{
  NO_FUNCTIONS = 0x00,
  ALT_KEYPRESS = (1 << 0),
  ATTACH_INPUT = (1 << 1),
  MINIMIZE_RESTORE = (1 << 2),
  DISABLE_FOREGROUND_TIMEOUT = (1 << 3), // not implemented
  RESET_FOREGROUND_TIMEOUT = (1 << 4),   // not implemented
  ALL_FUNCTIONS = 0xFF
};

void
WindowsForceFocus::init(workrave::config::IConfigurator::Ptr config)
{
  WindowsForceFocus::config = config;
}

/* WindowsForceFocus::GetFunctions()

Check for an existing configuration preference at advanced/force_focus_functions specifying which
functions we should use to try to force window focus.

-
The names match to the focus functions' enumerated names. For example:

[HKEY_CURRENT_USER\Software\Workrave\advanced]
"force_focus_functions"="minimize_restore,attach_input"

If the above was found then this function returns ( MINIMIZE_RESTORE | ATTACH_INPUT ).
-

returns the functions specified in the config or if no config found returns ALL_FUNCTIONS
*/
DWORD
WindowsForceFocus::GetFunctions()
{
  W32CriticalSection::Guard guard(cs_GetFunctions);

  static bool func_initialized = false;
  static DWORD flags = 0;

  if (func_initialized)
    return flags;

  string str;

  if (config->get_value("advanced/force_focus_functions", str))
    {
      transform(str.begin(), str.end(), str.begin(), ::toupper);

      vector<string> names;
      boost::split(names, str, boost::is_any_of(","));
      for (vector<string>::iterator name = names.begin(); name != names.end(); ++name)
        {
          if (!name->compare("ALL_FUNCTIONS"))
            {
              flags = ALL_FUNCTIONS;
              break;
            }
          else if (!name->compare("NO_FUNCTIONS"))
            {
              flags = NO_FUNCTIONS;
              break;
            }
          else if (!name->compare("ALT_KEYPRESS"))
            flags |= ALT_KEYPRESS;
          else if (!name->compare("ATTACH_INPUT"))
            flags |= ATTACH_INPUT;
          else if (!name->compare("MINIMIZE_RESTORE"))
            flags |= MINIMIZE_RESTORE;
        }
    }
  else
    {
      flags = (ATTACH_INPUT | MINIMIZE_RESTORE);
    }

  func_initialized = true;
  return flags;
}

/* WindowsForceFocus::GetForceFocusValue()

Cache the user's advanced/force_focus preference in WindowsForceFocus::force_focus or set false if none.

returns force_focus
*/
bool
WindowsForceFocus::GetForceFocusValue()
{
  W32CriticalSection::Guard guard(cs_GetForceFocusValue);

  static bool func_initialized = false;

  if (func_initialized)
    return force_focus;

  config->get_value_with_default("advanced/force_focus", force_focus, false);

  func_initialized = true;
  return force_focus;
}

/* WindowsForceFocus::GetForegroundWindowTryHarder()

Call GetForegroundWindow() repeatedly to get the foreground window. Try up to 'max_retries'.

While the window manager is switching the foreground window it sets it as NULL temporarily. In some
cases the functions in this class cannot succeed without the current foreground window so that
value is unacceptable. We will use this function to wait a bit for any pending transition to
complete.

Empirical testing in my virtual machines shows an average transition takes two dozen retries.
However my development Vista x86 machine shows an average transition takes 150 retries. Not sure
why there's such a big discrepancy there, but maybe it's because I have so many windows open on
this computer. For now I'm setting the default 'max_retries' to 200 to cover even heavy use cases.

returns the foreground window or NULL if the window could not be found after 'max_retries'
*/
HWND
WindowsForceFocus::GetForegroundWindowTryHarder(DWORD max_retries // default: 200
)
{
  for (DWORD i = 0; i < max_retries; ++i)
    {
      /* GetForegroundWindow() returns NULL when:
      1. There is no foreground window.
      2. The foreground window is in transition.
      3. Our calling thread's desktop is not the input desktop.
      4. Our process' window station is locked (see #3) or not connected.
      */
      HWND hwnd = GetForegroundWindow();
      if (hwnd)
        return hwnd;

      Sleep(1);
    }

  return NULL;
}

/* WindowsForceFocus::AltKeypress()

Simulate an ALT keypress while setting 'hwnd' as the foreground window.

The oldest known method (circa 1999) to bypass Microsoft's SetForegroundWindow() restrictions.

From 'Programming Applications for Microsoft Windows' (Richter):
"The system automatically unlocks the SetForegroundWindow function when the user presses the Alt
key or if the user explicitly brings a window to the foreground. This prevents an application from
keeping SetForegroundWindow locked all the time."

returns true on success: ( GetForegroundWindow() == hwnd )
*/
bool
WindowsForceFocus::AltKeypress(HWND hwnd)
{
  bool simulated = false;

  if (!IsWindowVisible(hwnd))
    return false;

  if (IsIconic(hwnd))
    ShowWindow(hwnd, SW_RESTORE);

  HWND foreground_hwnd = GetForegroundWindowTryHarder();
  if (foreground_hwnd == hwnd)
    return true;

  // If the user isn't already pressing down an ALT key then simulate one
  if (!(GetAsyncKeyState(VK_MENU) & 0x8000))
    {
      simulated = true;
      keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY, 0);
    }

  BringWindowToTop(hwnd);
  SetForegroundWindow(hwnd);

  if (simulated)
    keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

  foreground_hwnd = GetForegroundWindowTryHarder();
  return (foreground_hwnd == hwnd);
}

/* WindowsForceFocus::AttachInput()

Attach to the current foreground window's thread queue and set 'hwnd' as the foreground window.

This is another old and proven method to set the foreground window. Attach to the input queue of
whatever thread is currently receiving the raw input (ie the thread holding the current foreground
window) and if successful set our window as the foreground window, then detach.

Attaching can fail due to lack of privilege or if the target thread is not a GUI thread.

This method is frought with problems because we're essentially sharing processing with another
thread and we can't predict that thread's behavior. Like most of the focus functions in this class
it should be called from a sacrificial worker thread so that if something goes wrong the rest of
our program stays sane.

returns true on success: ( GetForegroundWindow() == hwnd )
*/
bool
WindowsForceFocus::AttachInput(HWND hwnd)
{
  bool attached = false;

  if (!IsWindowVisible(hwnd))
    return false;

  if (IsIconic(hwnd))
    ShowWindow(hwnd, SW_RESTORE);

  HWND foreground_hwnd = GetForegroundWindowTryHarder();
  if (foreground_hwnd == hwnd)
    return true;

  DWORD foreground_pid = 0, foreground_tid = 0;
  if (foreground_hwnd)
    {
      foreground_tid = GetWindowThreadProcessId(foreground_hwnd, &foreground_pid);
      if (foreground_tid && foreground_pid && (foreground_pid != GetCurrentProcessId()))
        {
          attached = !!AttachThreadInput(GetCurrentThreadId(), foreground_tid, TRUE);
        }
    }

  BringWindowToTop(hwnd);
  SetForegroundWindow(hwnd);

  if (attached)
    AttachThreadInput(GetCurrentThreadId(), foreground_tid, FALSE);

  foreground_hwnd = GetForegroundWindowTryHarder();
  return (foreground_hwnd == hwnd);
}

/* WindowsForceFocus::MinimizeRestore()

Minimize and restore a dummy message window to set 'hwnd' as the foreground window.

The minimization and restoration of a dummy window may allow it to take the foreground. If that
happens foreground permissions have been acquired so set 'hwnd' as the foreground window.

returns true on success: ( GetForegroundWindow() == hwnd )
*/
bool
WindowsForceFocus::MinimizeRestore(HWND hwnd)
{
  WINDOWPLACEMENT wp = {
    sizeof(wp),
    WPF_SETMINPOSITION,
    SW_MINIMIZE,
    {-32768, -32768},                 // ptMinPosition: x, y
    {-32768, -32768},                 // ptMaxPosition: x, y
    {-32768, -32768, -32768, -32768}, // rcNormalPosition: left, top, right, bottom
  };

  if (!IsWindowVisible(hwnd))
    return false;

  if (IsIconic(hwnd))
    ShowWindow(hwnd, SW_RESTORE);

  HWND foreground_hwnd = GetForegroundWindowTryHarder();
  if (foreground_hwnd == hwnd)
    return true;

  HWND message_hwnd = CreateWindowExW(WS_EX_TOOLWINDOW,
                                      L"Message",
                                      L"Workrave Focus Helper",
                                      (WS_DLGFRAME | WS_DISABLED | WS_POPUP),
                                      wp.rcNormalPosition.left, // X
                                      wp.rcNormalPosition.top,  // Y
                                      0,                        // nWidth
                                      0,                        // nHeight
                                      NULL,                     // hWndParent
                                      NULL,                     // hMenu
                                      GetModuleHandle(NULL),
                                      NULL);

  if (message_hwnd)
    {
      // minimize and restore our dummy window off screen so that it's not visible to the user
      wp.showCmd = SW_MINIMIZE;
      SetWindowPlacement(message_hwnd, &wp);

      wp.showCmd = SW_RESTORE;
      SetWindowPlacement(message_hwnd, &wp);

      /* wait for message_hwnd to become the foreground window. we'll continue regardless of
      whether or not it actually is because we're trying SetForegroundWindow() in any case.
      */
      foreground_hwnd = GetForegroundWindowTryHarder();
    }

  BringWindowToTop(hwnd);
  SetForegroundWindow(hwnd);

  if (message_hwnd)
    {
      DestroyWindow(message_hwnd);
      message_hwnd = NULL;
    }

  foreground_hwnd = GetForegroundWindowTryHarder();
  return (foreground_hwnd == hwnd);
}

/* WindowsForceFocus::ForceWindowFocus()

Create a sacrificial worker thread that attempts to run any of the focus hacks:
AltKeypress(), AttachInput(), MinimizeRestore()

Which of those will be run by the worker thread depend on the value returned by GetFunctions(),
which caches the preference advanced/force_focus_functions. Review its comment block for more.

You can optionally specify how many 'milliseconds_to_block' waiting for the worker thread to exit.
By default this function waits for 200 milliseconds and then returns. If the worker thread is still
running then it may set 'hwnd' as the foreground window after this function has returned.

returns true on success: ( GetForegroundWindow() == hwnd )
*/
bool
WindowsForceFocus::ForceWindowFocus(HWND hwnd,
                                    DWORD milliseconds_to_block // default: 200
)
{
  if (!IsWindowVisible(hwnd))
    return false;

  if (IsIconic(hwnd))
    ShowWindow(hwnd, SW_RESTORE);

  BringWindowToTop(hwnd);
  SetForegroundWindow(hwnd);

  HWND foreground_hwnd = GetForegroundWindow();
  if (foreground_hwnd == hwnd)
    return true;

  /* Check if thread_handle is still open from a prior call.

  If a handle to the last created worker thread is still open then that means that it did not
  terminate within the specified wait period for a prior call to this function to have caught it.

  If the thread has since terminated we can close out the handle and commence with creating a new
  worker thread. If it has not terminated it is still working or has hanged, and I don't think
  there's anything safe to do without affecting the integrity of the process. In that case we
  stop here, and do not try to create another worker or mess with the current, and return false.
  */
  if (thread_handle)
    {
      DWORD thread_exit_code = 0;
      if (!GetExitCodeThread(thread_handle, &thread_exit_code))
        return false;

      if (thread_exit_code == STILL_ACTIVE)
        return false;

      CloseHandle(thread_handle);
      thread_handle = NULL;
    }

  ti.retval = 0;
  ti.hwnd = hwnd;

  ti.flags = GetFunctions();
  if (!ti.flags)
    return false;

  thread_handle = CreateThread(NULL, 0, thread_Worker, (void *)TRUE, 0, NULL);
  if (!thread_handle)
    return false;

  /* We can wait a very short time but not indefinitely because it's possible one of the hacks
  caused our sacrificial worker thread to hang and also we don't want to interrupt our GUI thread.
  */
  if (WaitForSingleObject(thread_handle, milliseconds_to_block) == WAIT_OBJECT_0)
    {
      CloseHandle(thread_handle);
      thread_handle = NULL;
    }

  foreground_hwnd = GetForegroundWindow();
  return (foreground_hwnd == hwnd);
}

/* WindowsForceFocus::thread_Worker()

ThreadProc. Should return 0 in any case.

This is the procedure for the sacrificial worker thread. It reads static struct thread_info for the
hwnd (ti->hwnd) and the functions (ti->flags) that should be used to try to make hwnd the
foreground window.

This function's return is 0, however the actual return value is stored in ti->retval before return.

On return ti->retval contains the flag for whichever WindowsForceFocus function worked to set hwnd the
foreground window or 0 if unsuccessful.
*/
DWORD WINAPI
WindowsForceFocus::thread_Worker(LPVOID recursive)
{
  ti.retval = 0;

  if (!ti.flags || !ti.hwnd)
    return 0;

  /* If our desktop is not visible to the user then do not try to force the focus.
  This call is in the worker thread because it might be more expensive in milliseconds if it has
  to check with an RPC and we don't want to disrupt the GUI thread.
  */
  if (!WindowsCompat::IsOurDesktopVisible())
    return 0;

  // try the safest way first
  if (ti.flags & MINIMIZE_RESTORE)
    {
      if (MinimizeRestore(ti.hwnd))
        {
          ti.retval = MINIMIZE_RESTORE;
          return 0;
        }
    }

  if (ti.flags & ATTACH_INPUT)
    {
      if (AttachInput(ti.hwnd))
        {
          ti.retval = ATTACH_INPUT;
          return 0;
        }
    }

  if (ti.flags & ALT_KEYPRESS)
    {
      if (AltKeypress(ti.hwnd))
        {
          ti.retval = ALT_KEYPRESS;
          return 0;
        }
    }

  /* If we tried minimize and restore and it did not work, and any other method did not work, we
  can try minimize and restore again and the results may be different. As best I can tell this is
  because any of the above methods may "break" the foreground lock but for whatever reason our
  window might not be made the foreground window.

  The way I tested this is to spawn another worker thread and call only the minimize and restore
  and that works.
  */
  if (recursive && (ti.flags & MINIMIZE_RESTORE))
    {
      ti.flags = MINIMIZE_RESTORE;

      HANDLE thread2 = CreateThread(NULL, 0, thread_Worker, (void *)FALSE, 0, NULL);
      if (thread2)
        {
          WaitForSingleObject(thread2, INFINITE);
          CloseHandle(thread2);
        }
    }

  return 0;
}
