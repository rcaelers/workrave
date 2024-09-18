// Harpoon.cc --- ActivityMonitor for W32
//
// Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
// Copyright (C) 2007, 2008, 2010 Rob Caelers <robc@krandor.org>
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

#define _CRT_SECURE_NO_WARNINGS
#include <string>

#include <windows.h>
#include <winuser.h>
#include "HarpoonHelper.h"
#include "Debug.h"
#include "Config.h"

using namespace std;

HarpoonHelper::HarpoonHelper(char *args)
  : args(args)
{
}

HarpoonHelper::~HarpoonHelper()
{
  terminate();
}

bool
HarpoonHelper::init(HINSTANCE hInstance)
{
  TRACE_ENTER("HarpoonHelper::init");
  this->hInstance = hInstance;

  DWORD dwStyle, dwExStyle;

  dwStyle = WS_OVERLAPPED;
  dwExStyle = WS_EX_TOOLWINDOW;

  WNDCLASSEX wclass =
    {sizeof(WNDCLASSEX), 0, harpoon_window_proc, 0, 0, hInstance, NULL, NULL, NULL, NULL, HARPOON_HELPER_WINDOW_CLASS, NULL};

  notification_class = RegisterClassEx(&wclass);
  if (!notification_class)
    return FALSE;

  notification_window = CreateWindowEx(dwExStyle,
                                       HARPOON_HELPER_WINDOW_CLASS,
                                       HARPOON_HELPER_WINDOW_CLASS,
                                       dwStyle,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       NULL,
                                       NULL,
                                       hInstance,
                                       NULL);

  if (!notification_window)
    {
      UnregisterClass(HARPOON_WINDOW_CLASS, hInstance);
      notification_class = 0;
      return FALSE;
    }

  init_critical_filename_list();

  bool debug = false;
  bool mouse_lowlevel = true;
  bool keyboard_lowlevel = true;

  Config config;
  config.get_value("advanced/harpoon/debug", debug);
  config.get_value("advanced/harpoon/mouse_lowlevel", mouse_lowlevel);
  config.get_value("advanced/harpoon/keyboard_lowlevel", keyboard_lowlevel);

  if (!harpoon_init(critical_filename_list, (BOOL)debug))
    {
      TRACE_RETURN(false);
      return false;
    }

  TRACE_RETURN(true);
  return true;
}

//! Stops the activity monitoring.
void
HarpoonHelper::terminate()
{
  harpoon_exit();
}

void
HarpoonHelper::run()
{
  TRACE_ENTER("HarpoonHelper::run");
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  TRACE_EXIT();
}

void
HarpoonHelper::init_critical_filename_list()
{
  TRACE_ENTER("HarpoonHelper::init_critical_filename_list");
  Config config;
  int i;

  for (i = 0; i < HARPOON_MAX_UNBLOCKED_APPS; ++i)
    critical_filename_list[i][0] = '\0';

  // Task Manager is always on the critical_filename_list
  if (!check_for_taskmgr_debugger(critical_filename_list[0]))
    {
      strcpy(critical_filename_list[0], "taskmgr.exe");
    }

  strcpy(critical_filename_list[1], "workrave.exe");
  strcpy(critical_filename_list[2], args);

  TRACE_MSG(args);

  int filecount = 0;
  config.get_value("advanced/critical_files/filecount", filecount);

  if (filecount > 0)
    {
      if (filecount >= HARPOON_MAX_UNBLOCKED_APPS - 2)
        {
          filecount = HARPOON_MAX_UNBLOCKED_APPS - 3;
        }

      char loc[40];
      string buffer;

      for (i = 1; i <= filecount; ++i)
        {
          sprintf(loc, "advanced/critical_files/file%d", i);
          if (config.get_value(loc, buffer))
            {
              strcpy_s(critical_filename_list[i + 2], 510, buffer.c_str());
              critical_filename_list[i][510] = '\0';
            }
        }
    }
  TRACE_EXIT();
}

bool
HarpoonHelper::check_for_taskmgr_debugger(char *out)
{
  HKEY hKey = NULL;
  LONG err;
  DWORD size;
  unsigned char *p, *p2, *buffer;

  // If there is a debugger for taskmgr, it's always critical
  err = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\"
                      "Image File Execution Options\\taskmgr.exe",
                      0,
                      KEY_QUERY_VALUE,
                      &hKey);

  if (err != ERROR_SUCCESS)
    {
      RegCloseKey(hKey);
      return false;
    }

  // get the size, in bytes, required for buffer
  err = RegQueryValueExA(hKey, "Debugger", NULL, NULL, NULL, &size);

  if (err != ERROR_SUCCESS || !size)
    {
      RegCloseKey(hKey);
      return false;
    }

  if (!(buffer = (unsigned char *)malloc(size + 1)))
    {
      RegCloseKey(hKey);
      return false;
    }

  err = RegQueryValueExA(hKey, "Debugger", NULL, NULL, (LPBYTE)buffer, &size);

  if (err != ERROR_SUCCESS || !size)
    {
      free(buffer);
      RegCloseKey(hKey);
      return false;
    }

  buffer[size] = '\0';

  // get to innermost quoted
  for (p2 = buffer; *p2 == '\"'; ++p2)
    ;
  if (p2 != buffer)
    // e.g. "my debugger.exe" /y /x
    {
      if ((p = _mbschr(p2, '\"')))
        *p = '\0';
    }
  else
    // e.g. debugger.exe /y /x
    {
      if ((p = _mbschr(p2, ' ')))
        *p = '\0';
    }

  // Search the path to find where the filename starts:
  if ((p = (unsigned char *)_mbsrchr(p2, '\\')))
    // Point to first (mb) filename character
    ++p;
  else
    // No path.
    p = p2;

  _mbstrncpy_lowercase(out, (char *)p, 510);
  out[510] = '\0';

  RegCloseKey(hKey);
  free(buffer);
  return true;
}

LRESULT CALLBACK
HarpoonHelper::harpoon_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  TRACE_ENTER("HarpoonHelper::harpoon_window_proc");
  int evt_type;
  evt_type = uMsg - WM_USER;

  TRACE_MSG(evt_type);
  if (evt_type >= 0 && evt_type < HARPOON_HELPER_EVENT__SIZEOF)
    {
      switch ((HarpoonHelperEventType)evt_type)
        {
        case HARPOON_HELPER_INIT:
          TRACE_MSG("init");
          break;

        case HARPOON_HELPER_EXIT:
          TRACE_MSG("exit");
          PostQuitMessage(0);
          break;

        case HARPOON_HELPER_BLOCK:
          TRACE_MSG("block");
          harpoon_block_input();
          break;

        case HARPOON_HELPER_UNBLOCK:
          TRACE_MSG("unblock");
          harpoon_unblock_input();
          break;

        case HARPOON_HELPER_NOTHING:
        case HARPOON_HELPER_EVENT__SIZEOF:
          break;
        }
    }

  TRACE_EXIT();
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
