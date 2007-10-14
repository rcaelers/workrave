// W32AlternateMonitor.cc --- Alternate Activity monitor for win32
//
// Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
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
// This file contains code necessary to bypass mouse & keyboard
// hooks normally required by Workrave.
//
// Upside: no hooks!
// Downside: no mouse & keyboard statistics.
//
// jay satiro, workrave project, may 2007
//

#if defined(WIN32)

#include "debug.hh"

#include <sstream>
#include <unistd.h>

#include "W32AlternateMonitor.hh"
#include "Harpoon.hh"

volatile IInputMonitorListener *W32AlternateMonitor::listener = NULL;

W32AlternateMonitor::W32AlternateMonitor()
{
  TRACE_ENTER( "W32AlternateMonitor::W32AlternateMonitor" );
  
  listener = NULL;
  
  CoreFactory::get_configurator()->get_value_with_default( "advanced/interval", interval, 500);
  TRACE_EXIT();
}

W32AlternateMonitor::~W32AlternateMonitor()
{
  TRACE_ENTER( "W32AlternateMonitor::~W32AlternateMonitor" );
  TRACE_EXIT();
}


bool W32AlternateMonitor::init( IInputMonitorListener *l )
{
  TRACE_ENTER( "W32AlternateMonitor::init" );
  
  DWORD id;
  HANDLE hThread;

  if( listener != NULL )
  /*
  Calling terminate (listener = NULL), will cause the
  monitor thread to exit. This shouldn't come up though.
  */
  {
    TRACE_MSG( " listener != NULL " );
    msg( " W32AlternateMonitor::init : listener != NULL " );
    terminate();
    Sleep( interval * 2 + 1000 ); // wait for thread to terminate
  }
  listener = l;
  
  /* Try to get the address of GetLastInputInfo()... */
  GetLastInputInfo = ( BOOL ( WINAPI * ) ( LASTINPUTINFO * ) )
    GetProcAddress( GetModuleHandleA( "user32.dll" ), "GetLastInputInfo" );
  
  if( GetLastInputInfo == NULL )
  /*
    GetLastInputInfo() is only available in Win2000 or better.
    AdvancedPreferencePage and ActivityMonitor check the OS using
    GetVersion(), so it shouldn't come to this.
  */
  {
    //msg( "GetLastInputInfo() address not found in user32.dll" );
    TRACE_MSG( "GetLastInputInfo() address not found in user32.dll" );
    TRACE_EXIT();
    return false;
  }
  
  hThread = CreateThread( NULL, 0, thread_Monitor, this, 0, &id );
  
  if( hThread == NULL )
  {
    //msg( "W32AlternateMonitor::init() : Thread could not be created.\n" );
    TRACE_MSG( "Thread could not be created. GetLastError : " << GetLastError() );
    TRACE_EXIT();
    return false;
  }
  
  /* Close the handle now, we don't need it anymore. */
  CloseHandle( hThread );
  
  TRACE_EXIT();

  Harpoon::init(NULL);
  return true;
}

void W32AlternateMonitor::terminate()
{
  TRACE_ENTER( "W32AlternateMonitor::terminate" );
  listener = NULL;
  TRACE_EXIT();
}

DWORD WINAPI W32AlternateMonitor::thread_Monitor( LPVOID lpParam )
{
  W32AlternateMonitor *pThis = (W32AlternateMonitor *) lpParam;
  pThis->Monitor();
  return (DWORD) 0;
}

void W32AlternateMonitor::Monitor()
{
  TRACE_ENTER( "W32AlternateMonitor::Monitor" );
  
  LASTINPUTINFO lii;
  DWORD dwPreviousTime;
  
  SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL );
  
  lii.cbSize = sizeof( lii ); 
  
  Update( &lii );
  
  
  while( listener != NULL )
  /* Main loop */
  {
    dwPreviousTime = lii.dwTime;
    Update( &lii );
    
    if( lii.dwTime > dwPreviousTime )
    /* User session has received input */
    {
      /* Notify the activity monitor */
       const_cast<IInputMonitorListener *>( listener )->action_notify();
    }
    
    Sleep( interval );
  }
  
  TRACE_EXIT();
}


inline void W32AlternateMonitor::Update( LASTINPUTINFO *p )
{
  while( ( *GetLastInputInfo ) ( p ) == 0 && listener != NULL )
  /*
  If GetLastInputInfo errors out, sleep & try again.
  When will this ever happen though?
  */
  {
          // TRACE_MSG( "GetLastInputInfo() failed." );
    Sleep( interval + rand() % 1000 );
  }
}

void W32AlternateMonitor::msg( char *msg )
{
  MessageBoxA( NULL, msg, "Workrave: Debug Message", MB_OK );
}

void W32AlternateMonitor::exitmsg( char *msg )
{
  TRACE_ENTER( "W32AlternateMonitor::exitmsg" );
  TRACE_MSG( msg );
  MessageBoxA( NULL, msg, "Workrave: Unrecoverable Error. Exiting...", MB_OK );
  
  TRACE_EXIT();
  exit( 0 );
}

#endif // defined(WIN32)
