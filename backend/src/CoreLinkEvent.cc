// CoreLinkEvent.cc
//
// Copyright (C) 2007, 2008 Rob Caelers
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
#include "CoreLinkEvent.hh"
#include "Serializer.hh"

using namespace std;

// Register event
REGISTER_TYPE(LinkEvent, CoreLinkEvent, "corelinkevent");


//! Create event
CoreLinkEvent::CoreLinkEvent()
  : core_event(CORE_EVENT_NONE)
{
}


//! Create event
CoreLinkEvent::CoreLinkEvent(CoreEvent event)
  : core_event(event)
{
}


//! Destruct event
CoreLinkEvent::~CoreLinkEvent()
{
}


//! Return the core event
CoreLinkEvent::CoreEvent
CoreLinkEvent::get_core_event() const
{
  return core_event;
}


//! Return a string representation of the event
string
CoreLinkEvent::str() const
{
  stringstream ss;

  ss << "[CoreLinkEvent "
     << LinkEvent::str() << " "
     << " event: " << core_event << " ]";

  return ss.str();
}


//! Define serialization
void
CoreLinkEvent::serialize(workrave::serialization::Target *s)
{
  LinkEvent::serialize(s);

  (*s) & workrave::serialization::attr("core_event", (int &)core_event);
}
