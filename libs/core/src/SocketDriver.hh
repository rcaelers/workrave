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

#ifndef SOCKETDRIVER_HH
#define SOCKETDRIVER_HH

class ISocket;
class ISocketServer;

#include "utils/Exception.hh"

using namespace workrave;

//! Asynchronous socket callbacks.
class ISocketListener
{
public:
  ISocketListener() = default;
  virtual ~ISocketListener() = default;

  //! The specified socket is now connected.
  virtual void socket_connected(ISocket *con, void *data) = 0;

  //! The specified socket has data ready to be read.
  virtual void socket_io(ISocket *con, void *data) = 0;

  //! The specified socket closed its connection.
  virtual void socket_closed(ISocket *con, void *data) = 0;
};

//! Asynchronous server socket callbacks.
class ISocketServerListener
{
public:
  ISocketServerListener() = default;
  virtual ~ISocketServerListener() = default;

  //! The specified server socket has accepted a new connection
  virtual void socket_accepted(ISocketServer *server, ISocket *con) = 0;
};

//! TCP Socket.
class ISocket
{
public:
  ISocket() = default;
  virtual ~ISocket() = default;

  //! Create a connection to the specified host and port.
  virtual void connect(const std::string &hostname, int port) = 0;

  //! Read data from the connection.
  virtual void read(void *buf, int count, int &bytes_read) = 0;

  //! Write data to the connection
  virtual void write(void *buf, int count, int &bytes_written) = 0;

  //! Close the connection.
  virtual void close() = 0;

  //! Set the callback listener for asynchronous socket events.
  void set_listener(ISocketListener *l);

  //! Set user data
  void set_data(void *data);

protected:
  //! Listener that received notifications of socket events.
  ISocketListener *listener{nullptr};

  // User data for callback.
  void *user_data{nullptr};
};

//! TCP Listen ISocket.
class ISocketServer
{
public:
  ISocketServer()
    : listener(nullptr)
  {
  }

  virtual ~ISocketServer() = default;

  //! Listen at the specified port.
  /*! \pre set_listener called
   */
  virtual void listen(int port) = 0;

  //! Sets the callback listener for asynchronous events.
  void set_listener(ISocketServerListener *l);

protected:
  //! Listener that receives notification of accepted connections.
  ISocketServerListener *listener;
};

//! TCP Socket abstraction.
class SocketDriver
{
public:
  static SocketDriver *create();

  virtual ~SocketDriver() = default;

  //! Create a new socket
  virtual ISocket *create_socket() = 0;

  //! Create a new listen socket
  virtual ISocketServer *create_server() = 0;
};

//! Socket exception
class SocketException : public workrave::utils::Exception
{
public:
  explicit SocketException(const std::string &detail)
    : Exception(detail)
  {
  }

  ~SocketException() throw() override = default;
};

#include "SocketDriver.icc"

#endif // SOCKETDRIVER_HH
