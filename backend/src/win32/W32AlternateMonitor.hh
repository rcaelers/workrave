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
// See comments in W32AlternateMonitor.cc

#ifndef W32ALTERNATEMONITOR_HH
#define W32ALTERNATEMONITOR_HH

#include <windows.h>

#include "ICore.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"

#include "IInputMonitor.hh"
#include "IInputMonitorListener.hh"

class W32AlternateMonitor : 
	public IInputMonitor
{
public:
	W32AlternateMonitor();
	virtual ~W32AlternateMonitor(); //really?
	void init( IInputMonitorListener * );
	void terminate();

protected:
	static DWORD WINAPI thread_Monitor( LPVOID );

private:
	void Monitor();
	void Update( LASTINPUTINFO * );
	void msg( char * );
	void exitmsg( char * );
	
	BOOL ( WINAPI *GetLastInputInfo ) ( LASTINPUTINFO * );
	IInputMonitorListener *listener; //volatile?
	int interval;
};

#endif // W32ALTERNATEMONITOR_HH
