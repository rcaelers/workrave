// debug.hh
//
// Copyright (C) 2001 - 2010 Rob Caelers <robc@krandor.nl>
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

#ifndef TRACING

#define TRACE_ENTER(x)
#define TRACE_ENTER_MSG(x,y)
#define TRACE_RETURN(x)
#define TRACE_EXIT()
#define TRACE_MSG(x)

#else

#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>

#include "Mutex.hh"

extern Mutex g_log_mutex;
extern std::ofstream g_log_stream;

class Debug
{
public:
  static void init();
  static std::string trace_get_time();
};

#define TRACE_ENTER(x   ) g_log_mutex.lock(); \
                          const char *_trace_method_name = x;   \
                          std::cerr << Debug::trace_get_time() << ">>> " << x << std::endl; \
                          g_log_mutex.unlock();

#define TRACE_ENTER_MSG(x, y) g_log_mutex.lock(); \
                          const char *_trace_method_name = x; \
                          std::cerr  << Debug::trace_get_time() << ">>> " << x << " " << y << std::endl; \
                          g_log_mutex.unlock();

#define TRACE_RETURN(y)   g_log_mutex.lock(); \
                          std::cerr << Debug::trace_get_time() << "<<< " << _trace_method_name << y << std::endl; \
                          g_log_mutex.unlock();

#define TRACE_EXIT()      g_log_mutex.lock(); \
                          std::cerr << Debug::trace_get_time() << "<<< " << _trace_method_name << std::endl; \
                          g_log_mutex.unlock();

#define TRACE_MSG(msg)    g_log_mutex.lock(); \
                          std::cerr << Debug::trace_get_time() << "    " << _trace_method_name << " " << msg  << std::endl; \
                          g_log_mutex.unlock();

#endif // TRACING

#endif // DEBUG_HH
