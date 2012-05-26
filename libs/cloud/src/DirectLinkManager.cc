// DirectLinkManager.cc
//
// Copyright (C) 2007, 2008, 2009, 2012 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <boost/shared_ptr.hpp>

#include "debug.hh"

#include "DirectLinkManager.hh"

using namespace std;

DirectLinkManager::Ptr
DirectLinkManager::create(Marshaller::Ptr marshaller)
{
  return DirectLinkManager::Ptr(new DirectLinkManager(marshaller));
}


DirectLinkManager::DirectLinkManager(Marshaller::Ptr marshaller) : marshaller(marshaller)
{
  TRACE_ENTER("DirectLinkManager::DirectLinkManager");
  unicast_server = SocketServer::create();
  TRACE_EXIT();
}


DirectLinkManager::~DirectLinkManager()
{
  TRACE_ENTER("DirectLinkManager::~DirectLinkManager");
  TRACE_EXIT();
}


void
DirectLinkManager::init(int port)
{
  TRACE_ENTER("DirectLinkManager::init");
  unicast_server->init(port);
  unicast_server->signal_accepted().connect(boost::bind(&DirectLinkManager::on_accepted, this, _1));
  TRACE_EXIT();
}


void
DirectLinkManager::terminate()
{
  TRACE_ENTER("DirectLinkManager::terminate");
  TRACE_EXIT();
}


//!
void
DirectLinkManager::on_accepted(Socket::Ptr socket)
{
  TRACE_ENTER("DirectLinkManager::on_accepted");
  DirectLink::Ptr info = DirectLink::create(marshaller, socket);
  new_link_signal(info);
  TRACE_EXIT();
}


boost::signals2::signal<void(DirectLink::Ptr)> &
DirectLinkManager::signal_new_link()
{
  return new_link_signal;
}
