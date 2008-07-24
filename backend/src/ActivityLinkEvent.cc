// ActivityLinkEvent.cc
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
#include "ActivityLinkEvent.hh"
#include "Serializer.hh"

using namespace std;

// Register event
REGISTER_TYPE(LinkEvent, ActivityLinkEvent, "activitylinkevent");


//! Create new event
ActivityLinkEvent::ActivityLinkEvent()
{
}


//! Create new event
ActivityLinkEvent::ActivityLinkEvent(ActivityState state)
  : state(state)
{
}


//! Destruct event
ActivityLinkEvent::~ActivityLinkEvent()
{
}


//! Return the activity state of the remote user
ActivityState
ActivityLinkEvent::get_state() const
{
  return state;
}


//! Return a string representation of the event
string
ActivityLinkEvent::str() const
{
  stringstream ss;

  ss << "[ActivityLinkEvent "
     << LinkEvent::str() << " "
     << " state: " << state << " ]";

  return ss.str();
}


//! Define serialization
void
ActivityLinkEvent::serialize(workrave::serialization::Target *s)
{
  LinkEvent::serialize(s);

  (*s) & workrave::serialization::attr("state", (int &)state);
}
