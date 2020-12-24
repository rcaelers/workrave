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
#ifdef HAVE_HARPOON
  Harpoon::unblock_input();
#endif
  std::cout << "crashpad_handler\n";

  return false;
}

void
CrashReporter::Pimpl::init()
{
  const std::filesystem::path temp_dir = std::filesystem::temp_directory_path() / "workrave-crashpad";
  std::cout << "temp dir: " << temp_dir << "\n";

  const std::filesystem::path app_dir = workrave::utils::Platform::get_application_directory();
  std::cout << "application path: " << app_dir << "\n";

  std::string handler_exe;

#ifdef PLATFORM_OS_WINDOWS
  handler_exe = "crashpad_handler.exe";
#else
  handler_exe = "crashpad_handler";
#endif

  base::FilePath handler(app_dir / "bin" / handler_exe);
  const std::string url("http://192.168.7.185:8080/");

  std::map<std::string, std::string> annotations;
  std::vector<std::string> arguments;

  base::FilePath reports_dir(temp_dir);
  base::FilePath metrics_dir(temp_dir);
  std::unique_ptr<crashpad::CrashReportDatabase> database = crashpad::CrashReportDatabase::Initialize(reports_dir);

  if (database == NULL)
    {
      std::cout << "no database\n";
      return;
    }

  crashpad::Settings *settings = database->GetSettings();
  if (settings == NULL)
    {
      std::cout << "no settings\n";
      return;
    }

  settings->SetUploadsEnabled(true);

  client = new crashpad::CrashpadClient();
  bool success = client->StartHandler(handler, reports_dir, metrics_dir, url, annotations, arguments, /* restartable */ true, /* asynchronous_start */ false);
  std::cout << "result = " << success << "\n";

  crashpad::CrashpadClient::SetFirstChanceExceptionHandler(&CrashReporter::Pimpl::crashpad_handler);
}
