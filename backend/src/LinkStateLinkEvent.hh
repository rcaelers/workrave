// LinkStateLinkEvent.hh --- An event of the Workrave core
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

#ifndef LINKSTATELINKEVENT_HH
#define LINKSTATELINKEVENT_HH

#include <string>
#include <list>

#include "LinkEvent.hh"
#include "ICore.hh"

using namespace workrave;

//! Link Event
class LinkStateLinkEvent : public LinkEvent
{
public:
  enum LinkState
    {
      LINKSTATE_NONE,
      LINKSTATE_UP,
      LINKSTATE_DOWN,
    };

public:
  LinkStateLinkEvent();
  LinkStateLinkEvent(const UUID &id, LinkState state);
  virtual ~LinkStateLinkEvent();

  virtual std::string str() const;
  virtual std::string class_name() const;
  virtual void serialize(workrave::serialization::Target *s);

  const UUID &get_link_id() const
  {
    return link_id;
  }

  LinkState get_link_state() const
  {
    return link_state;
  }

private:
  /*! Link ID */
  UUID link_id;

  /*! LinkState Event */
  LinkState link_state;
};

#endif // LINKSTATELINKEVENT_HH
