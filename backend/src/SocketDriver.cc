// SocketDriver.cc
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
//

static const char rcsid[] = "$Id: GNetSocketDriver.cc 1184 2007-05-12 09:16:31Z rcaelers $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SocketDriver.hh"
#include "GIOSocketDriver.hh"

//! Create a new socket
ISocket *
SocketDriver::create_socket()
{
  return new GIOSocket();
}


//! Create a new listen socket
ISocketServer *
SocketDriver::create_server()
{
  return new GIOSocketServer();
}
