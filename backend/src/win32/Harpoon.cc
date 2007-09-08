// Harpoon.cc --- ActivityMonitor for W32
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007 Raymond Penners <raymond@dotsphinx.com>
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

static const char rcsid[] = "$Id: Harpoon.cc 1298 2007-08-30 14:18:04Z rcaelers $";

#include <assert.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

#include <windows.h>
#include <winuser.h>
#include "debug.hh"
#include "Harpoon.hh"

#include "ICore.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"

#include "timeutil.h"
#include "harpoon.h"

static char critical_filename_list[ HARPOON_MAX_UNBLOCKED_APPS ][ 511 ];

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
      get_value_default( "advanced/harpoon/debug", &debug, false );
  CoreFactory::get_configurator()->
      get_value_default( "advanced/harpoon/mouse_lowlevel", &mouse_lowlevel, false );
  CoreFactory::get_configurator()->
      get_value_default( "advanced/harpoon/keyboard_lowlevel", &keyboard_lowlevel, true );

  if (harpoon_init(critical_filename_list, (BOOL)debug) == FALSE)
    {
      return false;
    }

  if (func != NULL)
    {
      if (harpoon_hook(func, (BOOL)keyboard_lowlevel, (BOOL)mouse_lowlevel) == FALSE)
        {
          return false;
        }
    }
  return true;
}


//! Stops the activity monitoring.
void
Harpoon::terminate()
{
  harpoon_exit();
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
      get_value( "advanced/critical_files/filecount", &filecount) || !filecount )
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
          get_value( loc, &buffer) )
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
      if( p = _mbschr( p2, '\"' ) )
          *p = '\0';
    }
  else
  // e.g. debugger.exe /y /x
    {
      if( p = _mbschr( p2, ' ' ) )
          *p = '\0';
    }

  // Search the path to find where the filename starts:
  if( p = (unsigned char *)_mbsrchr( p2, '\\' ) )
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
