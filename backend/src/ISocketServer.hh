// ISocketServer.hh
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

#ifndef ISOCKETSERVER_HH
#define ISOCKETSERVER_HH

class ISocketServer;
class ISocket;

//! Asynchronous server socket callbacks.
class ISocketServerListener
{
public:
  ISocketServerListener() {}
  virtual ~ISocketServerListener() {}

  //! The specified server socket has accepted a new connection
  virtual void socket_accepted(ISocketServer *server, ISocket *con) = 0;
};


//! TCP Listen ISocket.
class ISocketServer
{
public:
  ISocketServer() :
    listener(NULL)
  {
  }

  virtual ~ISocketServer() {};

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

#endif // ISOCKETSERVER_HH
