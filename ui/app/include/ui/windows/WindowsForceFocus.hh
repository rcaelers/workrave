// WindowsForceFocus.hh --- Collection of hacks to force win32 window focus
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

#ifndef W32FORCEFOCUS_HH
#define W32FORCEFOCUS_HH

#include "config/IConfigurator.hh"
#include <windows.h>

class WindowsForceFocus
{
public:
  static void init(workrave::config::IConfigurator::Ptr config);

  // Simulate an ALT keypress in an attempt to make hwnd foreground
  static bool AltKeypress(HWND hwnd);

  // Attach to the thread of the current foreground window in an attempt to make hwnd foreground
  static bool AttachInput(HWND hwnd);

  // Minimize and restore a dummy window in an attempt to make hwnd foreground
  static bool MinimizeRestore(HWND hwnd);

  // Create a worker thread that runs any of the above hacks in an attempt to make hwnd foreground
  static bool ForceWindowFocus(HWND hwnd, DWORD milliseconds_to_block = 200);

  // Check the user preference advanced/force_focus
  static bool GetForceFocusValue();

private:
  // Get the flags of functions that should be used to make hwnd foreground
  static DWORD GetFunctions();

  // Retry the GetForegroundWindow() function if it fails
  static HWND GetForegroundWindowTryHarder(DWORD max_retries = 200);

  // This is the ThreadProc called by CreateThread to create the worker thread
  static DWORD WINAPI thread_Worker(LPVOID lpParameter);

  // A handle to the worker thread
  static HANDLE thread_handle;

  // Info needed by thread_Worker
  static struct thread_info
  {
    // the hwnd we want to force focus to
    HWND hwnd; // in

    // the functions we want to call to force the focus
    DWORD flags; // in, out (ThreadProc may change the flags for recursive calls)

    // the function that worked to force the focus, if any
    DWORD retval; // out
  } ti;

  // Cached value of user preference advanced/force_focus
  static bool force_focus;

  static inline workrave::config::IConfigurator::Ptr config;
};

#endif // W32FORCEFOCUS_HH
