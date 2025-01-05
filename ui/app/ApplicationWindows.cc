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

#include <memory>

#include "ApplicationWindows.hh"

#include "ui/windows/WindowsForceFocus.hh"
#include "ui/windows/WindowsCompat.hh"

ApplicationWindows::ApplicationWindows(int argc, char **argv, std::shared_ptr<IToolkitFactory> toolkit_factory)
  : Application(argc, argv, toolkit_factory)
{
}

ApplicationWindows::~ApplicationWindows()
{
  TRACE_ENTRY();
}
void
ApplicationWindows::init_platform_pre()
{
  WindowsForceFocus::init(get_configurator());
  WindowsCompat::init(get_configurator());
}

void
ApplicationWindows::init_platform_post()
{
}
