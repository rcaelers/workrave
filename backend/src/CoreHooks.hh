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

#ifndef COREHOOKS_HH
#define COREHOOKS_HH

#include "ICoreHooks.hh"
#include "ICoreTestHooks.hh"

class CoreHooks : public ICoreHooks
#ifdef HAVE_TESTS
                , public ICoreTestHooks
#endif
{
public:
  typedef boost::shared_ptr<CoreHooks> Ptr;

  static Ptr create();

  CoreHooks();
  ~CoreHooks() override;

#ifdef HAVE_TESTS
  boost::function<workrave::config::IConfigurator::Ptr()> &hook_create_configurator() override;
  boost::function<IActivityMonitor::Ptr()> &hook_create_monitor() override;
  boost::function<bool(Timer::Ptr[workrave::BREAK_ID_SIZEOF])> &hook_load_timer_state() override;
#endif

private:
#ifdef HAVE_TESTS
  boost::function<workrave::config::IConfigurator::Ptr()> create_configurator_hook;
  boost::function<IActivityMonitor::Ptr()> create_monitor_hook;
  boost::function<bool(Timer::Ptr[workrave::BREAK_ID_SIZEOF])> load_timer_state_hook;
#endif
};

#endif // COREHOOKS_HH
