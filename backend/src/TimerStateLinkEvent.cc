// TimerStateLinkEvent.cc
//
// Copyright (C) 2007 Rob Caelers
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

static const char rcsid[] = "$Id";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <sstream>

#include "Factory.hh"
#include "TimerStateLinkEvent.hh"
#include "Serializer.hh"

using namespace std;

REGISTER_TYPE(LinkEvent, TimerStateLinkEvent, "timerstatelinkevent");

TimerStateLinkEvent::TimerStateLinkEvent()
{
}


TimerStateLinkEvent::TimerStateLinkEvent(const std::vector<int> &idle_times,
                                         const std::vector<int> &active_times)
  : idle_times(idle_times), active_times(active_times)
{
}

TimerStateLinkEvent::~TimerStateLinkEvent()
{
}


const std::vector<int> &
TimerStateLinkEvent::get_idle_times() const
{
  return idle_times;
}

const std::vector<int> &
TimerStateLinkEvent::get_active_times() const
{
  return active_times;
}

string
TimerStateLinkEvent::str() const
{
  stringstream ss;

  ss << "[TimerStateLinkEvent "
     << LinkEvent::str() << " "
     << " idle[mp] " << idle_times[BREAK_ID_MICRO_BREAK] << " "
     << " idle[rb] " << idle_times[BREAK_ID_REST_BREAK] << " "
     << " idle[dl] " << idle_times[BREAK_ID_DAILY_LIMIT] << " "
     << " active[mp] " << active_times[BREAK_ID_MICRO_BREAK] << " "
     << " active[rb] " << active_times[BREAK_ID_REST_BREAK] << " "
     << " active[dl] " << active_times[BREAK_ID_DAILY_LIMIT] << " "
     << " ]";

  return ss.str();
}


void
TimerStateLinkEvent::serialize(workrave::serialization::Target *s)
{
  LinkEvent::serialize(s);

  (*s) & workrave::serialization::attr("idle", idle_times)
      & workrave::serialization::attr("active", active_times);
}
