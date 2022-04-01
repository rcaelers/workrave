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

#if defined(HAVE_CRASH_REPORT)
#  include "crash/CrashReporter.hh"
#endif

//#include "Toolkit.hh"
#include "ToolkitFactory.hh"
#include "ApplicationFactory.hh"
#include "Application.hh"

extern "C" int run(int argc, char **argv);

using namespace workrave::utils;

int
run(int argc, char **argv)
{
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
