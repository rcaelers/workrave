// ICoreInternal.hh --- The main controller interface
//
// Copyright (C) 2001 - 2009, 2011, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef ICOREINTERNAL_HH
#define ICOREINTERNAL_HH

#include <string>

#include "ICore.hh"

#include "Timer.hh"
#include "Break.hh"
#include "IActivityMonitor.hh"
#include "Statistics.hh"

class Statistics;

class ICoreInternal : public workrave::ICore
{
public:
  virtual ~ICoreInternal() {}
  
  //! Return the current time
  virtual time_t get_time() const = 0;

  virtual ActivityState get_current_monitor_state() const = 0;
  virtual IActivityMonitor *get_activity_monitor() const = 0;
  virtual Timer *get_timer(workrave::BreakId id) const = 0;
  virtual Timer *get_timer(std::string name) const = 0;
  virtual Break *get_break(workrave::BreakId id) = 0;
  virtual void defrost() = 0;
  virtual void freeze() = 0;
  virtual void force_break_idle(workrave::BreakId id) = 0;
  virtual Statistics *get_statistics() const = 0;
  virtual void stop_prelude(BreakId break_id) = 0;
  virtual void post_event(CoreEvent event) = 0;
};

#endif // ICOREINTERNAL_HH
