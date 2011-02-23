// Harpoon.cc --- ActivityMonitor for W32
//
// Copyright (C) 2007, 2010 Ray Satiro <raysatiro@yahoo.com>
// Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.org>
// Copyright (C) 2010 Rob Caelers <robc@krandor.nl>
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

#include <assert.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

#include <windows.h>
#include <winuser.h>

#include <stdio.h>
#include <tchar.h>

#include "debug.hh"
#include "Harpoon.hh"

#include "ICore.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"
#include "Util.hh"

#include "timeutil.h"
#include "harpoon.h"
#include "HarpoonHelper.h"

using namespace workrave;
using namespace std;

char Harpoon::critical_filename_list[HARPOON_MAX_UNBLOCKED_APPS][511];
HWND Harpoon::helper_window = NULL;
bool Harpoon::helper_started = false;

Harpoon::Harpoon()
{
}


Harpoon::~Harpoon()
{
  terminate();
}


bool
Harpoon::init(HarpoonHookFunc func)
{
  assert( HARPOON_MAX_UNBLOCKED_APPS );
  init_critical_filename_list();

  bool debug, mouse_lowlevel, keyboard_lowlevel;

  CoreFactory::get_configurator()->
      get_value_with_default( "advanced/harpoon/debug", debug, false );

  bool default_mouse_lowlevel = false;
  if ( LOBYTE( LOWORD( GetVersion() ) ) >= 6)
    {
      default_mouse_lowlevel = true;
    }
  
  CoreFactory::get_configurator()->
      get_value_with_default( "advanced/harpoon/mouse_lowlevel", mouse_lowlevel, default_mouse_lowlevel );

  CoreFactory::get_configurator()->
      get_value_with_default( "advanced/harpoon/keyboard_lowlevel", keyboard_lowlevel, true );

  if (!harpoon_init(critical_filename_list, (BOOL)debug))
    {
      return false;
    }

  if (func != NULL)
    {
      if (!harpoon_hook(func, (BOOL)keyboard_lowlevel, (BOOL)mouse_lowlevel))
        {
          return false;
        }
    }

#if 1
  if (is_64bit_windows())
    {
       start_harpoon_helper();
    }
#endif

  return true;
}


//! Stops the activity monitoring.
void
Harpoon::terminate()
{
  stop_harpoon_helper();
  harpoon_exit();
}


void 
Harpoon::block_input()
{
  harpoon_block_input();

  if (helper_started)
    {
      if (helper_window == NULL)
        {
          helper_window = recursive_find_window(NULL, HARPOON_HELPER_WINDOW_CLASS);
        }

      if (helper_window != NULL)
        {
          PostMessage(helper_window, WM_USER + HARPOON_HELPER_BLOCK, 0, 0);
        }
    }
}

void 
Harpoon::unblock_input()
{
  harpoon_unblock_input();
  if (helper_started)
    {
      if (helper_window == NULL)
        {
          helper_window = recursive_find_window(NULL, HARPOON_HELPER_WINDOW_CLASS);
        }

      if (helper_window != NULL)
        {
          PostMessage(helper_window, WM_USER + HARPOON_HELPER_UNBLOCK, 0, 0);
        }
    }
}

void
Harpoon::init_critical_filename_list()
{
  int i, filecount;

  // Task Manager is always on the critical_filename_list
  if( GetVersion() >= 0x80000000 )
  // Windows Me/98/95
      strcpy( critical_filename_list[ 0 ], "taskman.exe" );
  else if( !check_for_taskmgr_debugger( critical_filename_list[ 0 ] ) )
      strcpy( critical_filename_list[ 0 ], "taskmgr.exe" );

  for( i = 1; i < HARPOON_MAX_UNBLOCKED_APPS; ++i )
      critical_filename_list[ i ][ 0 ] = '\0';

  filecount = 0;
  if( !CoreFactory::get_configurator()->
      get_value( "advanced/critical_files/filecount", filecount) || !filecount )
          return;

  if( filecount >= HARPOON_MAX_UNBLOCKED_APPS )
  // This shouldn't happen
    {
      filecount = HARPOON_MAX_UNBLOCKED_APPS - 1;
      CoreFactory::get_configurator()->
          set_value( "advanced/critical_files/filecount", filecount );
    }

  char loc[40];
  string buffer;
  for( i = 1; i <= filecount; ++i )
    {
      sprintf( loc, "advanced/critical_files/file%d", i );
      if( CoreFactory::get_configurator()->
          get_value( loc, buffer) )
        {
          strncpy( critical_filename_list[ i ], buffer.c_str(), 510 );
          critical_filename_list[ i ][ 510 ] = '\0';
        }
    }
}


