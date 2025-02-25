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

#ifndef ICORETESTHOOKS_HH
#define ICORETESTHOOKS_HH

#include <string>
#include <istream>
#include <memory>

#include <boost/signals2.hpp>

#include "config/IConfigurator.hh"
#include "IActivityMonitor.hh"
#include "core/CoreTypes.hh"
#include "Timer.hh"

class ICoreTestHooks
{
public:
  virtual ~ICoreTestHooks() = default;

  using Ptr = std::shared_ptr<ICoreTestHooks>;

  virtual std::function<IActivityMonitor::Ptr()> &hook_create_monitor() = 0;
  virtual std::function<bool(Timer *timers[workrave::BREAK_ID_SIZEOF])> &hook_load_timer_state() = 0;
};

#endif // ICOREHOOKS_HH
