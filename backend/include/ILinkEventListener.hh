// ILinkListener.hh -- Listener interface for network events.
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

#ifndef ILINKEVENTLISTENER_HH
#define ILINKEVENTLISTENER_HH

namespace workrave
{
  // Forward declaratons
  class LinkEvent;

  //! Listener interface for network events.
  class ILinkEventListener
  {
  public:
    virtual ~ILinkEventListener() {}

    //! An event have been received from the Workrave network.
    virtual void event_received(LinkEvent *event) = 0;
  };
}

#endif // ILINKEVENTLISTENER_HH
