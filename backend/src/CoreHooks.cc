// CoreHooks.cc
//
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
#include "config.h"
#endif

#include "debug.hh"

#include "CoreHooks.hh"

CoreHooks::Ptr
CoreHooks::create()
{
  return Ptr(new CoreHooks());
}


//! Constructs a new CoreHooks.
CoreHooks::CoreHooks()
{
  TRACE_ENTER("CoreHooks::CoreHooks");
  TRACE_EXIT();
}


//! 
CoreHooks::~CoreHooks()
{
}

#ifdef HAVE_TESTS
boost::function<workrave::config::IConfigurator::Ptr()> &
CoreHooks::hook_create_configurator()
{
  return create_configurator_hook;
}

boost::function<IActivityMonitor::Ptr()> &
CoreHooks::hook_create_monitor()
{
  return create_monitor_hook;
}

boost::function<bool(Timer::Ptr timers[workrave::BREAK_ID_SIZEOF])> &
CoreHooks::hook_load_timer_state()
{
  return load_timer_state_hook;
}

#endif
