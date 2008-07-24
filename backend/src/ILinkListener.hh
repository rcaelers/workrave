// ILinkListener.hh  --- Interface definition for a Workrave link server
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

#ifndef ILINKLISTENER_HH
#define ILINKLISTENER_HH

// Forward declarion of external interface.
namespace workrave {
  class LinkEvent;
  class UUID;
}
using namespace workrave;


//! Listener for events from a Workrave Link.
class ILinkListener
{
public:
  virtual ~ILinkListener() {}

  //! Received incoming event from Link
  virtual void event_received(const UUID &id, LinkEvent *event) = 0;

  //! The specified link is down.
  virtual void link_down(const UUID &id) = 0;

  //! The specified link is now up and running.
  virtual void link_up(const UUID &id) = 0;
};

#endif // ILINKLISTENER_HH
