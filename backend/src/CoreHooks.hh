// Core.hh --- The main controller
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

#ifndef COREHOOKS_HH
#define COREHOOKS_HH

#include "ICoreHooks.hh"
#include "ICoreTestHooks.hh"

class CoreHooks : public ICoreHooks, public ICoreTestHooks
{
public:
  typedef boost::shared_ptr<CoreHooks> Ptr;

  static Ptr create();

  CoreHooks();
  
  boost::function<ActivityState()> &hook_local_activity_state();
  boost::function<workrave::config::IConfigurator::Ptr()> &hook_create_configurator();
  
  boost::signals2::signal<void(bool)> &signal_local_active_changed();
  boost::signals2::signal<bool(), IsActiveCombiner> &hook_is_active();

private:

  
  boost::function<ActivityState()> local_activity_state_hook;
  boost::function<workrave::config::IConfigurator::Ptr()> create_configurator_hook;
  
  boost::signals2::signal<void(bool)> local_active_changed_signal;
  boost::signals2::signal<bool(), IsActiveCombiner> is_active_hook;
};

#endif // COREHOOKS_HH
