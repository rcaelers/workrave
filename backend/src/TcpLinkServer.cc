// TcpLinkServer.cc
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
//

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <assert.h>

#include "TcpLinkServer.hh"
#include "ILinkServerListener.hh"
#include "GNetSocketDriver.hh"
#include "TcpLink.hh"


//! Constructs a new Gnet socket driver.
TcpLinkServer::TcpLinkServer(ILinkServerListener *link_listener)
  : port(27272)
{
  this->link_listener = link_listener;
}


//! Constructs a new Gnet socket driver.
TcpLinkServer::TcpLinkServer(int port, ILinkServerListener *link_listener)
  : port(port)
{
  this->link_listener = link_listener;
}

//! Destructs the socket driver.
TcpLinkServer::~TcpLinkServer()
{
  delete socket;
}


//! Initializes the driver.
bool
TcpLinkServer::init()
{
  TRACE_ENTER("TcpLinkServer::init");
  bool ret = true;

  try
    {
      socket = SocketDriver::create_server();
      socket->set_listener(this);
      socket->listen(port);
    }
  catch(SocketException &e)
    {
      TRACE_MSG("SocketException: " << e.details());
      ret = false;
    }

  TRACE_EXIT();
  return ret;
}

void
TcpLinkServer::terminate()
{
}

//!
void
TcpLinkServer::socket_accepted(ISocketServer *server, ISocket *con)
{
  TRACE_ENTER("TcpLinkServer::socket_accepted");

  (void) server;

  if (con != NULL)
    {
      TcpLink *link = new TcpLink(con);
      link_listener->new_link(link);
      link->init();
    }
  TRACE_EXIT();
}
