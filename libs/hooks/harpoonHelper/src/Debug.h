// Copyright (C) 2001 - 2011 Rob Caelers <robc@krandor.nl>
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

#if defined(NDEBUG)

#  define TRACE_ENTER(x)
#  define TRACE_ENTER_MSG(x, y)
#  define TRACE_RETURN(x)
#  define TRACE_EXIT()
#  define TRACE_MSG(x)

#else

#  include <iostream>
#  include <iomanip>
#  include <fstream>
#  include <ctime>
#  include <string>

extern std::ofstream g_log_stream;

class Debug
{
public:
  static void init();
  static std::string trace_get_time();
};

#  define TRACE_ENTER(x)                                              \
    const char *_trace_method_name = x;                               \
    std::cerr << Debug::trace_get_time() << ">>> " << x << std::endl; \
    std::cerr.flush();

#  define TRACE_ENTER_MSG(x, y)                                                   \
    const char *_trace_method_name = x;                                           \
    std::cerr << Debug::trace_get_time() << ">>> " << x << " " << y << std::endl; \
    std::cerr.flush();

#  define TRACE_RETURN(y)                                                                   \
    std::cerr << Debug::trace_get_time() << "<<< " << _trace_method_name << y << std::endl; \
    std::cerr.flush();

#  define TRACE_EXIT()                                                                 \
    std::cerr << Debug::trace_get_time() << "<<< " << _trace_method_name << std::endl; \
    std::cerr.flush();

#  define TRACE_MSG(msg)                                                                             \
    std::cerr << Debug::trace_get_time() << "    " << _trace_method_name << " " << msg << std::endl; \
    std::cerr.flush();

#endif // HAVE_TRACING
#endif // DEBUG_H
