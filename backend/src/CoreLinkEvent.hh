// CoreLinkEvent.hh
//
// Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.nl>
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

#ifndef CORELINKEVENT_HH
#define CORELINKEVENT_HH

#include <string>

#include "LinkEvent.hh"
#include "ICore.hh"

using namespace workrave;

//! Event indication action to be performed on a core.
class CoreLinkEvent : public LinkEvent
{
public:
  //! Core event
  enum CoreEvent
    {
      //! No event
      CORE_EVENT_NONE,

      //! Operation mode change to suspended
      CORE_EVENT_MODE_SUSPENDED,

      //! Operation mode change to quiet
      CORE_EVENT_MODE_QUIET,

      //! Operation mode change to normal
      CORE_EVENT_MODE_NORMAL,
    };

public:
  CoreLinkEvent();
  CoreLinkEvent(CoreEvent event);
  virtual ~CoreLinkEvent();

  // ISerializable
  virtual std::string str() const;
  virtual std::string class_name() const;
  virtual void serialize(workrave::serialization::Target *s);

  // Event specific functions
  CoreEvent get_core_event() const;

private:
  //! Core Event
  CoreEvent core_event;
};

#endif // CORELINKEVENT_HH
