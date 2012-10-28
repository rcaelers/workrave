// Router.cc
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

#include "debug.hh"

#include <string.h>
#include <string>
#include <sstream>
#include <fstream>

#include <boost/shared_ptr.hpp>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <glib.h>

#include "Router.hh"
#include "Util.hh"

#include "cloud.pb.h"

using namespace std;
using namespace workrave::cloud;

//! Create a new network router
Router::Ptr
Router::create()
{
  return Router::Ptr(new Router());
}

// TODO: move to factory
//! Create a new network router
ICloud::Ptr
ICloud::create()
{
  return Router::create();
}


//! Constructs a new network router
Router::Router() : cycle_failures(0)
{
  TRACE_ENTER("Router::Router");
  marshaller = Marshaller::create();
  announce = Announce::create(marshaller);
  direct_link_manager = DirectLinkManager::create(marshaller);
  TRACE_EXIT();
}


//! Destructs the network router.
Router::~Router()
{
  TRACE_ENTER("Router::~Router");
  TRACE_EXIT();
}


//! Initializes the network router.
void
Router::init(int port, string username, string secret)
{
  TRACE_ENTER("Router::init");
  this->username = username;
  this->secret = secret;
    
  init_myid(port);

  marshaller->set_id(myid);
  marshaller->set_credentials(username, secret);
  
  announce->init(IRouter::WeakPtr(shared_from_this()), 7272, myid, port);
  direct_link_manager->init(port);

  announce->signal_data().connect(boost::bind(&Router::on_data, this, _1, _2, SCOPE_MULTICAST));
  direct_link_manager->signal_new_link().connect(boost::bind(&Router::on_direct_link_created, this, _1));

  TRACE_EXIT();
}


//! Terminates the network router.
void
Router::terminate()
{
  TRACE_ENTER("Router::terminate");
  TRACE_EXIT();
}


void
Router::heartbeat()
{
}


void
Router::connect(const string &host, int port)
{
  TRACE_ENTER_MSG("Router::connect", host << " " << port);
  
  Client::Ptr client = Client::create();
  
  DirectLink::Ptr link = DirectLink::create(marshaller);
  link->connect(host, port);

  link->signal_data().connect(boost::bind(&Router::on_data, this, _1, _2, SCOPE_DIRECT));
  link->signal_state().connect(boost::bind(&Router::on_direct_link_state_changed, this, _1));
  
  client->link = link;

  TRACE_MSG("Add client " << client.get());
  clients.push_back(client);

  TRACE_EXIT();
}


void
Router::connect(NetworkAddress::Ptr host, int port)
{
  TRACE_ENTER_MSG("Router::connect", host << " " << port);
  
  connect(host->addr_str(),  port);

  TRACE_EXIT();
}



list<workrave::cloud::ClientInfo>
Router::get_client_infos() const
{
  list<workrave::cloud::ClientInfo> ret;

  for (ClientCIter it = clients.begin(); it != clients.end(); it++)
    {
      ClientInfo info;
      info.id = (*it)->id;

      ViaLink::Ptr via = boost::dynamic_pointer_cast<ViaLink>((*it)->link);
      bool x = via;
      info.via = x ? via->id : info.id;
            
      ret.push_back(info);
    }
  return ret;
}

void
Router::send_message(Message::Ptr message, MessageParams::Ptr params)
{
  TRACE_ENTER("Router::send_message");
  
  PacketOut::Ptr packet = PacketOut::create(message);
  packet->sign = params->sign;
  packet->source = myid;
  
  string msg_str = marshaller->marshall(packet);

  Scope scope = params->scope;
  
  if ((scope & SCOPE_DIRECT) == SCOPE_DIRECT)
    {
      for (ClientIter it = clients.begin(); it != clients.end(); it++)
        {
          TRACE_MSG((*it).get());
          if ((*it)->link->state == Link::CONNECTION_STATE_CONNECTED)
            {
              (*it)->link->send_message(msg_str);
            }
        }
    }
  
  if ((scope & SCOPE_MULTICAST) == SCOPE_MULTICAST)
    {
      announce->send_message(msg_str);
    }
      
  TRACE_EXIT();
}


void
Router::send_message(Link::Ptr link, Message::Ptr message, MessageParams::Ptr params)
{
  TRACE_ENTER("Router::send_message");
  
  PacketOut::Ptr packet = PacketOut::create(message);
  packet->sign = params->sign;
  packet->source = myid;
  
  string msg_str = marshaller->marshall(packet);

  if (link->state == Link::CONNECTION_STATE_CONNECTED)
    {
      link->send_message(msg_str);
    }
      
  TRACE_EXIT();
}

