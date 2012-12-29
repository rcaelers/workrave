// Marshaller.cc
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

#define TRACE_EXTRA " (" << myid.str() << ")"
#include "debug.hh"

#include <string.h>
#include <string>
#include <sstream>

#include <boost/shared_ptr.hpp>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <glib.h>

#include "ByteStream.hh"
#include "Marshaller.hh"
#include "Util.hh"

#include "cloud.pb.h"

using namespace std;


//! Create a new network data marshaller
Marshaller::Ptr
Marshaller::create()
{
  return Marshaller::Ptr(new Marshaller());
}


//! Constructs a new network data marshaller
Marshaller::Marshaller()
{
  TRACE_ENTER("Marshaller::Marshaller");
  TRACE_EXIT();
}


//! Destructs the network data marshaller.
Marshaller::~Marshaller()
{
  TRACE_ENTER("Marshaller::~Marshaller");
  TRACE_EXIT();
}


void
Marshaller::set_id(UUID &id)
{
  myid = id;
}


void
Marshaller::set_credentials(const std::string &username, const std::string &secret)
{
  this->username = username;
  this->secret = secret;
}


const string
Marshaller::get_nonce() const
{
  static bool random_seeded = 1;
  const int nonce_size = 32;
  const char *valid_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  const int valid_chars_count = strlen(valid_chars);
  char nonce[nonce_size + 1] = { 0, };
  
  if (!random_seeded)
    {
      g_random_set_seed(time(NULL));
      random_seeded = true;
    }

  for (int i = 0; i < nonce_size; i++)
    {
      nonce[i] = valid_chars[g_random_int_range(0, valid_chars_count)];
    }

  return nonce;
}


bool
Marshaller::check_authentication(Header::Ptr header)
{
  TRACE_ENTER("Marshaller::check_message_authentication");
  bool ret = false;

  if (header->has_auth())
    {
      int hash_len = g_checksum_type_get_length(G_CHECKSUM_SHA256);
      string nonce = header->auth().nonce();
      string remote_username = header->auth().username();
      string remote_hash = header->auth().hash();
      
      string auth = nonce + ":" + username + ":" + secret;
      char *hash = g_compute_checksum_for_data(G_CHECKSUM_SHA256, (const guchar*) auth.c_str(), auth.length());
      
      TRACE_MSG(auth);
      
      ret = (memcmp(hash, remote_hash.c_str(), hash_len) == 0) && remote_username == username;

      g_free(hash);
    }

  TRACE_EXIT();
  return ret;
}


void
Marshaller::add_authentication(Header::Ptr header)
{
  TRACE_ENTER("Marshaller::add_message_authentication");
  int hash_len = g_checksum_type_get_length(G_CHECKSUM_SHA256);
  string nonce = get_nonce();
  string auth = nonce + ":" + username + ":" + secret;

  TRACE_MSG(auth);
  
  char *hash = g_compute_checksum_for_data(G_CHECKSUM_SHA256, (const guchar*) auth.c_str(), auth.length());

  header->mutable_auth()->set_nonce(nonce);
  header->mutable_auth()->set_username(username);
  header->mutable_auth()->set_hash(string(hash, hash_len));

  g_free(hash);
  TRACE_EXIT();
}


const string
Marshaller::marshall(PacketOut::Ptr packet)
{
  TRACE_ENTER("Marshaller::marshall_message");
  string ret;

  Header::Ptr header(new proto::Header);

  header->set_source(packet->source.raw());

  if (packet->sign)
    {
      add_authentication(header);
    }
  
  const google::protobuf::Descriptor *ext = packet->message->GetDescriptor();
  if (ext != NULL)
    {
      TRACE_MSG("Msg type " << ext->full_name());
      const google::protobuf::FieldDescriptor *fd = ext->extension(0);
      if (fd != NULL)
        {
          int num = fd->number();

          const string &full_name = ext->full_name();
          std:: string::size_type pos = full_name.rfind('.');
          if (pos != std::string::npos)
            {
              TRACE_MSG("Domain " <<  num << " " << full_name.substr(0, pos));
              header->set_domain(get_domain_of_namespace(full_name.substr(0, pos)));
            }
          else
            {
              header->set_domain(1);
            }
      
          header->set_payload(num);

          boost::shared_ptr<ByteStreamOutput> output(new ByteStreamOutput(1024));

          TRACE_MSG("header : " << header->ByteSize() << " msg: " << packet->message->ByteSize());
          output->write_u16(header->ByteSize());
          output->write_u16(packet->message->ByteSize());
      
          header->SerializeToZeroCopyStream(output.get());

          TRACE_MSG("packet : " << output->get_position());
          
          //const gchar *payload = output->get_ptr();
          packet->message->SerializeToZeroCopyStream(output.get());
          //gsize payload_size = output->get_ptr() - payload;

          // TODO:
          //GChecksum *checksum = g_checksum_new(G_CHECKSUM_SHA256);
          //g_checksum_update(checksum, (const guchar *)payload, payload_size);
          //g_checksum_update(checksum, (const guchar *)secret.data(), secret.size());
          //const gchar *hash = g_checksum_get_string(checksum);

          ret = output->get_buffer_as_string();
          TRACE_MSG("packet str: " << ret.size());
          output.reset();
        }
    }

  TRACE_EXIT();
  return ret;
}


