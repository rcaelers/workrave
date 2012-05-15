// NetworkRouter.cc
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

#include <string.h>
#include <string>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <glib.h>

#include "debug.hh"

#include "NetworkRouter.hh"

#include "workrave.pb.h"
#include "cloud.pb.h"

#include "Util.hh"

using namespace std;

//! Constructs a new network router
NetworkRouter::NetworkRouter(string username, string secret) : username(username), secret(secret)
{
  TRACE_ENTER("NetworkRouter::NetworkRouter");
  announce = NetworkAnnounce::create();
  direct_links = NetworkDirectLink::create();
  TRACE_EXIT();
}


//! Destructs the network router.
NetworkRouter::~NetworkRouter()
{
  TRACE_ENTER("NetworkRouter::~NetworkRouter");
  TRACE_EXIT();
}


//! Initializes the network router.
void
NetworkRouter::init(int port)
{
  TRACE_ENTER("NetworkRouter::init");

  init_myid(port);

  announce->init(port);
  direct_links->init(port);

  announce->signal_data().connect(sigc::mem_fun(*this, &NetworkRouter::on_data));
  direct_links->signal_data().connect(sigc::mem_fun(*this, &NetworkRouter::on_data));
  direct_links->signal_client_update().connect(sigc::mem_fun(*this, &NetworkRouter::on_client_changed));
  
  TRACE_EXIT();
}


//! Terminates the network announcer.
void
NetworkRouter::terminate()
{
  TRACE_ENTER("NetworkRouter::terminate");
  TRACE_EXIT();
}


//! Periodic heartbeart from the core.
void
NetworkRouter::heartbeat()
{
  TRACE_ENTER("NetworkRouter::heartbeat");
  static bool once = false;
  
  // TODO: debugging code.
  if (!once)
    {
      NetworkMessage<cloud::ActivityState>::Ptr as = NetworkMessage<cloud::ActivityState>::create();
      as->scope = NetworkClient::SCOPE_DIRECT;
      as->authenticated = true;

      boost::shared_ptr<cloud::ActivityState> a = as->msg();
      //as->msg()->set_state(1);
      send_message(as);

      once = true;
    }
  
  TRACE_EXIT();
}


void
NetworkRouter::connect(const string &host, int port)
{
  TRACE_ENTER_MSG("NetworkRouter::connect", host << " " << port << " (" << myid.str() << ")");
  direct_links->connect(host, port);
  TRACE_EXIT();
}


void
NetworkRouter::send_message(NetworkMessageBase::Ptr base)
{
  TRACE_ENTER_MSG("NetworkRouter::send_message", base->scope << " (" << myid.str() << ")");
  
  string str = marshall_message(base);
  
  if ((base->scope & NetworkClient::SCOPE_DIRECT) == NetworkClient::SCOPE_DIRECT)
    {
      direct_links->send_message(str);
    }
  
  if ((base->scope & NetworkClient::SCOPE_MULTICAST) == NetworkClient::SCOPE_MULTICAST)
    {
      announce->send_message(str);
    }
      
  TRACE_EXIT();
}

void
NetworkRouter::send_message_except(boost::shared_ptr<workrave::Header> header,
                                   boost::shared_ptr<google::protobuf::Message> message,
                                   NetworkClient::Ptr client)
{
  TRACE_ENTER_MSG("NetworkRouter::send_message_except", "(" << myid.str() << ")");
  string str = marshall_message(header, message);
  
  direct_links->send_message_except(str, client);
      
  TRACE_EXIT();
}


NetworkRouter::MessageSignal &
NetworkRouter::signal_message(int domain, int id)
{
  return message_signals[std::make_pair(domain, id)];
}


void
NetworkRouter::on_data(gsize size, const gchar *data, NetworkClient::Ptr client)
{
  TRACE_ENTER_MSG("NetworkRouter::on_data", " (" << myid.str() << ")");
  boost::shared_ptr<google::protobuf::Message> message;
  boost::shared_ptr<workrave::Header> header;
  
  bool ok = unmarshall_message(size, data, client, message, header);
  if (ok)
    {
      TRACE_MSG(client);
      TRACE_MSG(client->address->str());
      TRACE_MSG(header->DebugString());
      TRACE_MSG(message->DebugString());
      TRACE_MSG(client->authenticated);

      if (myid == UUID::from_raw(header->source()))
        {
          // TODO: Handle cycle
          TRACE_MSG("cycle!");
          ok = false;
        }
    }

  if (ok && client->authenticated)
    {
      NetworkMessageBase::Ptr m = NetworkMessageBase::create(header, message); 
      
      fire_message_signal(header->domain(), header->payload(), m);
      send_message_except(header, message, client);
    }
  TRACE_EXIT();
}


void
NetworkRouter::process_message(boost::shared_ptr<workrave::Header> header, boost::shared_ptr<google::protobuf::Message> message)
{
  (void) header;
  (void) message;
}


const string
NetworkRouter::get_nonce() const
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
NetworkRouter::check_message_authentication(boost::shared_ptr<workrave::Header> header, NetworkClient::Ptr client)
{
  TRACE_ENTER("NetworkRouter::check_message_authentication");
  bool ret = true;

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

      client->authenticated = ret;
      g_free(hash);
    }

  TRACE_EXIT();
  return ret;
}


