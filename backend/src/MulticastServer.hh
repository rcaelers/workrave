// MulticastServer.hh
//
// Copyright (C) 2002, 2003, 2005, 2007, 2010, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef MULTICASTSERVER_HH
#define MULTICASTSERVER_HH

#include <sigc++/sigc++.h>

//! Multicast Server.
class IMulticastServer
{
public:
  virtual ~IMulticastServer() {};

  //! Initializes multicast server
  virtual void init() = 0;

  //! Multicast data.
  virtual void send(const gchar *buf, gsize count) = 0;

  //!
  virtual sigc::signal<void, int, void *> &signal_multicast_data() = 0;
};

class MulticastServerBase : public IMulticastServer
{
public:
  virtual ~MulticastServerBase() {}
  
  virtual sigc::signal<void, int, void *> &signal_multicast_data()
  {
    return multicast_data_signal;
  }

protected:
  sigc::signal<void, int, void *> multicast_data_signal;
};

#endif // MULTICASTSERVER_HH
