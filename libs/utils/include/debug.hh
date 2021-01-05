// debug.hh
//
// Copyright (C) 2001 - 2010, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UTILS_DEBUG_HH
#define WORKRAVE_UTILS_DEBUG_HH

#include <cassert>

#ifndef TRACING

#  define TRACE_ENTER(x)
#  define TRACE_ENTER_MSG(x, y)
#  define TRACE_RETURN(x)
#  define TRACE_EXIT()
#  define TRACE_MSG(x)
#  define TRACE_GERROR(x)
#else

#  include <iostream>
#  include <iomanip>
#  include <fstream>
#  include <ctime>

#  include <boost/thread.hpp>
#  include <boost/thread/mutex.hpp>

extern boost::recursive_mutex g_log_mutex;
extern std::map<boost::thread::id, std::ofstream *> g_log_streams;

class Debug
{
public:
  static void init(const std::string &name = "");
  static void name(const std::string &name = "");
  static std::string trace_string();
  static std::ofstream &stream();
};

#  ifndef TRACE_EXTRA
#    define TRACE_EXTRA ""
#  endif

#  define TRACE_ENTER(x)                                                                 \
    g_log_mutex.lock();                                                                  \
    const char *_trace_method_name = x;                                                  \
    Debug::stream() << Debug::trace_string() << ">>> " << x << TRACE_EXTRA << std::endl; \
    g_log_mutex.unlock();

#  define TRACE_ENTER_MSG(x, y)                                                                      \
    g_log_mutex.lock();                                                                              \
    const char *_trace_method_name = x;                                                              \
    Debug::stream() << Debug::trace_string() << ">>> " << x << TRACE_EXTRA << " " << y << std::endl; \
    g_log_mutex.unlock();

#  define TRACE_RETURN(y)                                                                       \
    g_log_mutex.lock();                                                                         \
    Debug::stream() << Debug::trace_string() << "<<< " << _trace_method_name << y << std::endl; \
    g_log_mutex.unlock();

#  define TRACE_EXIT()                                                                     \
    g_log_mutex.lock();                                                                    \
    Debug::stream() << Debug::trace_string() << "<<< " << _trace_method_name << std::endl; \
    g_log_mutex.unlock();

#  define TRACE_MSG(msg)                                                                                 \
    g_log_mutex.lock();                                                                                  \
    Debug::stream() << Debug::trace_string() << "    " << _trace_method_name << " " << msg << std::endl; \
    g_log_mutex.unlock();

#  define TRACE_GERROR(err)                                                                                                 \
    g_log_mutex.lock();                                                                                                     \
    if (err != NULL)                                                                                                        \
      {                                                                                                                     \
        Debug::stream() << Debug::trace_string() << "    " << _trace_method_name << " error:" << err->message << std::endl; \
      }                                                                                                                     \
    g_log_mutex.unlock();

#  define TRACE_LOG(err)                                                    \
    g_log_mutex.lock();                                                     \
    Debug::stream() << Debug::trace_string() << "    " << err << std::endl; \
    g_log_mutex.unlock();

#endif // TRACING

#endif // WORKRAVE_UTILS_DEBUG_HH
