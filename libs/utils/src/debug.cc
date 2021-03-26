// debug.cc
//
// Copyright (C) 2001, 2002, 2003, 2007, 2009, 2011, 2012, 2013 Rob Caelers <robc@krandor.org>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef TRACING

#  include <sstream>
#  include <chrono>

#  include <thread>
#  include <boost/filesystem.hpp>

#  ifdef PLATFORM_OS_WINDOWS
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h> /* for GetFileAttributes */
#  endif

#  include "debug.hh"
#  include "utils/TimeSource.hh"

using namespace std;
using namespace workrave::utils;

std::recursive_mutex g_log_mutex;
std::map<std::thread::id, std::ofstream *> g_log_streams;

static thread_local std::string g_thread_name;
static std::string g_prefix;

std::string
Debug::trace_string()
{
  char logtime[256];

#  ifdef HAVE_TESTS
  auto t = static_cast<time_t>(TimeSource::get_real_time_sec());
  struct tm *tmlt = localtime(&t);
  strftime(logtime, 256, "%d%b%Y %H:%M:%S", tmlt);

  stringstream ss;
  ss << logtime;

  if (TimeSource::source)
    {
      auto tt = std::chrono::system_clock::now();
      auto ms = std::chrono::duration_cast<std::chrono::microseconds>(tt.time_since_epoch()).count();

      t = std::chrono::system_clock::to_time_t(tt);

      tmlt = localtime(&t);

      strftime(logtime, 256, "%H:%M:%S", tmlt);
      ss << " " << logtime << "." << (ms % 1000000) / 1000;
    }

  ss << " " << std::this_thread::get_id() /* g_thread_self() */ << " ";
  return ss.str();

#  else

  auto t = (time_t)TimeSource::get_real_time_sec();
  struct tm *tmlt = localtime(&t);
  strftime(logtime, 256, "%d%b%Y %H:%M:%S", tmlt);

  stringstream ss;
  ss << logtime << " " << std::this_thread::get_id() /* g_thread_self() */ << " ";
  return ss.str();
#  endif
}

void
Debug::init(const std::string &name)
{
  g_prefix = name;
}

void
Debug::name(const std::string &name)
{
  g_log_mutex.lock();
  if (g_log_streams.find(std::this_thread::get_id()) != g_log_streams.end())
    {
      g_log_streams[std::this_thread::get_id()]->close();
      delete g_log_streams[std::this_thread::get_id()];
      g_log_streams.erase(std::this_thread::get_id());
    }

  g_thread_name = g_prefix + name;
  g_log_mutex.unlock();
}

std::ofstream &
Debug::stream()
{
  g_log_mutex.lock();
  if (g_log_streams.find(std::this_thread::get_id()) == g_log_streams.end())
    {
      std::string debug_filename;

#  if defined(WIN32) || defined(PLATFORM_OS_WINDOWS)
      char path_buffer[MAX_PATH];

      DWORD ret = GetTempPath(MAX_PATH, path_buffer);
      if (ret > MAX_PATH || ret == 0)
        {
          debug_filename = "C:\\temp\\";
        }
      else
        {
          debug_filename = path_buffer;
        }

      boost::filesystem::path dir(debug_filename);
      boost::filesystem::create_directory(dir);

#  elif defined(PLATFORM_OS_MACOS)
      debug_filename = "/tmp/";
#  elif defined(PLATFORM_OS_UNIX)
      debug_filename = "/tmp/";
#  else
#    error Unknown platform.
#  endif

      char logfile[128];
      time_t ltime;

      time(&ltime);
      struct tm *tmlt = localtime(&ltime);
      strftime(logfile, 128, "workrave-%d%b%Y-%H%M%S", tmlt);

      stringstream ss;
      ss << debug_filename << logfile << "-" << std::this_thread::get_id();

      if (g_thread_name != "")
        {
          ss << "-" << g_thread_name;
        }

      g_log_streams[std::this_thread::get_id()] = new std::ofstream();
      g_log_streams[std::this_thread::get_id()]->open(ss.str().c_str(), std::ios::app);
    }

  std::ofstream *ret = g_log_streams[std::this_thread::get_id()];
  g_log_mutex.unlock();
  return *ret;
}

#endif
