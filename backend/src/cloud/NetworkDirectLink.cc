// NetworkDirectLink.cc
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

#include "NetworkDirectLink.hh"

using namespace std;

NetworkDirectLink::Ptr
NetworkDirectLink::create()
{
  return NetworkDirectLink::Ptr(new NetworkDirectLink());
}


NetworkDirectLink::NetworkDirectLink()
{
  TRACE_ENTER("NetworkDirectLink::NetworkDirectLink");
  unicast_server = SocketServer::create();
  TRACE_EXIT();
}


NetworkDirectLink::~NetworkDirectLink()
{
  TRACE_ENTER("NetworkDirectLink::~NetworkDirectLink");
  TRACE_EXIT();
}


void
NetworkDirectLink::init(int port)
{
  TRACE_ENTER("NetworkDirectLink::init");
  unicast_server->init(port);
  unicast_server->signal_accepted().connect(sigc::mem_fun(*this, &NetworkDirectLink::on_accepted));
  TRACE_EXIT();
}


void
NetworkDirectLink::terminate()
{
  TRACE_ENTER("NetworkDirectLink::terminate");
  TRACE_EXIT();
}


void
NetworkDirectLink::connect(const string &host, int port)
{
  TRACE_ENTER_MSG("NetworkDirectLink::connect", host << " " << port);

  Socket::Ptr socket = Socket::create();
  socket->connect(host, port);

  ConnectionInfo::Ptr info(new ConnectionInfo());
  info->state = CONNECTION_STATE_CONNECTING;
  info->socket = socket;
  info->address = socket->get_remote_address();
  info->stream = boost::shared_ptr<ByteStream>(new ByteStream(1024));
  connections.push_back(info);

  socket->signal_io().connect(sigc::bind<0>(sigc::mem_fun(*this, &NetworkDirectLink::on_data), info));
  socket->signal_connected().connect(sigc::bind<0>(sigc::mem_fun(*this, &NetworkDirectLink::on_connected), info));
  socket->signal_disconnected().connect(sigc::bind<0>(sigc::mem_fun(*this, &NetworkDirectLink::on_disconnected), info));

  
  TRACE_EXIT();
}


void
NetworkDirectLink::send_message(const string &message)
{
  TRACE_ENTER("NetworkDirectLink::send_message");

  guint16 length = GINT16_TO_BE(message.length());

  // Send to remote workraves.
  for (ConnectionIter i = connections.begin(); i != connections.end(); i++)
    {
      boost::shared_ptr<ConnectionInfo> info = *i;

      info->socket->write((gchar *) (&length), 2);
      info->socket->write(message.c_str(), message.length());
    }
  TRACE_EXIT();
}


//!
void
NetworkDirectLink::on_accepted(Socket::Ptr socket)
{
  TRACE_ENTER("NetworkDirectLink::on_accepted");
  ConnectionInfo::Ptr info(new ConnectionInfo());
  info->state = CONNECTION_STATE_CONNECTED;
  info->socket = socket;
  info->address = socket->get_remote_address();
  info->stream = boost::shared_ptr<ByteStream>(new ByteStream(1024));
  connections.push_back(info);

  socket->signal_io().connect(sigc::bind<0>(sigc::mem_fun(*this, &NetworkDirectLink::on_data), info));
  socket->signal_connected().connect(sigc::bind<0>(sigc::mem_fun(*this, &NetworkDirectLink::on_connected), info));
  socket->signal_disconnected().connect(sigc::bind<0>(sigc::mem_fun(*this, &NetworkDirectLink::on_disconnected), info));
  TRACE_EXIT();
}


void
NetworkDirectLink::on_connected(ConnectionInfo::Ptr info)
{
  TRACE_ENTER("NetworkDirectLink::on_connected");
  info->state = CONNECTION_STATE_CONNECTED;
  info->address = info->socket->get_remote_address();
  TRACE_EXIT();
}


void
NetworkDirectLink::on_disconnected(ConnectionInfo::Ptr info)
{
  TRACE_ENTER("NetworkDirectLink::on_disconnected");
  info->state = CONNECTION_STATE_CLOSING;
  close(info);
  TRACE_EXIT();
}

//!
void
NetworkDirectLink::on_data(ConnectionInfo::Ptr info)
{
  TRACE_ENTER("NetworkDirectLink::on_data");
  
  bool error = false;
  
  try
    {
      const gsize header_size = sizeof(guint16);
      gsize bytes_read = 0;
      gsize bytes_to_read = 2;
      gsize packet_size = 0;

      TRACE_MSG(info->stream->get_read_buffer_size());
      
      if (info->stream->get_read_buffer_size() >= header_size)
        {
          packet_size = GINT16_FROM_BE(*((guint16 *)(info->stream->get_read_buffer())));
          
          bytes_to_read = packet_size - info->stream->get_read_buffer_size() + header_size;

          info->stream->resize(packet_size + header_size);
        }

      TRACE_MSG(bytes_to_read);
      
      if (bytes_to_read > 0)
        {
          info->socket->read(info->stream->get_write_buffer(), bytes_to_read, bytes_read);
          if (bytes_read == 0)
            {
              TRACE_MSG("socket closed");
              error = true;
            }
          else
            {
              g_assert(bytes_read > 0);
              info->stream->advance_write_buffer(bytes_read);
            }
          TRACE_MSG(bytes_read);
        }
      
      if (packet_size > 0 && packet_size + header_size == info->stream->get_read_buffer_size())
        {
          data_signal.emit(packet_size, info->stream->get_read_buffer() + header_size, info->address);
          info->stream->init(1024);
        }
    }
  catch(...)
    {
      TRACE_MSG("Exception");
      error = true;
    }

  if (error)
    {
      info->socket->close();
      close(info);
    }

  TRACE_EXIT();
}


void
NetworkDirectLink::close(ConnectionInfo::Ptr info)
{
  TRACE_ENTER("NetworkDirectLink::close");
  connections.remove(info);
  TRACE_EXIT()
}

sigc::signal<void, gsize, const gchar *, NetworkAddress::Ptr> &
NetworkDirectLink::signal_data()
{
  return data_signal;
}
