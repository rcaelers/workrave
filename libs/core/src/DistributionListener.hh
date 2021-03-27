// DistributionListener.hh
//
// Copyright (C) 2002, 2003, 2005 Rob Caelers <robc@krandor.org>
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

#ifndef DISTRIBUTIONLISTENER_HH
#define DISTRIBUTIONLISTENER_HH

class DistributionListener
{
public:
  virtual ~DistributionListener() = default;

  //! A remote client has signed on.
  virtual void signon_remote_client(std::string client_id) = 0;

  //! A remote client has signed off.
  virtual void signoff_remote_client(std::string client_id) = 0;
};

#endif // DISTRIBUTIONLISTENER_HH