void
Router::forward_message(Link::Ptr link, PacketIn::Ptr packet)
{
  TRACE_ENTER("Router::forward_message");
  
  PacketOut::Ptr packet_out(PacketOut::create(packet->message));
  packet_out->sign = packet->authentic;
  packet_out->source = packet->source;
  
  string msg_str = marshaller->marshall(packet_out);

  for (ClientIter it = clients.begin(); it != clients.end(); it++)
    {
      TRACE_MSG((*it).get());

      if ((*it)->link->state == Link::CONNECTION_STATE_CONNECTED &&
          (*it)->link != link)
        {
          TRACE_MSG("forwarding");
          (*it)->link->send_message(msg_str);
        }
    }

  TRACE_EXIT();
}

Router::MessageSignal &
Router::signal_message(int domain, int id)
{
  if (message_signals[std::make_pair(domain, id)] == NULL)
    {
      message_signals[std::make_pair(domain, id)].reset(new MessageSignal);
    }
  return *message_signals[std::make_pair(domain, id)].get();
}


bool
Router::on_data(Link::Ptr link, PacketIn::Ptr packet, Scope scope)
{
  TRACE_ENTER("Router::on_data");
  bool ret = true;
  
  if (packet)
    {
      TRACE_MSG("Source " << packet->source.str());
      //TRACE_MSG("Link " << link->id.str());

      if (myid == packet->source)
        {
          TRACE_MSG("cycle!");
          cycle_failures++;
          ret = false;
        }

      if (ret && packet->authentic)
        {
          MessageContext::Ptr context(MessageContext::create());
          context->source = packet->source;
          context->scope = scope;
          
          fire_message_signal(packet->header->domain(), packet->header->payload(), packet->message, context);

          bool need_to_forward = true;
          if (packet->header->domain() == 0)
            {
              if (packet->header->payload() == proto::Alive::kTypeFieldNumber)
                {
                  process_alive(link, packet);
                  need_to_forward = false;
                }
              else if (packet->header->payload() == proto::Signoff::kTypeFieldNumber)
                {
                  process_signoff(link, packet);
                }
            }
           
          if (need_to_forward)
            {
              forward_message(link, packet);
            }
        }
    }
  TRACE_RETURN(ret);
  return ret;
}


void
Router::on_direct_link_created(DirectLink::Ptr link)
{
  TRACE_ENTER("Router::on_direct_link_created");
  TRACE_MSG(link);

  Client::Ptr client = Client::create();
  
  link->signal_data().connect(boost::bind(&Router::on_data, this, _1, _2, SCOPE_DIRECT));
  link->signal_state().connect(boost::bind(&Router::on_direct_link_state_changed, this, _1));

  client->link = link;
  
  clients.push_back(client);
  TRACE_MSG("Add client " << client.get());
  send_alive();
  
  TRACE_EXIT();
}


void
Router::on_direct_link_state_changed(DirectLink::Ptr link)
{
  TRACE_ENTER("Router::on_direct_link_state_changed");
  
  Client::Ptr client = find_client(link);
  if (client)
    {
      NetworkAddress::Ptr a = link->address;
      if (a)
        {
          TRACE_MSG("cl " << a->str());
        }

      list<UUID> ids;

      ids.push_back(client->id);
      
      if (link->state == Link::CONNECTION_STATE_CLOSED)
        {
          clients.remove(client);
          TRACE_MSG("closed");

          for (ClientIter it = clients.begin(); it != clients.end(); )
            {
              ViaLink::Ptr via = boost::dynamic_pointer_cast<ViaLink>((*it)->link);
              if (via && via->link == link)
                {
                  ids.push_back((*it)->id);
                  it = clients.erase(it);
                }
              else
                {
                  it++;
                }
            }

          send_signoff(ids);
        }
      else if (link->state == Link::CONNECTION_STATE_CONNECTED)
        {
          TRACE_MSG("connected");
          send_alive(link);
        }
    }
  
  TRACE_EXIT();
}

