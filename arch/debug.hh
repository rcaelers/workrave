// debug.hh 
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-11-11 23:30:14 robc>
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

#ifndef DEBUG_HH
#define DEBUG_HH

#ifndef CWDEBUG

#define AllocTag1(p)
#define AllocTag2(p, desc)
#define AllocTag_dynamic_description(p, x)
#define AllocTag(p, x)
#define Debug(x)
#define Dout(a, b)
#define DoutFatal(a, b) LibcwDoutFatal(::std, , a, b)
#define ForAllDebugChannels(STATEMENT)
#define ForAllDebugObjects(STATEMENT)
#define LibcwDebug(dc_namespace, x)
#define LibcwDout(a, b, c, d)
#define LibcwDoutFatal(a, b, c, d) do { ::std::cerr << d << ::std::endl; ::std::exit(254); } while(1)
#define NEW(x) new x
#define CWDEBUG_ALLOC 0
#define CWDEBUG_MAGIC 0
#define CWDEBUG_LOCATION 0
#define CWDEBUG_LIBBFD 0
#define CWDEBUG_DEBUG 0
#define CWDEBUG_DEBUGOUTPUT 0
#define CWDEBUG_DEBUGM 0
#define CWDEBUG_DEBUGT 0
#define CWDEBUG_MARKER 0

#ifdef NDEBUG

#define TRACE_ENTER(x)
#define TRACE_ENTER_MSG(x,y)
#define TRACE_RETURN(x)
#define TRACE_EXIT()
#define TRACE_MSG(x)

#else

#include <iostream>
#include <iomanip>

#include "Mutex.hh"

extern Mutex g_logMutex;

using namespace std;


#define TRACE_ENTER(x   ) g_logMutex.lock(); \
                          char *debugMethod = x; \
                          cerr << ">>> " << x << endl; \
                          g_logMutex.unlock();

#define TRACE_ENTER_MSG(x, y) g_logMutex.lock(); \
                          char *debugMethod = x; \
                          cerr << ">>> " << x << " " << y << endl; \
                          g_logMutex.unlock();

#define TRACE_RETURN(y)   g_logMutex.lock(); \
                          cerr << "<<< " << debugMethod << y << endl; \
                          g_logMutex.unlock();

#define TRACE_EXIT()      g_logMutex.lock(); \
                          cerr << "<<< " << debugMethod << endl; \
                          g_logMutex.unlock();

#define TRACE_MSG(msg)    g_logMutex.lock(); \
                          cerr << "    " << debugMethod << " " << msg  << endl; \
                          g_logMutex.unlock();

#endif // NDEBUG

#else // CWDEBUG

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <libcw/sysd.h>
#include <iomanip>

#define TRACE_ENTER(x   ) char *debugMethod = x; \
                          Dout(dc::trace, ">>> " << x);

#define TRACE_ENTER_MSG(x, y) char *debugMethod = x; \
                          Dout(dc::trace, ">>> " << x << " " << y);

#define TRACE_RETURN(y)   Dout(dc::trace, "<<< " << debugMethod << y);

#define TRACE_EXIT()      Dout(dc::trace, "<<< " << debugMethod);

#define TRACE_MSG(msg)    Dout(dc::trace, "    " << debugMethod << " " << msg);


#ifndef DEBUGCHANNELS
// This must be defined before <libcw/debug.h> is included and must be the
// name of the namespace containing your `dc' (Debug Channels) namespace
// (see below).  You can use any namespace(s) you like, except existing
// namespaces (like ::, ::std and ::libcwd).
#define DEBUGCHANNELS ::workrave::debug::channels
#endif
#include <libcw/debug.h>

namespace workrave
{
  namespace debug
  {
    namespace channels
    {
      namespace dc
      {
	using namespace ::libcw::debug::channels::dc;

	extern ::libcw::debug::channel_ct trace;
      }
    }
  }
}

#endif // CWDEBUG

#endif // DEBUG_HH
