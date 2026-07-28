// Copyright (C) 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#include "debug.hh"

#include "CoreHooks.hh"

#if defined(HAVE_TESTS)
#  if defined(HAVE_DBUS) && !defined(HAVE_RPC_DBUS)
std::function<std::shared_ptr<workrave::dbus::IDBus>()> &
CoreHooks::hook_create_dbus()
{
  return create_dbus_hook;
}
#  endif

std::function<IActivityMonitor::Ptr()> &
CoreHooks::hook_create_monitor()
{
  return create_monitor_hook;
}

std::function<bool(Timer::Ptr timers[workrave::BREAK_ID_SIZEOF])> &
CoreHooks::hook_load_timer_state()
{
  return load_timer_state_hook;
}

#endif
