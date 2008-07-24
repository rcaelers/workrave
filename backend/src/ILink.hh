// ILink.hh --- Interface definition for a Workrave link
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
// $Id: IInputMonitor.hh 1090 2006-10-01 20:49:47Z dotsphinx $
//

#ifndef ILINK_HH
#define ILINK_HH

// Forward declarion of external interfaces.
namespace workrave {
  class LinkEvent;
  class UUID;
}


// Forward declarion of internal interfaces.
class ILinkListener;


//! Peer-to-Peer communication link workrave clients.
class ILink
{
public:
  virtual ~ILink() {}

  //! Returns the unique link id
  virtual const workrave::UUID &get_link_id() const = 0;

  //! Sets the link listener
  virtual void set_link_listener(ILinkListener *listener) = 0;

  //! Sends an event to the link
  virtual void send_event(workrave::LinkEvent *event) = 0;
};

#endif // ILINK_HH
