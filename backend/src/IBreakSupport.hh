// IBreakSupport.hh --- The main controller interface
//
// Copyright (C) 2001 - 2009, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef IBREAKSUPPORT_HH
#define IBREAKSUPPORT_HH

#include <string>

#include "CoreTypes.hh"
#include "IActivityMonitor.hh"
#include "Timer.hh"

class IBreakSupport
{
public:
  typedef boost::shared_ptr<IBreakSupport> Ptr;

  virtual ~IBreakSupport() {}
  
  virtual void defrost() = 0;
  virtual void freeze() = 0;
  virtual void resume_reading_mode_timers() = 0;
  virtual void set_insensitive_mode(InsensitiveMode mode) = 0;
  virtual IActivityMonitor::Ptr create_timer_activity_monitor(const std::string &break_name) = 0;
};

#endif // IBREAKSUPPORT_HH
