// ILinkListener.hh --- Interface definition for a Workrave link listener
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

#ifndef ILINKSERVERLISTENER_HH
#define ILINKSERVERLISTENER_HH

// Forward declarion of internal interfaces.
class ILink;


//! Listener for events from a Workrave Link Server.
class ILinkServerListener
{
public:
  virtual ~ILinkServerListener() {}

  //! A new link was created.
  virtual void new_link(ILink *link) = 0;
};

#endif // ILINKSERVERLISTENER_HH
