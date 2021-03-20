// InputMonitorFactory.cc
//
// Copyright (C) 2003, 2004, 2005, 2007 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "InputMonitorFactory.hh"

#ifdef PLATFORM_OS_WINDOWS
#  include "W32InputMonitorFactory.hh"
#endif
#ifdef PLATFORM_OS_MACOS
#  include "MacOSInputMonitorFactory.hh"
#endif
#ifdef PLATFORM_OS_UNIX
#  include "UnixInputMonitorFactory.hh"
#endif

#include "nls.h"

IInputMonitorFactory *InputMonitorFactory::factory = nullptr;

void
InputMonitorFactory::init(const char *display)
{
  if (factory == nullptr)
    {
#if defined(PLATFORM_OS_WINDOWS)
      factory = new W32InputMonitorFactory();
#elif defined(PLATFORM_OS_MACOS)
      factory = new MacOSInputMonitorFactory();
#elif defined(PLATFORM_OS_UNIX)
      factory = new UnixInputMonitorFactory();
#endif
    }

  if (factory != nullptr)
    {
      factory->init(display);
    }
}

IInputMonitor *
InputMonitorFactory::get_monitor(IInputMonitorFactory::MonitorCapability capability)
{
  if (factory != nullptr)
    {
      return factory->get_monitor(capability);
    }

  return nullptr;
}