PacketIn::Ptr
Marshaller::unmarshall(gsize size, const gchar *data)
{
  TRACE_ENTER("Marshaller::unmarshall_message");
  
  try
    {
      Header::Ptr header;
      boost::shared_ptr<ByteStreamInput> input(new ByteStreamInput(data, size));

      bool header_ok = true;
      guint16 header_size = 0;
      guint16 msg_size = 0;
      const google::protobuf::Descriptor *base = NULL;
      const google::protobuf::FieldDescriptor *field = NULL;
      const google::protobuf::Descriptor *ext = NULL;

      header_ok = header_ok && input->read_u16(header_size);
      header_ok = header_ok && input->read_u16(msg_size);

      TRACE_MSG("size: " << size << " header: " << header_size << " msg: " << msg_size);
      TRACE_MSG("available: " << input->get_available() << " " << input->get_position());
      
      if (header_ok)
        {
          header = Header::Ptr(new proto::Header());
          header_ok = header->ParseFromBoundedZeroCopyStream(input.get(), header_size);
        }
      
      TRACE_MSG("available: " << input->get_available() << " " << input->get_position());

      if (header_ok && size >= header_size + msg_size)
        {
          TRACE_MSG("Header: " << header->DebugString());
          TRACE_MSG("Source: " << UUID::from_raw(header->source()).str());

          const string domain_name = get_namespace_of_domain(header->domain()) + ".Domain";
          TRACE_MSG("Domain: " << domain_name);
          base = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(domain_name);
        }

      if (base != NULL)
        {
          TRACE_MSG("OK Base, payload:" <<  header->payload() << " name: " << base->full_name());
          field = google::protobuf::DescriptorPool::generated_pool()->FindExtensionByNumber(base, header->payload());
        }
      
      if (field != NULL)
        {
          TRACE_MSG("OK Field");
          ext = field->extension_scope();
        }
      
      if (ext != NULL)
        {
          TRACE_MSG("OK Ext");
          boost::shared_ptr<google::protobuf::Message> message(google::protobuf::MessageFactory::generated_factory()->GetPrototype(ext)->New());

          const google::protobuf::Descriptor *d = message->GetDescriptor();
          TRACE_MSG("Msg type " << d->full_name());

          TRACE_MSG("Checksum size " << msg_size << " " << secret.size());

          // TODO:
          // GChecksum *checksum = g_checksum_new(G_CHECKSUM_SHA256);
          // if (msg_size > 0)
          //   {
          //     g_checksum_update(checksum, (const guchar *)input->get_ptr(), msg_size);
          //   }
          // if (secret.size() > 0)
          //   {
          //     g_checksum_update(checksum, (const guchar *)secret.data(), secret.size());
          //   }
          // const gchar *hash = g_checksum_get_string(checksum);

          // TRACE_MSG(hash);

          if (message->ParseFromBoundedZeroCopyStream(input.get(), msg_size))
            {
              TRACE_MSG("available: " << input->get_available() << " " << input->get_position());
              TRACE_MSG("Message: " << message->DebugString());
              
              PacketIn::Ptr ret = PacketIn::create(header, message);
              
              ret->authentic = check_authentication(header);
              ret->source = UUID::from_raw(header->source());

              return ret;
            }
        }
    }
  catch(...)
    {
    }
  
  TRACE_EXIT();
  return PacketIn::Ptr();
}


string
Marshaller::get_namespace_of_domain(int domain)
{
  switch (domain)
    {
    case 0:
      return "workrave.cloud.proto";

    case 1:
      return "workrave.networking";
    }

  return "";
}

int
Marshaller::get_domain_of_namespace(const string &ns)
{
  if (ns == "workrave.cloud.proto")
    {
      return 0;
    }
  else if (ns == "workrave.networking")
    {
      return 1;
    }

  return 0;
}
