#if defined(WIN32) || defined(PLATFORM_OS_WIN32)

#ifndef BACKEND
#include <gdk/gdkwin32.h>
#endif
#include <windows.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>

using namespace std;

#define DEBUGTXT "c:\\temp\\workrave.txt"

#define APPEND(x, y)	{ \
	ofstream append; \
	append.open( DEBUGTXT, std::ios::app ); \
	if( append.is_open() ) \
	{ \
		append << x << ": " << y << endl; \
		append.close(); \
	} \
}

#define APPEND_ENDL()	{ \
	ofstream append; \
	append.open( DEBUGTXT, std::ios::app ); \
	if( append.is_open() ) \
	{ \
		append << endl; \
		append.close(); \
	} \
}

#define APPEND_DATE()	{ \
	struct tm *_localtime; \
	time_t _time; \
	_time = time( NULL ); \
	_localtime = localtime( &_time ); \
	\
	ofstream append; \
	append.open( DEBUGTXT, std::ios::app ); \
	if( append.is_open() ) \
	{ \
		append << asctime( _localtime ); \
		append.close(); \
	} \
}

#define APPEND_TIME(x, y)	{ \
	SYSTEMTIME local; \
	char mer[ 3 ] = "AM"; \
	\
	GetLocalTime( &local ); \
	if( local.wHour > 12 ) \
	{ \
		local.wHour = local.wHour - 12; \
		mer[ 0 ] = 'P'; \
	} \
	else if( local.wHour == 0 ) \
		local.wHour = (WORD)12; \
	\
	ofstream append; \
	append.open( DEBUGTXT, std::ios::app ); \
	if( append.is_open() ) \
	{ \
		append << setfill( '0' ); \
		append << setw( 2 ) << local.wHour << ":" \
		<< setw( 2 ) << local.wMinute << ":" \
		<< setw( 2 ) << local.wSecond \
		<< " " << mer << ":\t" << x << ": " << y << endl; \
		append.close(); \
	} \
}


#else

#define APPEND(x, y)					{ 1; }
#define APPEND_ENDL()				{ 1; }
#define APPEND_DATE()				{ 1; }
#define APPEND_TIME(x, y)		{ 1; }

#endif
