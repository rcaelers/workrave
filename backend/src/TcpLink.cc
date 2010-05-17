// TcpLink.cc
//
// Copyright (C) 2007, 2008, 2009, 2010 Rob Caelers <robc@krandor.nl>
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

#include <string.h>

#include "TcpLink.hh"

#include "IConfigurator.hh"
#include "CoreFactory.hh"
#include "ILinkListener.hh"
//#include "GNetSocketDriver.hh"
#include "Serializer.hh"
#include "BinaryArchive.hh"
#include "LinkEvent.hh"
#include "LinkException.hh"

using namespace std;
using namespace workrave;


//! Constructs a new Tcp Link.
TcpLink::TcpLink(ISocket *con)
  : socket(con),
    link_listener(NULL),
    authenticated(false)
{
  byte_stream.init(1024, 1024);

  if (socket != NULL)
    {
      socket->set_listener(this);
    }
}


//! Destructs the tcp link.
TcpLink::~TcpLink()
{
  if (socket != NULL)
    {
      socket->close();
    }
  delete socket;
}


//! Connects to a remote Workrave.
bool
TcpLink::connect(const string &host, int port)
{
  bool ret = true;
  try
    {
      socket = SocketDriver::create_socket();
      socket->set_listener(this);
      socket->connect(host, port);
    }
  catch (SocketException)
    {
      ret = false;
    }

  return ret;
}


void
TcpLink::init()
{
  send_auth();
}

void
TcpLink::process_packet()
{
  TRACE_ENTER("TcpLink::process_packet");

  try
    {
      byte_stream.rewind();

      guint32 size      = byte_stream.get_u32();
      guint16 version   = byte_stream.get_u16();
      guint16 type      = byte_stream.get_u16();
      WRID    uuid      = byte_stream.get_uuid();
      guint32 reserved1 = byte_stream.get_u32();
      guint32 reserved2 = byte_stream.get_u32();

      (void) size;
      (void) reserved1;
      (void) reserved2;

      if (version != 1)
        {
          throw LinkException("Incompatible verion");
        }
      if (uuid == link_id)
        {
          throw LinkException("Cycle");
        }
      if (!authenticated && (PacketType)type != PACKET_AUTH)
        {
          throw LinkException("Not authenticated");
        }

      switch((PacketType)type)
        {
        case PACKET_EVENT:
          process_event();
          break;

        case PACKET_AUTH:
          process_auth(uuid);
          break;

        default:
          throw LinkException("Unknown message type");
        }
    }
  catch (ByteStreamException)
    {
      throw LinkException("bytestream error");
    }

  TRACE_EXIT();
}


void
TcpLink::process_event()
{
  TRACE_ENTER("TcpLink::process_event");
  LinkEvent *event = NULL;

  try
    {
      BinaryArchive ba(&byte_stream);

      event = workrave::serialization::load<LinkEvent>(&ba);

      TRACE_MSG("incoming event " << event->str());
    }
  catch (Exception)
    {
      // Catch ByteStreamException and ArchiveException
      if (event != NULL)
        {
          delete event;
          event = NULL;
        }
    }

  if (link_listener != NULL &&
      event != NULL)
    {
      link_listener->event_received(link_id, event);
    }

  if (event != NULL)
    {
      delete event;
    }
  
  TRACE_EXIT();
}

void
TcpLink::process_auth(const WRID &uuid)
{
  TRACE_ENTER("TcpLink::process_auth");
  IConfigurator *config = CoreFactory::get_configurator();

  string secret;
  config->get_value("networking/secret", secret);

  int hash_len = g_checksum_type_get_length(G_CHECKSUM_SHA256);

  WRID random = byte_stream.get_uuid();
  gchar *username = byte_stream.get_string();
  guint8 *other_hash = byte_stream.get_raw(hash_len);

  string auth = random.str() + ":" + uuid.str() + ":" + string(username) + ":" + secret;
  char *hash = g_compute_checksum_for_data (G_CHECKSUM_SHA256, (const guchar*) auth.c_str(), auth.length());

  if (memcmp(hash, other_hash, hash_len) != 0)
    {
      g_free(hash);
  
      throw LinkException("Auth failed");
    }
  else
    {
      authenticated = true;
      if (link_listener != NULL)
        {
          link_listener->link_up(link_id);
        }
    }

  g_free(hash);

  TRACE_EXIT();
}



