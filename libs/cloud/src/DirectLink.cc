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

#include "DirectLink.hh"
#include "ByteStream.hh"

using namespace std;

DirectLink::Ptr
DirectLink::create(Marshaller::Ptr marshaller)
{
  return DirectLink::Ptr(new DirectLink(marshaller));
}


DirectLink::Ptr
DirectLink::create(Marshaller::Ptr marshaller, Socket::Ptr socket)
{
  return DirectLink::Ptr(new DirectLink(marshaller, socket));
}


DirectLink::DirectLink(Marshaller::Ptr marshaller) : marshaller(marshaller)
{
  TRACE_ENTER("DirectLink::DirectLink");
  stream = boost::shared_ptr<ByteStream>(new ByteStream(1024));
  socket = Socket::create();
  this->state = CONNECTION_STATE_INVALID;
  TRACE_EXIT();
}


DirectLink::DirectLink(Marshaller::Ptr marshaller, Socket::Ptr socket) : marshaller(marshaller), socket(socket)
{
  TRACE_ENTER("DirectLink::DirectLink");
  stream = boost::shared_ptr<ByteStream>(new ByteStream(1024));
  this->state = CONNECTION_STATE_CONNECTED;
  this->address = socket->get_remote_address();

  socket->signal_io().connect(boost::bind(&DirectLink::on_data, this));
  socket->signal_disconnected().connect(boost::bind(&DirectLink::on_disconnected, this));
  TRACE_EXIT();
}


DirectLink::~DirectLink()
{
  TRACE_ENTER("DirectLink::~DirectLink");
  TRACE_EXIT();
}


void
DirectLink::connect(const string &host, int port)
{
  TRACE_ENTER_MSG("DirectLink::connect", host << " " << port);

  state = CONNECTION_STATE_CONNECTING;
  socket->connect(host, port);

  socket->signal_io().connect(boost::bind(&DirectLink::on_data, this));
  socket->signal_connected().connect(boost::bind(&DirectLink::on_connected, this));
  socket->signal_disconnected().connect(boost::bind(&DirectLink::on_disconnected, this));
  
  state_signal();
  TRACE_EXIT();
}


void
DirectLink::send_message(const string &message)
{
  TRACE_ENTER("DirectLink::send_message");

  if (state == CONNECTION_STATE_CONNECTED)
    {
      guint16 length = GINT16_TO_BE(message.length());
      
      socket->write((gchar *) (&length), 2);
      socket->write(message.data(), message.length());
    }
  
  TRACE_EXIT();
}


void
DirectLink::on_connected()
{
  TRACE_ENTER("DirectLink::on_connected");
  address = socket->get_remote_address();
  state = Link::CONNECTION_STATE_CONNECTED;
  state_signal();
  TRACE_EXIT();
}


void
DirectLink::on_disconnected()
{
  TRACE_ENTER("DirectLink::on_disconnected");
  close();
  TRACE_EXIT();
}


//!
void
DirectLink::on_data()
{
  TRACE_ENTER("DirectLink::on_data");
  
  bool error = false;
  
  try
    {
      const gsize header_size = sizeof(guint16);
      gsize bytes_read = 0;
      gsize bytes_to_read = 2;
      guint16 packet_size = 0;

      TRACE_MSG(stream->get_position());
      
      if (stream->get_position() >= header_size)
        {
          stream->read_u16(packet_size, 0);
          
          bytes_to_read = header_size + packet_size - stream->get_position();

          stream->resize(header_size + packet_size);
        }

      TRACE_MSG(bytes_to_read);
      
      if (bytes_to_read > 0)
        {
          socket->read(stream->get_ptr(), bytes_to_read, bytes_read);
          if (bytes_read == 0)
            {
              TRACE_MSG("socket closed");
              error = true;
            }
          else
            {
              g_assert(bytes_read > 0);
              stream->advance_buffer(bytes_read);
            }
          TRACE_MSG(bytes_read);
        }
      
      if (packet_size > 0 && header_size + packet_size == stream->get_position())
        {
          PacketIn::Ptr packet = marshaller->unmarshall(packet_size, stream->get_start() + header_size);

          if (packet)
            {
              boost::optional<bool> r = data_signal(packet);
              TRACE_MSG("signal result" << r << " " << (*r));
              if (!(*r))
              {
                error = true;
              }
            }
          
          stream->init(1024);
        }
    }
  catch(...)
    {
      TRACE_MSG("Exception");
      error = true;
    }

  if (error)
    {
      close();
    }

  TRACE_EXIT();
}


void
DirectLink::close()
{
  TRACE_ENTER("DirectLink::close");
  state = Link::CONNECTION_STATE_CLOSED;
  socket->close();
  state_signal();
  TRACE_EXIT()
}


boost::signals2::signal<bool(PacketIn::Ptr), DirectLink::BoolOrCombiner> &
DirectLink::signal_data()
{
  return data_signal;
}


boost::signals2::signal<void()> &
DirectLink::signal_state()
{
  return state_signal;
}
