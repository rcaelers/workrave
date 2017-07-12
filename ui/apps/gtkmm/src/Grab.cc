// Copyright (C) 2001 - 2008, 2011, 2013 Rob Caelers & Raymond Penners
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
#include "config.h"
#endif

#include "Grab.hh"

#if defined(PLATFORM_OS_UNIX)
#include "unix/UnixGrab.hh"
#elif defined(PLATFORM_OS_WIN32)
#include "win32/W32Grab.hh"
#if defined(HAVE_HARPOON)
#include "win32/W32GrabHarpoon.hh"
#endif
#endif

Grab *
Grab::instance()
{
  static Grab *instance = nullptr;

  if (instance == nullptr)
    {
#if defined(PLATFORM_OS_UNIX)
      instance = new UnixGrab();
#elif defined(PLATFORM_OS_WIN32)
#if defined(HAVE_HARPOON)
      instance = new W32GrabHarpoon();
#else
      instance = new W32Grab();
#endif
#endif
    }
  return instance;
}
