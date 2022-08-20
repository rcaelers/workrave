// Copyright (C) 2014 Rob Caelers <robc@krandor.org>
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

#include "MouseMonitor.hh"

#include <iostream>
#include <memory>

#include <Appkit/NSView.h>
#include <Appkit/NSWindow.h>

class MouseMonitor::Private
{
public:
  id monitor{};
  std::function<void(int, int)> func;

public:
  Private() = default;
};

MouseMonitor::MouseMonitor(std::function<void(int, int)> func)
{
  priv = std::make_unique<Private>();
  priv->func = func;
}

MouseMonitor::~MouseMonitor()
{
  stop();
}

void
MouseMonitor::start()
{
  if (priv->monitor == nil)
    {
      priv->monitor = [NSEvent addGlobalMonitorForEventsMatchingMask:NSEventMaskMouseMoved
                                                             handler:^(NSEvent *) {
                                                               NSPoint pos = [NSEvent mouseLocation];
                                                               priv->func(pos.x, pos.y);
                                                               ;
                                                             }];
    }
}

void
MouseMonitor::stop()
{
  if (priv->monitor != nil)
    {
      [NSEvent removeMonitor:priv->monitor];
      priv->monitor = nil;
    }
}
