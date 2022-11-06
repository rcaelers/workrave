// Copyright (C) 2001 - 2021 Rob Caelers & Raymond Penners
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

#include "debug.hh"

#include <cstdio>
#include <fstream>

#if defined(PLATFORM_OS_WINDOWS)
#  include <io.h>
#  include <fcntl.h>
#  include "utils/W32ActiveSetup.hh"
#endif

#include "debug.hh"
#include "utils/Platform.hh"
#include "utils/Paths.hh"

#if defined(HAVE_CRASH_REPORT)
#  include "crash/CrashReporter.hh"
#endif

#include "ToolkitFactory.hh"
#include "ApplicationFactory.hh"
#include "Application.hh"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#if SPDLOG_VERSION >= 10600
#  include <spdlog/pattern_formatter.h>
#endif
#if SPDLOG_VERSION >= 10801
#  include <spdlog/cfg/env.h>
#endif

extern "C" int run(int argc, char **argv);

using namespace workrave::utils;

static void
init_logging()
{
  const auto log_dir = Paths::get_log_directory();
  std::filesystem::create_directories(log_dir);

  const auto log_file = log_dir / "workrave.log";

  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file.string(), 1024 * 1024, 5, true);

  auto logger{std::make_shared<spdlog::logger>("workrave", std::initializer_list<spdlog::sink_ptr>{file_sink, console_sink})};
  logger->flush_on(spdlog::level::critical);
  spdlog::set_default_logger(logger);

  spdlog::set_level(spdlog::level::info);
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%-5l%$] %v");
  spdlog::info("Workrave started");
  spdlog::info("Log file: {}", log_file.string());

#if SPDLOG_VERSION >= 10801
  spdlog::cfg::load_env_levels();
#endif
#if defined(HAVE_TRACING)
  const auto trace_file = log_dir / "workrave-trace.log";
  auto trace_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(trace_file.string(), 1024 * 1024, 10, true);
  auto tracer = std::make_shared<spdlog::logger>("trace", trace_sink);
  tracer->set_level(spdlog::level::trace);
  tracer->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] %v");
  spdlog::info("Trace file: {}", trace_file.string());

  ScopedTrace::init(tracer);
#endif
}

int
run(int argc, char **argv)
{
  init_logging();
  TRACE_ENTRY();

#if defined(HAVE_CRASH_REPORT)
  try
    {
      bool no_crashpad = std::getenv("WORKRAVE_NO_CRASHPAD") != nullptr;
      if (!no_crashpad)
        {
          TRACE_MSG("Starting crashhandler");
          workrave::crash::CrashReporter::instance().init();
        }
    }
  catch (std::exception &e)
    {
      TRACE_VAR(std::string("Crashhandler init exception:") + e.what());
    }
#endif
#if defined(PLATFORM_OS_WINDOWS)
  W32ActiveSetup::update_all();
#endif

  auto app = ApplicationFactory::create(argc, argv, std::make_shared<ToolkitFactory>());
  app->main();

  return 0;
}

#if !defined(PLATFORM_OS_WINDOWS) || (!defined(PLATFORM_OS_WINDOWS_NATIVE) && !defined(NDEBUG))
int
main(int argc, char **argv)
{
  int ret = run(argc, argv);
  return ret;
}

#else

#  include <windows.h>

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
  (void)hInstance;
  (void)hPrevInstance;
  (void)iCmdShow;

  char *argv[] = {szCmdLine};

  // InnoSetup: [...] requires that you add code to your application
  // which creates a mutex with the name you specify in this
  // directive.
  HANDLE mtx = CreateMutexA(NULL, FALSE, "WorkraveMutex");
  if (mtx != NULL && GetLastError() != ERROR_ALREADY_EXISTS)
    {
      run(sizeof(argv) / sizeof(argv[0]), argv);
    }
  return (0);
}

#endif
