// Copyright (C) 2001 - 2021 Rob Caelers <robc@krandor.nl>
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

#include "ToolkitFactory.hh"

#if defined(PLATFORM_OS_MACOS)
#  include "ToolkitMacOS.hh"
#elif defined(PLATFORM_OS_WINDOWS)
#  include "ToolkitWindows.hh"
#elif defined(PLATFORM_OS_UNIX)
#  include "ToolkitUnix.hh"
#endif

std::shared_ptr<IToolkit>
ToolkitFactory::create(int argc, char **argv)
{
#if defined(PLATFORM_OS_MACOS)
  return std::make_shared<ToolkitMacOS>(argc, argv);
#elif defined(PLATFORM_OS_WINDOWS)
  return std::make_shared<ToolkitWindows>(argc, argv);
#elif defined(PLATFORM_OS_UNIX)
  return std::make_shared<ToolkitUnix>(argc, argv);
#else
#  error Unsupported platform
#endif
}
