// CoreHooks.cc
//
// Copyright (C) 2012 Rob Caelers <robc@krandor.nl>
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


boost::function<ActivityState()> &
CoreHooks::hook_local_activity_state()
{
  return local_activity_state_hook;
}


boost::function<workrave::config::IConfigurator::Ptr()> &
CoreHooks::hook_create_configurator()
{
  return create_configurator_hook;
}


boost::signals2::signal<void(bool)> &
CoreHooks::signal_local_active_changed()
{
  return local_active_changed_signal;
}


boost::signals2::signal<bool(), CoreHooks::IsActiveCombiner> &
CoreHooks::hook_is_active()
{
  return is_active_hook;
}

