// debug.hh
//
// Copyright (C) 2001, 2002, 2003, 2006 Rob Caelers <robc@krandor.org>
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

#ifndef DEBUG_HH
#define DEBUG_HH

#include <assert.h>

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

#endif // DEBUG_HH
