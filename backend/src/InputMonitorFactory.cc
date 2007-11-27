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

static const char rcsid[] = "$Id: IdleLogManager.cc 1356 2007-10-22 18:22:13Z rcaelers $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "InputMonitorFactory.hh"

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

IInputMonitorFactory *InputMonitorFactory::factory = NULL;

void
InputMonitorFactory::init(const std::string &display)
{
  if (factory == NULL)
    {
#if defined(PLATFORM_OS_WIN32)
      factory = new W32InputMonitorFactory();
#elif defined(PLATFORM_OS_OSX)
      factory = new OSXInputMonitorFactory();
#elif defined(PLATFORM_OS_UNIX)
      factory = new UnixInputMonitorFactory();
#endif
    }

  if (factory != NULL)
    {
      factory->init(display);
    }
}

IInputMonitor *
InputMonitorFactory::get_monitor(IInputMonitorFactory::MonitorCapability capability)
{
  if (factory != NULL)
    {
      return factory->get_monitor(capability);
    }

  return NULL;
}

