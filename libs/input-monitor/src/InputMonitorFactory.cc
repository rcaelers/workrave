// Copyright (C) 2003 - 2013 Rob Caelers <robc@krandor.org>
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

#include "input-monitor/InputMonitorFactory.hh"

#if defined(PLATFORM_OS_WINDOWS)
#  include "W32InputMonitorFactory.hh"
#endif
#if defined(PLATFORM_OS_MACOS)
#  include "MacOSInputMonitorFactory.hh"
#endif
#if defined(PLATFORM_OS_UNIX)
#  include "UnixInputMonitorFactory.hh"
#endif

using namespace workrave::config;
using namespace workrave::input_monitor;

workrave::input_monitor::IInputMonitorFactory *workrave::input_monitor::InputMonitorFactory::factory = nullptr;

void
InputMonitorFactory::init(IConfigurator::Ptr config, const char *display)
{
  if (factory == nullptr)
    {
#if defined(PLATFORM_OS_WINDOWS)
      factory = new W32InputMonitorFactory(config);
#elif defined(PLATFORM_OS_MACOS)
      factory = new MacOSInputMonitorFactory(config);
#elif defined(PLATFORM_OS_UNIX)
      factory = new UnixInputMonitorFactory(config);
#endif
    }

  if (factory != nullptr)
    {
      factory->init(display);
    }
}

IInputMonitor::Ptr
InputMonitorFactory::create_monitor(MonitorCapability capability)
{
  if (factory != nullptr)
    {
      return factory->create_monitor(capability);
    }

  return IInputMonitor::Ptr();
}
