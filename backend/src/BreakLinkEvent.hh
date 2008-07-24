// BreakLinkEvent.hh
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

#ifndef BREAKLINKEVENT_HH
#define BREAKLINKEVENT_HH

#include <string>

#include "LinkEvent.hh"
#include "ICore.hh"

using namespace workrave;

//! Event indication action to be performed on a break.
class BreakLinkEvent : public LinkEvent
{
public:
  //! Break event
  enum BreakEvent
    {
      //! No event
      BREAK_EVENT_NONE,

      //! The user choose to postpone a break
      BREAK_EVENT_USER_POSTPONE,

      //! The user choose to skip a break
      BREAK_EVENT_USER_SKIP,

      //! The user chooses to start the break manually
      BREAK_EVENT_USER_FORCE_BREAK,

      //! The system force a break upon the user
      BREAK_EVENT_SYST_FORCE_BREAK,

      //! The prelude windows was removed prematurely
      BREAK_EVENT_SYST_STOP_PRELUDE
    };

public:
  BreakLinkEvent();
  BreakLinkEvent(BreakEvent event);
  BreakLinkEvent(BreakId id, BreakEvent event);
  virtual ~BreakLinkEvent();

  // ISerializable
  virtual std::string str() const;
  virtual std::string class_name() const;
  virtual void serialize(workrave::serialization::Target *s);

  // Event specific functions
  BreakId get_break_id() const;
  BreakEvent get_break_event() const;

private:
  //! Break ID
  BreakId break_id;

  //! Break Event
  BreakEvent break_event;
};

#endif // BREAKLINKEVENT_HH