bool
Harpoon::check_for_taskmgr_debugger( char *out )
{
  HKEY hKey = NULL;
  LONG err;
  DWORD size;
  unsigned char *p, *p2, *buffer;

  // If there is a debugger for taskmgr, it's always critical
  err = RegOpenKeyExA( HKEY_LOCAL_MACHINE,
      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\"
      "Image File Execution Options\\taskmgr.exe",
      0, KEY_QUERY_VALUE, &hKey );

  if( err != ERROR_SUCCESS )
    {
      RegCloseKey( hKey );
      return false;
    }

  // get the size, in bytes, required for buffer
  err = RegQueryValueExA( hKey, "Debugger", NULL, NULL, NULL, &size );

  if( err != ERROR_SUCCESS || !size )
    {
      RegCloseKey( hKey );
      return false;
    }

  if( !( buffer = (unsigned char *)malloc( size + 1 ) ) )
    {
      RegCloseKey( hKey );
      return false;
    }

  err = RegQueryValueExA( hKey, "Debugger", NULL, NULL, (LPBYTE)buffer, &size );

  if( err != ERROR_SUCCESS || !size )
    {
      free( buffer );
      RegCloseKey( hKey );
      return false;
    }

  buffer[ size ] = '\0';

  // get to innermost quoted
  for( p2 = buffer; *p2 == '\"'; ++p2 )
  ;
  if( p2 != buffer )
  // e.g. "my debugger.exe" /y /x
    {
      if( (p = _mbschr( p2, '\"' )) )
          *p = '\0';
    }
  else
  // e.g. debugger.exe /y /x
    {
      if( (p = _mbschr( p2, ' ' )) )
          *p = '\0';
    }

  // Search the path to find where the filename starts:
  if( (p = (unsigned char *)_mbsrchr( p2, '\\' )) )
  // Point to first (mb) filename character
      ++p;
  else
  // No path.
      p = p2;

  _mbstrncpy_lowercase( out, (char *)p, 510 );
  out[ 510 ] = '\0';

  RegCloseKey( hKey );
  free( buffer );
  return true;
}


bool 
Harpoon::is_64bit_windows()
{
#if defined(_WIN64)
  return TRUE;  // 64-bit programs run only on Win64
#elif defined(_WIN32)
  BOOL f64 = FALSE;

  typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
  LPFN_ISWOW64PROCESS fnIsWow64Process;

  fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandleA("kernel32.dll"), "IsWow64Process");
  if (fnIsWow64Process != NULL)
    {
      return fnIsWow64Process(GetCurrentProcess(), &f64) && f64;
    }
  else
    {
      return FALSE;
    }
#else
  return FALSE; // Win64 does not support Win16
#endif
}

HWND
Harpoon::recursive_find_window(HWND hwnd, LPCTSTR lpClassName)
{
  TRACE_ENTER("Harpoon::recursive_find_window");
  static char buf[80];
  int num = GetClassName(hwnd, buf, sizeof(buf)-1);
  buf[num] = 0;
  HWND ret = NULL;

  TRACE_MSG(buf);
  if (! stricmp(lpClassName, buf))
    {
      ret =  hwnd;
    }
  else
    {
      HWND child = FindWindowEx(hwnd, 0, NULL, NULL);
      while (child != NULL)
        {
          ret = recursive_find_window(child, lpClassName);
          if (ret)
            {
              break;
            }
          child = FindWindowEx(hwnd, child, NULL, NULL);
        }
    }
  TRACE_EXIT();
  return ret;
}


void
Harpoon::start_harpoon_helper()
{
  TRACE_ENTER("Harpoon::start_harpoon_helper" );

  if (helper_window == NULL)
    {
      helper_window = recursive_find_window(NULL, HARPOON_HELPER_WINDOW_CLASS);
      if (helper_window != NULL)
        {
          helper_started = true;
        }
    }

  if (!helper_started)
    {
      STARTUPINFO si;
      PROCESS_INFORMATION pi;

      ZeroMemory(&si, sizeof(si));
      si.cb = sizeof(si);

      ZeroMemory(&pi, sizeof(pi));

      string install_dir = g_win32_get_package_installation_directory_of_module(NULL);
      string helper = install_dir + G_DIR_SEPARATOR_S + "lib" + G_DIR_SEPARATOR_S + "HarpoonHelper.exe";
      string args = helper + " " + g_get_prgname();

      TRACE_MSG(install_dir.c_str());
      TRACE_MSG(helper.c_str());
      TRACE_MSG(args.c_str());
      
      if (CreateProcessA(helper.c_str(), (LPSTR) args.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) 
        {
          helper_started = true;
          helper_window = recursive_find_window(NULL, HARPOON_HELPER_WINDOW_CLASS);
        }
      else
        {
          TRACE_MSG("CreateProcess failed " << GetLastError());
        }

      TRACE_MSG(pi.hProcess);
      TRACE_MSG(pi.hThread);
    }
    TRACE_EXIT();
}

void
Harpoon::stop_harpoon_helper()
{
  TRACE_ENTER("Harpoon::stop_harpoon_helper" );
  if (helper_window != NULL)
    {
      PostMessage(helper_window, WM_USER + HARPOON_HELPER_EXIT, 0, 0);
      helper_window = NULL;
      helper_started = false;
    }
  TRACE_EXIT();
}
