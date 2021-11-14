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
#include "input-monitor/IInputMonitor.hh"

#include "config/IConfigurator.hh"

using namespace workrave::input_monitor;
using namespace workrave::config;

class InputMonitorStub : public IInputMonitor
{
public:
  ~InputMonitorStub() override = default;

  bool init() override
  {
    return true;
  }

  void terminate() override
  {
  }

  void subscribe(IInputMonitorListener *listener) override
  {
    (void)listener;
  }

  void unsubscribe(IInputMonitorListener *listener) override
  {
    (void)listener;
  }
};

void
InputMonitorFactory::init(IConfigurator::Ptr config, const char *display)
{
  (void)config;
  (void)display;
}

IInputMonitor::Ptr
InputMonitorFactory::create_monitor(MonitorCapability capability)
{
  (void)capability;
  return IInputMonitor::Ptr(new InputMonitorStub());
}
