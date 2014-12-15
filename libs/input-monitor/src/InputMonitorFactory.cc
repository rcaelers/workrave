// InputMonitorFactory.cc
//
// Copyright (C) 2003, 2004, 2005, 2007, 2012, 2013 Rob Caelers <robc@krandor.org>
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
#include "config.h"
#endif

#include "input-monitor/InputMonitorFactory.hh"

#ifdef PLATFORM_OS_WIN32
#include "W32InputMonitorFactory.hh"
#endif
#ifdef PLATFORM_OS_OSX
#include "OSXInputMonitorFactory.hh"
#endif
#ifdef PLATFORM_OS_UNIX
#include "UnixInputMonitorFactory.hh"
#endif

#include "nls.h"

using namespace workrave::config;

IInputMonitorFactory *InputMonitorFactory::factory = NULL;

void
InputMonitorFactory::init(IConfigurator::Ptr config, const std::string &display)
{
  if (factory == NULL)
    {
#if defined(PLATFORM_OS_WIN32)
      factory = new W32InputMonitorFactory(config);
#elif defined(PLATFORM_OS_OSX)
      factory = new OSXInputMonitorFactory(config);
#elif defined(PLATFORM_OS_UNIX)
      factory = new UnixInputMonitorFactory(config);
#endif
    }

  if (factory != NULL)
    {
      factory->init(display);
    }
}

IInputMonitor::Ptr
InputMonitorFactory::create_monitor(IInputMonitorFactory::MonitorCapability capability)
{
  if (factory != NULL)
    {
      return factory->create_monitor(capability);
    }

  return IInputMonitor::Ptr();
}
