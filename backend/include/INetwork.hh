// INetwork.hh -- Interface to the networking facility
//
// Copyright (C) 2012 Rob Caelers <robc@krandor.nl>
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
  //! Interface to the networking facility of Workrave.
  class INetwork
  {
  public:
    virtual ~INetwork() {};
  };
}

#endif // INETWORK_HH
