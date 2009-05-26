// debug.hh
//
// Copyright (C) 2001, 2002, 2003, 2006, 2007 Rob Caelers <robc@krandor.nl>
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

#define TRACE_ENTER(x   ) g_logMutex.lock(); \
                          const char *debugMethod = x;   \
                          std::cerr << ">>> " << x << std::endl; \
                          g_logMutex.unlock();

#define TRACE_ENTER_MSG(x, y) g_logMutex.lock(); \
                          const char *debugMethod = x; \
                          std::cerr << ">>> " << x << " " << y << std::endl; \
                          g_logMutex.unlock();

#define TRACE_RETURN(y)   g_logMutex.lock(); \
                          std::cerr << "<<< " << debugMethod << y << std::endl; \
                          g_logMutex.unlock();

#define TRACE_EXIT()      g_logMutex.lock(); \
                          std::cerr << "<<< " << debugMethod << std::endl; \
                          g_logMutex.unlock();

#define TRACE_MSG(msg)    g_logMutex.lock(); \
                          std::cerr << "    " << debugMethod << " " << msg  << std::endl; \
                          g_logMutex.unlock();

#endif // NDEBUG

#endif // DEBUG_HH
