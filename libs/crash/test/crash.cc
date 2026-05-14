// Copyright (C) 2023 Rob Caelers & Raymond Penners
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

#if defined(HAVE_CRASH_REPORT)
#  include "crash/CrashReporter.hh"
#endif

#include "debug.hh"
#include "utils/Paths.hh"

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
  const auto *const log_file = "crash.log";

  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file, 1024 * 1024, 5, true);

  auto logger{std::make_shared<spdlog::logger>("workrave", std::initializer_list<spdlog::sink_ptr>{file_sink, console_sink})};
  logger->flush_on(spdlog::level::critical);
  logger->set_level(spdlog::level::trace);
  spdlog::set_default_logger(logger);

  spdlog::set_level(spdlog::level::trace);
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%-5l%$] %v");
  spdlog::info("Workrave started");
  spdlog::info("Log file: {}", log_file);

  const auto *const trace_file = "crash-trace.log";
  auto trace_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(trace_file, 1024 * 1024, 10, true);
  auto tracer = std::make_shared<spdlog::logger>("trace", trace_sink);
  tracer->set_level(spdlog::level::trace);
  tracer->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] %v");
  spdlog::info("Trace file: {}", trace_file);

  ScopedTrace::init(tracer);
}

static void
crash2()
{
  int *p = nullptr;
  *p = 0;
  TRACE_ENTRY();
}

static void
crash1()
{
  TRACE_ENTRY();
  crash2();
}

static void
crash()
{
  TRACE_ENTRY();
  crash1();
}

int
run(int argc, char **argv)
{
  init_logging();
  TRACE_ENTRY();

  try
    {
      workrave::crash::CrashReporter::instance().init();
    }
  catch (std::exception &e)
    {
      TRACE_VAR(std::string("Crashhandler init exception:") + e.what());
    }
  crash();
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
  run(sizeof(argv) / sizeof(argv[0]), argv);
  return (0);
}

#endif
