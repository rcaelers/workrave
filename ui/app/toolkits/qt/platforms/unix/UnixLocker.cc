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

#include <spdlog/spdlog.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "UnixLocker.hh"
#include "utils/Platform.hh"

using namespace workrave::utils;

bool
UnixLocker::can_lock()
{
  return !Platform::running_on_wayland();
}

void
UnixLocker::prepare_lock()
{
}

void
UnixLocker::lock()
{
  spdlog::info("UnixLocker::lock()");
  if (!Platform::running_on_wayland())
    {
    }
}

void
UnixLocker::unlock()
{
  if (!Platform::running_on_wayland())
    {
    }
}
