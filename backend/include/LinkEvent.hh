// LinkEvent.hh --- Interface definition for a Workrave link event
//
// Copyright (C) 2007, 2008, 2009 Rob Caelers <robc@krandor.nl>
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

#ifndef LINKEVENT_HH
#define LINKEVENT_HH

#include <string>
#include <list>

#include "UUID.hh"
#include "ISerializable.hh"


#include "Trackable.hh"

namespace workrave
{
  //! Event send across the Workrave network.
  class LinkEvent : public workrave::serialization::ISerializable
  {
  public:
    LinkEvent();
    virtual ~LinkEvent();

    //! Set the ID of the Workrave that sent this event.
    void set_source(const UUID &id);

    //! Return the ID of the Workrave that sent this event.
    UUID get_source() const;

    //! Set the priority of this event
    void set_priority(int prio);

    //! Return the priority of this event
    int get_priority() const;

    //! Return the type of this event
    std::string get_eventid() const;

    // ISerializable
    virtual std::string str() const;
    virtual std::string class_name() const;
    virtual void serialize(workrave::serialization::Target *s);

  private:
    //! ID of the Workrave that send the message
    UUID source;

    //! Event priority
    int priority;
  };
}

#endif // LINKEVENT_HH
