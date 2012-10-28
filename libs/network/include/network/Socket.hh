// Socket.hh
//
// Copyright (C) 2002 - 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_NETWORK_SOCKET_HH
#define WORKRAVE_NETWORK_SOCKET_HH

#include <string>

#include <glib.h>

#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#include "NetworkAddress.hh"

namespace workrave
{
  namespace network
  {
    //! Socket.
    class Socket
    {
    public:
      typedef boost::shared_ptr<Socket> Ptr;
      typedef boost::signals2::signal<void()> io_signal_type;
      typedef boost::signals2::signal<void()> connection_state_changed_signal_type;
  
    public:
      static Ptr create();

      virtual ~Socket() {};

      //! Creates a connection to the specified address.
      virtual void connect(const std::string &host_name, int port, bool tls) = 0;

      //!
      virtual bool is_connected() const = 0;
      
      //! Reads data from the connection.
      virtual bool read(gchar *data, gsize count, gsize &bytes_read) = 0;

      //! Writes data to the connection
      virtual bool write(const gchar *data, gsize count) = 0;

      //! Closes the connection.
      virtual void close() = 0;

      virtual NetworkAddress::Ptr get_remote_address() = 0;

      virtual io_signal_type &signal_io() = 0;
      virtual connection_state_changed_signal_type &signal_connection_state_changed() = 0;
    };
  }
}

#endif // WORKRAVE_NETWORK_SOCKET_HH
