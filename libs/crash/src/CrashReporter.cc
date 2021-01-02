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

#include "crash/CrashReporter.hh"

#include <iostream>
#include <filesystem>

#include "client/settings.h"
#include "client/crashpad_client.h"
#include "client/crash_report_database.h"

#ifdef PLATFORM_OS_WINDOWS
#  include <windows.h>
#endif

#include "debug.hh"
#include "utils/Platform.hh"
#ifdef HAVE_HARPOON
#  include "input-monitor/Harpoon.hh"
#endif

using namespace workrave::crash;

class CrashReporter::Pimpl
{
public:
  Pimpl() = default;

  void init();
  static bool crashpad_handler(EXCEPTION_POINTERS *);

  crashpad::CrashpadClient *client;
};

CrashReporter::CrashReporter()
{
  pimpl = std::make_unique<Pimpl>();
}

void
CrashReporter::init()
{
  pimpl->init();
}

bool
CrashReporter::Pimpl::crashpad_handler(EXCEPTION_POINTERS *info)
{
  TRACE_ENTER("CrashReporter::Pimpl::crashpad_handler");
#ifdef HAVE_HARPOON
  Harpoon::unblock_input();
#endif
  std::cout << "crashpad_handler\n";

  TRACE_EXIT();
  return false;
}

void
CrashReporter::Pimpl::init()
{
  const std::filesystem::path temp_dir = std::filesystem::temp_directory_path() / "workrave-crashpad";
  const std::filesystem::path app_dir = workrave::utils::Platform::get_application_directory();

#ifdef PLATFORM_OS_WINDOWS
  std::string handler_exe = "WorkraveCrashHandler.exe";
#else
  std::string handler_exe = "WorkraveCrashHandler";
#endif

  base::FilePath handler(app_dir / "bin" / handler_exe);
  const std::string url("http://192.168.7.185:8080/");

  std::map<std::string, std::string> annotations;
  std::vector<std::string> arguments;

  annotations["product"] = "Workrave";
  annotations["version"] = PACKAGE_VERSION;
#ifdef WORKRAVE_GIT_VERSION
  annotations["commit"] = WORKRAVE_GIT_VERSION;
#endif
#ifdef WORKRAVE_BUILD_ID
  annotations["buildid"] = WORKRAVE_BUILD_ID;
#endif

  base::FilePath reports_dir(temp_dir);
  base::FilePath metrics_dir(temp_dir);
  std::unique_ptr<crashpad::CrashReportDatabase> database = crashpad::CrashReportDatabase::Initialize(reports_dir);

  if (database == NULL)
    {
      throw std::runtime_error("failed to initialize crashplan database");
    }

  crashpad::Settings *settings = database->GetSettings();
  if (settings == NULL)
    {
      throw std::runtime_error("failed to obtain crashplan settings");
    }

  settings->SetUploadsEnabled(true);

  client = new crashpad::CrashpadClient();
  bool success = client->StartHandler(handler, reports_dir, metrics_dir, url, annotations, arguments, /* restartable */ true, /* asynchronous_start */ false);
  std::cout << "result = " << success << "\n";

  crashpad::CrashpadClient::SetFirstChanceExceptionHandler(&CrashReporter::Pimpl::crashpad_handler);
}
