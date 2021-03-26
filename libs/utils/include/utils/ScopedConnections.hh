// Copyright (C) 2014 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UTILS_SCOPED_CONNECTIONS_HH
#define WORKRAVE_UTILS_SCOPED_CONNECTIONS_HH

#include "boost/noncopyable.hpp"
#include "boost/signals2.hpp"

class scoped_connections : public boost::noncopyable
{
public:
  ~scoped_connections() { disconnect_all(); }

  void add(boost::signals2::connection conn) { connections.push_front(conn); }

  template<typename S>
  void connect(S &sig, const typename S::slot_function_type &sf)
  {
    add(sig.connect(sf));
  }

  void disconnect_all()
  {
    for (boost::signals2::connection c: connections)
      {
        c.disconnect();
      }
    connections.clear();
  }

private:
  std::list<boost::signals2::connection> connections;
};

#endif
