// LinkFactory.hh
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

#ifndef LINKFACTORY_HH
#define LINKFACTORY_HH

#include <string>

// Forward declarion of internal interfaces.
class ILinkServer;
class ILinkServerListener;


//! Factory that creates link servers.
class LinkFactory
{
public:
  //! Creates a link server of the specified type.
  static ILinkServer *create_link_server(std::string type, ILinkServerListener *link_listener);
};

#endif // LINKFACTORY_HH
