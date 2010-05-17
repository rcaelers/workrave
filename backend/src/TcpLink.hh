// TcpLink.hh --- Networking link server
//
// Copyright (C) 2007, 2009, 2010 Rob Caelers <robc@krandor.nl>
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

#ifndef TCPLINK_HH
#define TCPLINK_HH

#include "ILink.hh"
#include "SocketDriver.hh"
#include "ByteStream.hh"

// Forward declarion of external interface.
namespace workrave {
  class LinkEvent;
}

class ILinkListener;
class ISocket;

//*
class TcpLink :
  public ILink,
  public ISocketListener
{
public:
  TcpLink(ISocket *con = NULL);
  virtual ~TcpLink();

  // Public interface
  bool connect(const std::string &host, int port);
  void init();

  // ILink
  const WRID &get_link_id() const;
  void set_link_listener(ILinkListener *listener);
  void send_event(LinkEvent *event);

private:
  // ISocketListener
  virtual void socket_connected(ISocket *con, void *data);
  virtual void socket_io(ISocket *con, void *data);
  virtual void socket_closed(ISocket *con, void *data);

  // Internal
  void process_packet();
  void process_event();
  void process_auth(const WRID &uuid);
  void send_auth();

  enum PacketType {
    PACKET_EVENT = 1,
    PACKET_AUTH
  };

private:
  //!
  SocketDriver *socket_driver;

  //! The server socket.
  ISocket *socket;

  //!
  ILinkListener *link_listener;

  //!
  WRID link_id;

  //!
  ByteStream byte_stream;

  //!
  bool authenticated;
};


#include "TcpLink.icc"

#endif // TCPLINK_HH