void
TcpLink::send_event(LinkEvent *event)
{
  TRACE_ENTER("TcpLink::send_event");

  try
    {
      BinaryArchive ba;
      workrave::serialization::save(*event, &ba);

      int size = ba.get_data_size();
      guint8 *data = ba.get_data();

      ByteStream header(32, 8);

      header.put_u32(size + 32);          // packet size
      header.put_u16(1);                  // packet version
      header.put_u16(PACKET_EVENT);       // packet content type
      header.put_uuid(link_id);           // Link ID
      header.put_u32(0);                  // Reserved
      header.put_u32(0);                  // Reserved

      int bytes_written = 0;

      socket->write(header.get_data(), header.get_offset(), bytes_written);

      if (bytes_written != header.get_offset())
        {
          throw LinkException("incomplete write");
        }

      socket->write(data, size, bytes_written);
      if (bytes_written != size)
        {
          throw LinkException("incomplete write");
        }
    }
  catch (serialization::ArchiveException)
    {
      throw LinkException("bytestream error");
    }
  catch (ByteStreamException)
    {
      throw LinkException("bytestream error");
    }
  catch (SocketException)
    {
      throw LinkException("socket exception");
    }

  TRACE_MSG(event->str());
  TRACE_EXIT();
}


void
TcpLink::send_auth()
{
  TRACE_ENTER("TcpLink::send_auth");
  IConfigurator *config = CoreFactory::get_configurator();

  string secret;
  string username;
  config->get_value("networking/username", username);
  config->get_value("networking/secret", secret);

  int hash_len = g_checksum_type_get_length(G_CHECKSUM_SHA256);
  
  WRID random_id;
  string random = random_id.str();
  string auth = random + ":" + link_id.str() + ":" + username + ":" + secret;

  char *hash = g_compute_checksum_for_data(G_CHECKSUM_SHA256, (const guchar*) auth.c_str(), auth.length());

  random_id.raw();

  int size = ( 32 +                     // header
               WRID::RAW_LENGTH +
               username.length() + 1 +  // username
               hash_len);   // digest


  try
    {
      int bytes_written = 0;
      ByteStream header(size, 8);

      header.put_u32(size);                 // packet size
      header.put_u16(1);                    // packet version
      header.put_u16(PACKET_AUTH);          // packet content type
      header.put_uuid(link_id);             // Link ID
      header.put_u32(0);                    // Reserved
      header.put_u32(0);                    // Reserved

      header.put_uuid(random);              // Random ID
      header.put_string(username);
      header.put_raw(hash_len, (guint8 *)hash);

      g_free(hash);

      socket->write(header.get_data(), header.get_offset(), bytes_written);
      if (bytes_written != header.get_offset() ||
          bytes_written != size)
        {
          throw LinkException("cannot send auth");
        }
    }
  catch (ByteStreamException)
    {
      throw LinkException("bytestream error");
    }
  catch (SocketException)
    {
      throw LinkException("socket expection");
    }

  TRACE_EXIT();
}


  //! Incoming data.
void
TcpLink::socket_io(ISocket *con)
{
  TRACE_ENTER("TcpLink::socket_io");
  bool error = false;

  try
    {
      int bytes_read = 0;
      int bytes_to_read = 4;

      if (byte_stream.get_offset() >= 4)
        {
          bytes_to_read = byte_stream.peek_u32(0) - byte_stream.get_offset();

          if (bytes_to_read + 4 > byte_stream.get_size())
            {
              byte_stream.grow(bytes_to_read + 4);
            }
        }

      con->read(byte_stream.get_data_ptr(), bytes_to_read, bytes_read);

      if (bytes_read == 0)
        {
          TRACE_MSG("socket closed");
          throw LinkException("link closed");
        }
      else
        {
          g_assert(bytes_read > 0);
          byte_stream.advance(bytes_read);

          if ((int)byte_stream.peek_u32(0) == byte_stream.get_offset())
            {
              process_packet();
              byte_stream.init(1024, 1024);
            }
        }
    }
  catch (Exception e)
    {
      TRACE_MSG("Exception: " << e.details());
      error = true;
    }
  catch (...)
    {
      TRACE_MSG("Exception");
      error = true;
    }

  if (error)
    {
      socket->close();
      socket_closed(con);
    }

  TRACE_EXIT();
}


  //! Connection established
void
TcpLink::socket_connected(ISocket *con)
{
  TRACE_ENTER("TcpLink::socket_connected");
  (void) con;

  try
    {
      send_auth();
    }
  catch (Exception &e)
    {
      TRACE_MSG(e.details());
    }
  catch(...)
    {
      socket->close();
      socket_closed(con);
    }

  TRACE_EXIT();
}


  //! Connection closed.
void
TcpLink::socket_closed(ISocket *con)
{
  TRACE_ENTER("TcpLink::socket_closed");
  (void) con;

  try
    {
      if (link_listener != NULL)
        {
          link_listener->link_down(link_id);
        }
    }
  catch(...)
    {
    }

  TRACE_EXIT();
}

