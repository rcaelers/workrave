// LinkEvent.cc
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
#include "LinkEvent.hh"
#include "Serializer.hh"

using namespace workrave;
using namespace std;

// Register event
REGISTER_ABSTRACT_TYPE(workrave::LinkEvent, "linkevent");


//! Create a new event
LinkEvent::LinkEvent()
  : priority(10)
{
}


//! Destruct event
LinkEvent::~LinkEvent()
{
}


//! Set the ID of the Workrave that sent this event.
void
LinkEvent::set_source(const UUID &source)
{
  this->source = source;
}


//! Return the ID of the Workrave that sent this event.
UUID
LinkEvent::get_source() const
{
  return source;
}


//! Set the priority of this event
void
LinkEvent::set_priority(int priority)
{
  this->priority = priority;
}


//! Return the priority of this event
int
LinkEvent::get_priority() const
{
  return priority;
}


//! Return the type of this event
string
LinkEvent::get_eventid() const
{
  return class_name();
}


//! Return a string representation of the event
string
LinkEvent::str() const
{
  stringstream ss;

  ss << "[LinkEvent"
     << " id:" << get_eventid()
     << " src: " << source.str()
     << " prio: " << priority
     << " ]";

  return ss.str();
}


//! Define serialization
void
LinkEvent::serialize(workrave::serialization::Target *s)
{
  (*s) & workrave::serialization::attr("event_source", source)
       & workrave::serialization::attr("event_prio", priority);
}
