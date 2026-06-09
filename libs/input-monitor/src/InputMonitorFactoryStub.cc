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
#include "input-monitor/InputMonitorFactoryStub.hh"
#include "input-monitor/IInputMonitor.hh"
#include "input-monitor/IInputMonitorListener.hh"

#include "config/IConfigurator.hh"

#include <algorithm>
#include <vector>

using namespace workrave::input_monitor;
using namespace workrave::config;

namespace
{
  std::vector<IInputMonitorListener *> listeners;
}

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
    listeners.push_back(listener);
  }

  void unsubscribe(IInputMonitorListener *listener) override
  {
    std::erase(listeners, listener);
  }
};

void
workrave::input_monitor::test::fire_mouse(int x, int y, int wheel)
{
  for (auto *listener: listeners)
    {
      listener->mouse_notify(x, y, wheel);
    }
}

void
workrave::input_monitor::test::fire_button(bool is_press)
{
  for (auto *listener: listeners)
    {
      listener->button_notify(is_press);
    }
}

void
workrave::input_monitor::test::fire_keyboard(bool repeat)
{
  for (auto *listener: listeners)
    {
      listener->keyboard_notify(repeat);
    }
}

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
