// TimerStateLinkEvent.hh --- An event of the Workrave core
//
// Copyright (C) 2007 Rob Caelers <robc@krandor.nl>
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
// $Id$
//

#ifndef TIMERSTATELINKEVENT_HH
#define TIMERSTATELINKEVENT_HH

#include <vector>

#include "LinkEvent.hh"
#include "ICore.hh"

using namespace workrave;

//! Link Event
class TimerStateLinkEvent : public LinkEvent
{
private:
  
public:
  TimerStateLinkEvent();
  TimerStateLinkEvent(const std::vector<int> &idle_times,
                      const std::vector<int> &active_times);
  virtual ~TimerStateLinkEvent();

  virtual std::string str() const;
  virtual std::string class_name() const;
  virtual void serialize(workrave::serialization::Target *s);

  const std::vector<int> &get_idle_times() const;
  const std::vector<int> &get_active_times() const;
  
private:
  /*! Current idle time of all timers*/
  std::vector<int> idle_times;

  /*! Current active time of all timers*/
  std::vector<int> active_times;
};

#endif // TIMERSTATELINKEVENT_HH
