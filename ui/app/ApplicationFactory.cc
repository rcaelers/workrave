// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#include "ApplicationFactory.hh"
#include "ui/IToolkitFactory.hh"

class Application;
class IToolkit;

#if defined(PLATFORM_OS_MACOS)
#  include "ApplicationMacOS.hh"
#elif defined(PLATFORM_OS_WINDOWS)
#  include "ApplicationWindows.hh"
#elif defined(PLATFORM_OS_UNIX)
#  include "ApplicationUnix.hh"
#endif

std::shared_ptr<Application>
ApplicationFactory::create(int argc, char **argv, std::shared_ptr<IToolkitFactory> toolkit_factory)
{
#if defined(PLATFORM_OS_MACOS)
  return std::make_shared<ApplicationMacOS>(argc, argv, toolkit_factory);
#elif defined(PLATFORM_OS_WINDOWS)
  return std::make_shared<ApplicationWindows>(argc, argv, toolkit_factory);
#elif defined(PLATFORM_OS_UNIX)
  return std::make_shared<ApplicationUnix>(argc, argv, toolkit_factory);
#else
#  error Unsupported platform
#endif
}
