// BreakLinkEvent.cc
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
#include "BreakLinkEvent.hh"
#include "Serializer.hh"

using namespace std;

// Register event
REGISTER_TYPE(LinkEvent, BreakLinkEvent, "breaklinkevent");


//! Create event
BreakLinkEvent::BreakLinkEvent()
  : break_event(BREAK_EVENT_NONE)
{
}


//! Create event
BreakLinkEvent::BreakLinkEvent(BreakEvent event)
  : break_id(BREAK_ID_NONE), break_event(event)
{
}


//! Create event
BreakLinkEvent::BreakLinkEvent(BreakId id, BreakEvent event)
  : break_id(id), break_event(event)
{
}


//! Destruct event
BreakLinkEvent::~BreakLinkEvent()
{
}


//! Return break id of this event
BreakId
BreakLinkEvent::get_break_id() const
{
  return break_id;
}


//! Return the break event
BreakLinkEvent::BreakEvent
BreakLinkEvent::get_break_event() const
{
  return break_event;
}


//! Return a string representation of the event
string
BreakLinkEvent::str() const
{
  stringstream ss;

  ss << "[BreakLinkEvent "
     << LinkEvent::str() << " "
     << " event: " << break_event << " "
     << " break: " << break_id << " ]";

  return ss.str();
}


//! Define serialization
void
BreakLinkEvent::serialize(workrave::serialization::Target *s)
{
  LinkEvent::serialize(s);

  (*s) & workrave::serialization::attr("break_event", (int &)break_event)
       & workrave::serialization::attr("break_id", (int &)break_id);
}
