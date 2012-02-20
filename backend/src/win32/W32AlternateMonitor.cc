// W32AlternateMonitor.cc --- Alternate Activity monitor for win32
//
// Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
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
// This file contains code necessary to bypass mouse & keyboard
// hooks normally required by Workrave.
//
// Upside: no hooks!
// Downside: no mouse & keyboard statistics.
//
// jay satiro, workrave project, may 2007
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(PLATFORM_OS_WIN32)

#include "debug.hh"

#include <sstream>

#if !defined(PLATFORM_OS_WIN32_NATIVE)
#include <unistd.h>
#endif

#include "W32AlternateMonitor.hh"
#include "Harpoon.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"

using namespace workrave;


W32AlternateMonitor::W32AlternateMonitor() :
	initialized( false ),
	interval( 500 ),
	thread_abort_event( NULL ),
	thread_handle( NULL ),
	thread_id( 0 )
{
  TRACE_ENTER( "W32AlternateMonitor::W32AlternateMonitor" );

  TRACE_EXIT();
}


W32AlternateMonitor::~W32AlternateMonitor()
{
  TRACE_ENTER( "W32AlternateMonitor::~W32AlternateMonitor" );

  terminate();

  TRACE_EXIT();
}


bool W32AlternateMonitor::init()
{
	TRACE_ENTER( "W32AlternateMonitor::init" );

	if( initialized )
		goto cleanup;

	CoreFactory::get_configurator()->get_value_with_default( "advanced/interval", interval, 500 );

	SetLastError( 0 );
	thread_abort_event = CreateEvent( NULL, FALSE, FALSE, NULL );
	if( !thread_abort_event )
	{
		TRACE_MSG( "Thread abort event could not be created. GetLastError : " << GetLastError() );
		goto cleanup;
	}

	thread_id = 0;
	SetLastError( 0 );
	thread_handle = CreateThread( NULL, 0, thread_Monitor, this, 0, (DWORD *)&thread_id );
	if( !thread_handle || !thread_id )
	{
		TRACE_MSG( "Thread could not be created. GetLastError : " << GetLastError() );
		goto cleanup;
	}

	Harpoon::init( NULL );
	
	initialized = true;

cleanup:
	if( initialized == false )
		terminate();

	TRACE_EXIT();
	return initialized;
}


void W32AlternateMonitor::terminate()
{
	TRACE_ENTER( "W32AlternateMonitor::terminate" );

	thread_id = 0;

	if( thread_handle )
	{
		SetEvent( thread_abort_event );
		WaitForSingleObject( thread_handle, INFINITE );
		CloseHandle( thread_handle );
		thread_handle = NULL;
	}

	if( thread_abort_event )
	{
		CloseHandle( thread_abort_event );
		thread_abort_event = NULL;
	}

	Harpoon::terminate();
	
	initialized = false;

	TRACE_EXIT();
}


DWORD WINAPI W32AlternateMonitor::thread_Monitor( LPVOID lpParam )
{
  W32AlternateMonitor *pThis = (W32AlternateMonitor *) lpParam;
  pThis->Monitor();
  // invalidate the id to signal the thread is exiting
  pThis->thread_id = 0;
  return (DWORD) 0;
}


void W32AlternateMonitor::Monitor()
{
  const DWORD current_thread_id = GetCurrentThreadId();

  TRACE_ENTER_MSG( "W32AlternateMonitor::Monitor [ id: ", current_thread_id << " ]" );

  SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL );


  LASTINPUTINFO lii;
  DWORD dwPreviousTime = 0;

  lii.cbSize = sizeof( lii );
  lii.dwTime = GetTickCount();
  
  while( thread_id == current_thread_id )
  /* Main loop */
  {
    dwPreviousTime = lii.dwTime;

    if( GetLastInputInfo( &lii ) && ( lii.dwTime > dwPreviousTime ) )
    /* User session has received input */
    {
      /* Notify the activity monitor */
      fire_action();
    }

    if( WaitForSingleObject( thread_abort_event, interval ) != WAIT_TIMEOUT )
      break;
  }

  TRACE_EXIT();
}

#endif // defined(PLATFORM_OS_WIN32)
