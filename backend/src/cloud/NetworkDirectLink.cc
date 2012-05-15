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

NetworkDirectLink::NetworkDirectLinkClient::Ptr
NetworkDirectLink::NetworkDirectLinkClient::create(NetworkClient::Scope scope)
{
  return NetworkDirectLink::NetworkDirectLinkClient::Ptr(new NetworkDirectLink::NetworkDirectLinkClient(scope));
}


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

  NetworkDirectLinkClient::Ptr info = create_info_for_socket(socket, NetworkClient::CONNECTION_STATE_CONNECTING);
  
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
      boost::shared_ptr<NetworkDirectLinkClient> info = *i;

      TRACE_MSG("send");
      info->socket->write((gchar *) (&length), 2);
      info->socket->write(message.data(), message.length());
    }
  TRACE_EXIT();
}


// TODO: merge with send_message
void
NetworkDirectLink::send_message_to(const std::string &message, NetworkClient::Ptr client)
{
  TRACE_ENTER("NetworkDirectLink::send_message_to");

  guint16 length = GINT16_TO_BE(message.length());

  // Send to remote workraves.
  for (ConnectionIter i = connections.begin(); i != connections.end(); i++)
    {
      boost::shared_ptr<NetworkDirectLinkClient> info = *i;

      if (info == client)
        {
          TRACE_MSG("send");
          info->socket->write((gchar *) (&length), 2);
          info->socket->write(message.data(), message.length());
        }
    }
  TRACE_EXIT();
}


// TODO: merge with send_message
void
NetworkDirectLink::send_message_except(const std::string &message, NetworkClient::Ptr client)
{
  TRACE_ENTER("NetworkDirectLink::send_message_except");

  guint16 length = GINT16_TO_BE(message.length());

  // Send to remote workraves.
  for (ConnectionIter i = connections.begin(); i != connections.end(); i++)
    {
      boost::shared_ptr<NetworkDirectLinkClient> info = *i;

      if (info != client)
        {
          TRACE_MSG("send");
          info->socket->write((gchar *) (&length), 2);
          info->socket->write(message.data(), message.length());
        }
    }
  TRACE_EXIT();
}


//!
void
NetworkDirectLink::on_accepted(Socket::Ptr socket)
{
  TRACE_ENTER("NetworkDirectLink::on_accepted");
  NetworkDirectLinkClient::Ptr info = create_info_for_socket(socket, NetworkClient::CONNECTION_STATE_CONNECTED);

  socket->signal_io().connect(sigc::bind<0>(sigc::mem_fun(*this, &NetworkDirectLink::on_data), info));
  socket->signal_connected().connect(sigc::bind<0>(sigc::mem_fun(*this, &NetworkDirectLink::on_connected), info));
  socket->signal_disconnected().connect(sigc::bind<0>(sigc::mem_fun(*this, &NetworkDirectLink::on_disconnected), info));
  TRACE_EXIT();
}


void
NetworkDirectLink::on_connected(NetworkDirectLinkClient::Ptr info)
{
  TRACE_ENTER("NetworkDirectLink::on_connected");
  info->address = info->socket->get_remote_address();
  info->state = NetworkClient::CONNECTION_STATE_CONNECTED;
  client_update_signal.emit(info);
  TRACE_EXIT();
}


void
NetworkDirectLink::on_disconnected(NetworkDirectLinkClient::Ptr info)
{
  TRACE_ENTER("NetworkDirectLink::on_disconnected");
  close(info);
  TRACE_EXIT();
}


//!
void
NetworkDirectLink::on_data(NetworkDirectLinkClient::Ptr info)
{
  TRACE_ENTER("NetworkDirectLink::on_data");
  
  bool error = false;
  
  try
    {
      const gsize header_size = sizeof(guint16);
      gsize bytes_read = 0;
      gsize bytes_to_read = 2;
      guint16 packet_size = 0;

      TRACE_MSG(info->stream->get_position());
      
      if (info->stream->get_position() >= header_size)
        {
          info->stream->read_u16(packet_size, 0);
          
          bytes_to_read = header_size + packet_size - info->stream->get_position();

          info->stream->resize(header_size + packet_size);
        }

      TRACE_MSG(bytes_to_read);
      
      if (bytes_to_read > 0)
        {
          info->socket->read(info->stream->get_ptr(), bytes_to_read, bytes_read);
          if (bytes_read == 0)
            {
              TRACE_MSG("socket closed");
              error = true;
            }
          else
            {
              g_assert(bytes_read > 0);
              info->stream->advance_buffer(bytes_read);
            }
          TRACE_MSG(bytes_read);
        }
      
      if (packet_size > 0 && header_size + packet_size == info->stream->get_position())
        {
          data_signal.emit(packet_size, info->stream->get_start() + header_size, info);
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
      close(info);
    }

  TRACE_EXIT();
}


NetworkDirectLink::NetworkDirectLinkClient::Ptr
NetworkDirectLink::create_info_for_socket(Socket::Ptr socket, NetworkClient::State state)
{
  NetworkDirectLinkClient::Ptr info = NetworkDirectLinkClient::create(NetworkClient::SCOPE_DIRECT);
  info->state = state;
  info->socket = socket;
  info->address = socket->get_remote_address();
  info->stream = boost::shared_ptr<ByteStream>(new ByteStream(1024));
  connections.push_back(info);
  client_update_signal.emit(info);
  
  return info;
}


void
NetworkDirectLink::close(NetworkDirectLinkClient::Ptr info)
{
  TRACE_ENTER("NetworkDirectLink::close");
  info->state = NetworkClient::CONNECTION_STATE_CLOSED;
  info->socket->close();
  connections.remove(info);
  client_update_signal.emit(info);
  TRACE_EXIT()
}


sigc::signal<void, gsize, const gchar *, NetworkClient::Ptr> &
NetworkDirectLink::signal_data()
{
  return data_signal;
}


sigc::signal<void, NetworkClient::Ptr> &
NetworkDirectLink::signal_client_update()
{
  return client_update_signal;
}
