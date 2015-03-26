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
#include "input-monitor/IInputMonitor.hh"

#include "config/IConfigurator.hh"

using namespace workrave::input_monitor;
using namespace workrave::config;

class InputMonitorStub : public IInputMonitor
{
public:
  virtual ~InputMonitorStub() {}

  virtual bool init()
  {
    return true;
  }

  virtual void terminate()
  {
  }

  virtual void subscribe(IInputMonitorListener *listener)
  {
    (void) listener;
  }

  virtual void unsubscribe(IInputMonitorListener *listener)
  {
    (void) listener;
  }
};

void
InputMonitorFactory::init(IConfigurator::Ptr config, const std::string &display)
{
  (void) config;
  (void) display;
}

IInputMonitor::Ptr
InputMonitorFactory::create_monitor(IInputMonitorFactory::MonitorCapability capability)
{
  (void)capability;
  return IInputMonitor::Ptr(new InputMonitorStub());
}

