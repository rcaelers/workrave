// TcpLinkServer.hh --- Networking link server
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

#ifndef TCPLINKSERVER_HH
#define TCPLINKSERVER_HH

#include "SocketDriver.hh"
#include "ILinkServer.hh"

class ISocket;
class ILinkServerListener;

//*
class TcpLinkServer :
  public ILinkServer,
  public ISocketServerListener
{
public:
  TcpLinkServer(ILinkServerListener *link_listener);
  TcpLinkServer(int port, ILinkServerListener *link_listener);
  ~TcpLinkServer();

  bool init();
  void terminate();

private:
  virtual void socket_accepted(ISocketServer *server, ISocket *con);

private:
  //!
  SocketDriver *socket_driver;

  //! The server socket.
  ISocketServer *socket;

  //!
  int port;

  //!
  ILinkServerListener *link_listener;
};

#endif // TCPLINKSERVER_HH