void
NetworkRouter::add_message_authentication(boost::shared_ptr<workrave::Header> header)
{
  TRACE_ENTER("NetworkRouter::add_message_authentication");
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
NetworkRouter::marshall_message(NetworkMessageBase::Ptr message)
{
  TRACE_ENTER("NetworkRouter::marshall_message");
  string ret;

  boost::shared_ptr<workrave::Header> header(new workrave::Header());
  
  header->set_source(myid.str());

  if (message->authenticated)
    {
      add_message_authentication(header);
    }

  ret = marshall_message(header, message->msg());
  TRACE_EXIT();
  return ret;
}

const string
NetworkRouter::marshall_message(boost::shared_ptr<workrave::Header> header, boost::shared_ptr<google::protobuf::Message> message)
{
  TRACE_ENTER("NetworkRouter::marshall_message");
  string ret;
  
  const google::protobuf::Descriptor *ext = message->GetDescriptor();
  if (ext != NULL)
    {
      const google::protobuf::FieldDescriptor *fd = ext->extension(0);
      if (fd != NULL)
        {
          int num = fd->number();

          header->set_payload(num);
          header->set_domain(1);

          boost::shared_ptr<ByteStreamOutput> output(new ByteStreamOutput(1024));

          output->write_u16(header->ByteSize());
          output->write_u16(message->ByteSize());
      
          header->SerializeToZeroCopyStream(output.get());

          const gchar *payload = output->get_ptr();
          message->SerializeToZeroCopyStream(output.get());
          gsize payload_size = output->get_ptr() - payload;

          // TODO:
          GChecksum *checksum = g_checksum_new(G_CHECKSUM_SHA256);
          g_checksum_update(checksum, (const guchar *)payload, payload_size);
          g_checksum_update(checksum, (const guchar *)secret.data(), secret.size());
          const gchar *hash = g_checksum_get_string(checksum);

          TRACE_MSG(hash);
          
          TRACE_MSG(output->get_position());
          ret = output->get_buffer_as_string();
          TRACE_MSG(ret.length());
          output.reset();
        }
    }

  TRACE_EXIT();
  return ret;
}


bool
NetworkRouter::unmarshall_message(gsize size, const gchar *data, NetworkClient::Ptr client,
                                  boost::shared_ptr<google::protobuf::Message> &message,
                                  boost::shared_ptr<workrave::Header> &header)
{
  TRACE_ENTER_MSG("NetworkRouter::unmarshall_message", " (" << myid.str() << ")");
  bool result = false;
  
  try
    {
      boost::shared_ptr<ByteStreamInput> input(new ByteStreamInput(data, size));

      bool header_ok = true;;
      guint16 header_size;
      guint16 msg_size;
      const google::protobuf::Descriptor *base = NULL;
      const google::protobuf::FieldDescriptor *field = NULL;
      const google::protobuf::Descriptor *ext = NULL;

      header_ok = header_ok && input->read_u16(header_size);
      header_ok = header_ok && input->read_u16(msg_size);

      TRACE_MSG(header_size);
      TRACE_MSG(msg_size);
      
      if (header_ok)
        {
          header_ok = header->ParseFromBoundedZeroCopyStream(input.get(), header_size);
        }
      
      if (header_ok)
        {
          header_ok = check_message_authentication(header, client);
        }
      
      if (header_ok && size >= header_size + msg_size)
        {
          const string domain_name = get_namespace_of_domain(header->domain()) + ".Domain";
          base = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(domain_name);
        }

      if (base != NULL)
        {
          field = google::protobuf::DescriptorPool::generated_pool()->FindExtensionByNumber(base, header->payload());
        }
      
      if (field != NULL)
        {
          ext = field->extension_scope();
        }
      
      if (ext != NULL)
        {
          google::protobuf::Message *msg = google::protobuf::MessageFactory::generated_factory()->GetPrototype(ext)->New();

          // TODO:
          GChecksum *checksum = g_checksum_new(G_CHECKSUM_SHA256);
          g_checksum_update(checksum, (const guchar *)input.get()->get_ptr(), msg_size);
          g_checksum_update(checksum, (const guchar *)secret.data(), secret.size());
          const gchar *hash = g_checksum_get_string(checksum);

          TRACE_MSG(hash);

          if (msg->ParseFromBoundedZeroCopyStream(input.get(), msg_size))
            {
              message = boost::shared_ptr<google::protobuf::Message>(msg);
              result = true;
            }
        }
    }
  catch(...)
    {
    }
  
  return result;
  TRACE_EXIT();
}


string
NetworkRouter::get_namespace_of_domain(int domain)
{
  switch (domain)
    {
    case 1:
      return "cloud";
    }

  return "";
}


void
NetworkRouter::on_client_changed(NetworkClient::Ptr client)
{
  TRACE_ENTER_MSG("NetworkRouter::on_client_changed", "(" << myid.str() << ")");
  ClientIter it = find(clients.begin(), clients.end(), client);

  if (client->state == NetworkClient::CONNECTION_STATE_CLOSED &&
      it != clients.end())
    {
      clients.erase(it);
      TRACE_MSG("closed");
    }
  else if (it == clients.end())
    {
      clients.push_back(client);
      TRACE_MSG("new");
    }
  TRACE_MSG(client->address->str());
  TRACE_EXIT();
}


void
NetworkRouter::init_myid(int instanceid)
{
  TRACE_ENTER("Network::init_myid");
  bool ok = false;
  stringstream ss;

  ss << Util::get_home_directory() << "id-" << instanceid << ends;
  string idfilename = ss.str();

  if (Util::file_exists(idfilename))
    {
      ifstream file(idfilename.c_str());
      
      if (file)
        {
          string id_str;
          file >> id_str;

          myid = UUID::from_str(id_str);              

          file.close();
        }
    }

  if (! ok)
    {
      ofstream file(idfilename.c_str());

      file << myid.str() << endl;
      file.close();
    }


  TRACE_EXIT();
}


void
NetworkRouter::fire_message_signal(int domain, int id, NetworkMessageBase::Ptr message)
{
  MessageSignalMapIter it = message_signals.find(std::make_pair(domain, id));
  if (it != message_signals.end())
    {
      it->second.emit(message);
    }
}
