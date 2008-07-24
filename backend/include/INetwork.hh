// INetwork.hh -- Interface to the networking facility
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
// $Id: INetwork.hh 1090 2006-10-01 20:49:47Z dotsphinx $
//

#ifndef INETWORK_HH
#define INETWORK_HH

namespace workrave
{
  // Forward declarion of external interfaces.
  class LinkEvent;
  class ILinkEventListener;
  class UUID;

  //! Interface to the networking facility of Workrave.
  class INetwork
  {
  public:
    virtual ~INetwork() {};

    //
    // Setup
    //

    //! Connects to a remote Workrave
    virtual void connect(std::string url) = 0;

    //! Leave the network
    virtual void leave() = 0;

    //
    // Distributed events.
    //

    //! Send an event across the workrave network.
    virtual bool send_event(LinkEvent *event) = 0;

    //! Send an event through a single link.
    virtual bool send_event_to_link(const UUID &link_id, LinkEvent *event) = 0;

    //! Subscribe to a specific network event.
    virtual bool subscribe(const std::string &eventid, ILinkEventListener *listener) = 0;

    //! Unsubscribe from a specific network event.
    virtual bool unsubscribe(const std::string &eventid, ILinkEventListener *listener) = 0;

    //
    // Distributed configuration
    //

    //! Register a configuration key that MUST the the same on all Workrave clients.
    virtual void monitor_config(const std::string &key) = 0;

    //! Resolve a configuration setting with conflicting value
    virtual void resolve_config(const std::string &key, const std::string &typed_value) = 0;
  };
}

#endif // INETWORK_HH