void
Router::init_myid(int instanceid)
{
  TRACE_ENTER("::init_myid");
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
Router::fire_message_signal(int domain, int id, Message::Ptr message, MessageContext::Ptr context)
{
  MessageSignalMapIter it = message_signals.find(std::make_pair(domain, id));
  if (it != message_signals.end())
    {
      MessageSignal &m = *it->second;
      m(message, context);
    }
}


void
Router::send_alive(Link::Ptr link)
{
  TRACE_ENTER("Router::send_alive");
  boost::shared_ptr<proto::Alive> a(new proto::Alive());

  string *c = a->add_path();
  *c = myid.raw();

  if (link)
    {
      send_message(link, a, MessageParams::create());
    }
  else
    {
      send_message(a, MessageParams::create());
    }
  
  TRACE_EXIT();
}

void
Router::send_signoff(list<UUID> ids)
{
  TRACE_ENTER("Router::send_signoff");
  boost::shared_ptr<proto::Signoff> a(new proto::Signoff());

  for (list<UUID>::iterator it = ids.begin(); it != ids.end(); it++)
    {
      string *new_id = a->add_id();
      *new_id = (*it).raw();
    }

  send_message(a, MessageParams::create());

  TRACE_EXIT();
}


Client::Ptr
Router::find_client(UUID id)
{
  TRACE_ENTER_MSG("Router::find_client", id.str());
  for (ClientIter it = clients.begin(); it != clients.end(); it++)
    {
      TRACE_MSG("checking " << (*it).get());
      if ((*it)->id == id)
        {
          return *it;
        }
    }
  TRACE_EXIT();
  return Client::Ptr();
}


Client::Ptr
Router::find_client(Link::Ptr link)
{
  TRACE_ENTER("Router::find_client");
  for (ClientIter it = clients.begin(); it != clients.end(); it++)
    {
      TRACE_MSG("checking " << (*it).get());

      if ((*it)->link == link)
        {
          return *it;
        }
    }
  TRACE_EXIT();
  return Client::Ptr();
}


void
Router::process_alive(Link::Ptr link, PacketIn::Ptr packet)
{
  TRACE_ENTER("Router::process_alive");
    
  boost::shared_ptr<proto::Alive> a = boost::dynamic_pointer_cast<proto::Alive>(packet->message);

  if (a)
    {
      google::protobuf::RepeatedPtrField<string> path = a->path();
      UUID &id = packet->source;
      
      TRACE_MSG("Source " << packet->source.str());
      for (google::protobuf::RepeatedPtrField<string>::iterator i = path.begin(); i != path.end(); i++)
        {
          TRACE_MSG("Path " << UUID::from_raw(*i).str());
        }

      Client::Ptr client;

      if (a->path_size() == 1)
        {
          TRACE_MSG("Direct connection");
          client = find_client(link);
        }
      else
        {
          TRACE_MSG("Via connection");
          client = find_client(id);

          if (!client)
            {
              ViaLink::Ptr via_link = ViaLink::create(link);
              via_link->state = Link::CONNECTION_STATE_CONNECTED;

              TRACE_MSG(via_link.get());
              client = Client::create();
              client->link = via_link;
              client->id = id;
          
              clients.push_back(client);
              TRACE_MSG("Add client " << client.get());
              send_alive();
            }
        }
        
      if (client)
        {
          TRACE_MSG("have client " << UUID::from_raw(a->path(0)).str());
              
          if (!client->authenticated)
            {
              client->authenticated = true;
              client->id = UUID::from_raw(a->path(0));
            }
          else
            {
              if (client->id != UUID::from_raw(a->path(0)))
                {
                  TRACE_MSG("Illegal change of ID ");
                }
            }

          string *c = a->add_path();
          *c = myid.raw();
          forward_message(link, packet);
        }
      else
        {
          TRACE_MSG("Unknown client");
        }

    }
  TRACE_EXIT();
}


void
Router::process_signoff(Link::Ptr link, PacketIn::Ptr packet)
{
  TRACE_ENTER("Router::process_signoff");
    
  boost::shared_ptr<proto::Signoff> a = boost::dynamic_pointer_cast<proto::Signoff>(packet->message);

  if (a)
    {
      google::protobuf::RepeatedPtrField<string> signedoff_ids = a->id();
      
      TRACE_MSG("Source " << packet->source.str());
      for (google::protobuf::RepeatedPtrField<string>::iterator i = signedoff_ids.begin(); i != signedoff_ids.end(); i++)
        {
          const UUID &id = UUID::from_raw(*i);
          TRACE_MSG("Signoff " << id);
          Client::Ptr client = find_client(id);
          if (client)
            {
              ViaLink::Ptr via = boost::dynamic_pointer_cast<ViaLink>(client->link);
              if (via)
                {
                  clients.remove(client);
                }
              else
                {
                  TRACE_MSG("Not removing non-via link" << client.get());
                }
            }
          else
            {
              TRACE_MSG("Signoff for unknown client");
            }
        }
    }
  TRACE_EXIT();
}



#ifdef HAVE_TESTS

void
Router::start_announce()
{
  TRACE_ENTER("Router::start_announce");
  announce->start();
  TRACE_EXIT();
}

UUID
Router::get_id() const
{
  return myid;
}

std::list<UUID>
Router::get_clients() const
{
  std::list<UUID> ids;
  for (ClientCIter it = clients.begin(); it != clients.end(); it++)
    {
      ids.push_back((*it)->id);
    }
  return ids;
}


std::list<UUID>
Router::get_direct_clients() const
{
  std::list<UUID> ids;
  for (ClientCIter it = clients.begin(); it != clients.end(); it++)
    {
      ViaLink::Ptr via = boost::dynamic_pointer_cast<ViaLink>((*it)->link);
      if (!via)
        {
          ids.push_back((*it)->id);
        }
    }
  return ids;
}

int
Router::get_cycle_failures() const
{
  return cycle_failures;
}

void
Router::disconnect(UUID id)
{
  TRACE_ENTER("Router::disconnect");
  Client::Ptr client = find_client(id);
  if (client)
    {
      DirectLink::Ptr link = boost::dynamic_pointer_cast<DirectLink>(client->link);
      if (link)
        {
          link->close();
        }
      TRACE_MSG("closed");
    }
  TRACE_EXIT();
}
#endif


