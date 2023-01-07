// Copyright (C) 2020 Rob Caelers <robc@krandor.nl>
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

#include <string>
#include <list>
#include <mutex>
#include <exception>

#include "crash/CrashReporter.hh"

#include <iostream>
#include <filesystem>
#include <spdlog/spdlog.h>

#include "base/logging.h"
#include "client/settings.h"
#include "client/crashpad_client.h"
#include "client/crash_report_database.h"

#include "debug.hh"
#include "utils/Paths.hh"

#if defined(HAVE_HARPOON)
#  include "input-monitor/Harpoon.hh"
#endif

using namespace workrave::crash;

class CrashReporter::Pimpl
{
public:
  Pimpl() = default;

  void init();
  static bool crashpad_handler(EXCEPTION_POINTERS *info);

  void register_crash_handler(CrashHandler *handler);
  void unregister_crash_handler(CrashHandler *handler);

private:
  void call_crash_handlers();

private:
  crashpad::CrashpadClient *client{nullptr};
  std::mutex mutex;
  std::list<CrashHandler *> handlers;
};

CrashReporter &
CrashReporter::instance()
{
  static auto *crash_reporter = new CrashReporter();
  return *crash_reporter;
}

CrashReporter::CrashReporter()
{
  pimpl = std::make_unique<Pimpl>();
}

void
CrashReporter::init()
{
  pimpl->init();
}

void
CrashReporter::register_crash_handler(CrashHandler *handler)
{
  pimpl->register_crash_handler(handler);
}

void
CrashReporter::unregister_crash_handler(CrashHandler *handler)
{
  pimpl->unregister_crash_handler(handler);
}

bool
CrashReporter::Pimpl::crashpad_handler(EXCEPTION_POINTERS *info)
{
  TRACE_ENTRY();
#if defined(HAVE_HARPOON)
  Harpoon::unblock_input();
#endif
  CrashReporter::instance().pimpl->call_crash_handlers();
  spdlog::critical("Crash!");
  spdlog::shutdown();

  return false;
}

void
CrashReporter::Pimpl::init()
{
  TRACE_ENTRY();
  try
    {
      logging::SetLogMessageHandler(
        [](logging::LogSeverity severity, const char *file_path, int line, size_t message_start, const std::string &string) {
          spdlog::warn("Crashpad: {}", string);
          return false;
        });

      const std::filesystem::path temp_dir = std::filesystem::temp_directory_path() / "workrave-crashpad";
      const std::filesystem::path app_dir = workrave::utils::Paths::get_application_directory();
      const std::filesystem::path log_dir = workrave::utils::Paths::get_log_directory();

#if defined(PLATFORM_OS_WINDOWS)
      std::string handler_exe = "WorkraveCrashHandler.exe";
#else
      std::string handler_exe = "WorkraveCrashHandler";
#endif

      base::FilePath handler(app_dir / "lib" / handler_exe);
      const std::string url("http://192.168.7.241:8888/api/minidump/upload?api_key=94a8033818104a4396d92178bb33ec0a");

      std::map<std::string, std::string> annotations;
      std::vector<std::string> arguments;
      std::vector<base::FilePath> attachments;

      annotations["product"] = "Workrave";
      annotations["version"] = WORKRAVE_VERSION;
#if defined(WORKRAVE_GIT_VERSION)
      annotations["commit"] = WORKRAVE_GIT_VERSION;
#endif
#if defined(WORKRAVE_BUILD_ID)
      annotations["buildid"] = WORKRAVE_BUILD_ID;
#endif

      TRACE_MSG("handler = {}", app_dir);

      attachments.emplace_back(log_dir / "workrave.log");
      attachments.emplace_back(log_dir / "workrave.1.log");
      attachments.emplace_back(log_dir / "workrave-trace.log");
      attachments.emplace_back(log_dir / "workrave-trace.1.log");

      base::FilePath reports_dir(temp_dir);
      base::FilePath metrics_dir(temp_dir);
      std::unique_ptr<crashpad::CrashReportDatabase> database = crashpad::CrashReportDatabase::Initialize(reports_dir);

      if (database == nullptr)
        {
          throw std::runtime_error("failed to initialize crashplan database");
        }

      crashpad::Settings *settings = database->GetSettings();
      if (settings == nullptr)
        {
          throw std::runtime_error("failed to obtain crashplan settings");
        }

      settings->SetUploadsEnabled(true);

      arguments.emplace_back("--no-rate-limit");

      client = new crashpad::CrashpadClient();
      bool success = client->StartHandler(handler,
                                          reports_dir,
                                          metrics_dir,
                                          url,
                                          annotations,
                                          arguments,
                                          /* restartable */ true,
                                          /* asynchronous_start */ false,
                                          attachments);
      if (success)
        {
          crashpad::CrashpadClient::SetFirstChanceExceptionHandler(&CrashReporter::Pimpl::crashpad_handler);
        }
      else
        {
          throw std::runtime_error("failed to start crashplan handler");
        }
    }
  catch (std::exception &e)
    {
      spdlog::warn(std::string("failed to start crash handler:") + e.what());
    }
}

void
CrashReporter::Pimpl::register_crash_handler(CrashHandler *handler)
{
  std::scoped_lock lock(mutex);
  handlers.push_back(handler);
}

void
CrashReporter::Pimpl::unregister_crash_handler(CrashHandler *handler)
{
  std::scoped_lock lock(mutex);
  handlers.remove(handler);
}

void
CrashReporter::Pimpl::call_crash_handlers()
{
  std::scoped_lock lock(mutex);
  for (auto &h: handlers)
    {
      h->on_crashed();
    }
}
