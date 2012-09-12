// Announce.cc
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

#include "Announce.hh"

#include "utils/TimeSource.hh"


using namespace std;
using namespace workrave::utils;
using namespace workrave::cloud;

Announce::Ptr
Announce::create(IRouter::Ptr router, Marshaller::Ptr marshaller)
{
  return Announce::Ptr(new Announce(router, marshaller));
}


Announce::Announce(IRouter::Ptr router, Marshaller::Ptr marshaller) : router(router), marshaller(marshaller)
{
  TRACE_ENTER("Announce::Announce");
  multicast_server = MulticastSocketServer::create();

  goto_monitoring();

  TRACE_EXIT();
}


Announce::~Announce()
{
  TRACE_ENTER("Announce::~Announce");
  TRACE_EXIT();
}


void
Announce::init(int announce_port, UUID &id, int direct_link_port)
{
  TRACE_ENTER("Announce::init");
  this->myid = id;
  this->direct_link_port = direct_link_port;
  
  multicast_server->init("239.160.181.73", "ff15::1:145", announce_port);
  multicast_server->signal_data().connect(boost::bind(&Announce::on_data, this, _1, _2, _3));
  
  TRACE_EXIT();
}


void
Announce::terminate()
{
  TRACE_ENTER("Announce::terminate");
  TRACE_EXIT();
}


void
Announce::heartbeat()
{
  TRACE_ENTER("Announce::heartbeat");

  switch(state)
    {
    case ANNOUNCE_STATE_MONITORING:
      TRACE_MSG("state = monitor");
      break;

    case ANNOUNCE_STATE_HOLD:
      TRACE_MSG("state = hold");
      if (TimeSource::get_monotonic_time_usec() > wait_until_time)
        {
          TRACE_MSG("resume monitoring");
          goto_monitoring();
        }
      break;

    case ANNOUNCE_STATE_DISCOVER:
      TRACE_MSG("state = discover");
      if (TimeSource::get_monotonic_time_usec() > wait_until_time)
        {
          TRACE_MSG("discover left = " << announce_left);
          if (announce_left > 0)
            {
              TRACE_MSG("sending discover");
              announce_left--;
              send_discover();
            }
          else if (announce_left == 0)
            {
              TRACE_MSG("no more left -> build");
              announce_left--;
              goto_build();
            }
        }
      break;
      
    case ANNOUNCE_STATE_BUILD: 
      TRACE_MSG("state = build");
      if (TimeSource::get_monotonic_time_usec() > wait_until_time)
        {
          TRACE_MSG("building");
          connect_to_clients();
          goto_monitoring();
        }
      break;
    }
  TRACE_EXIT();
}


void
Announce::start()
{
  TRACE_ENTER("Announce::start");
  goto_discover(false);

  GSource *source = g_timeout_source_new(500);
  g_source_set_callback(source, static_on_timer, this, NULL);
  g_source_attach(source, g_main_context_get_thread_default());
  
  TRACE_EXIT();
}


void
Announce::goto_monitoring()
{
  TRACE_ENTER("Announce::goto_monitoring");
  state = ANNOUNCE_STATE_MONITORING;
  wait_until_time = 0;
  TRACE_EXIT();
}


void
Announce::goto_hold()
{
  TRACE_ENTER("Announce::goto_monitoring");
  state = ANNOUNCE_STATE_HOLD;
  wait_until_time = TimeSource::get_monotonic_time_usec() + 30 * TimeSource::USEC_PER_SEC;
  TRACE_EXIT();
}


void
Announce::goto_discover(bool immediate)
{
  TRACE_ENTER_MSG("Announce::goto_discover", immediate);
  state = ANNOUNCE_STATE_DISCOVER;
  remote_clients.clear();
  announce_left = 5;

  if (immediate)
    {
      send_discover();
      wait_until_time = TimeSource::get_monotonic_time_usec();
    }
  else
    {
      // TODO: add random time.
      wait_until_time = TimeSource::get_monotonic_time_usec() + 2 * TimeSource::USEC_PER_SEC;
    }
  TRACE_EXIT();
}


void
Announce::goto_build()
{
  TRACE_ENTER("Announce::goto_build");
  state = ANNOUNCE_STATE_BUILD;
  wait_until_time = TimeSource::get_monotonic_time_usec() + TimeSource::USEC_PER_SEC;
  TRACE_EXIT();
}


void
Announce::send_discover()
{
  TRACE_ENTER("Announce::send_discover");

  boost::shared_ptr<proto::Discover> a(new proto::Discover());

  send_message(a, MessageParams::create());
  
  TRACE_EXIT();
}


void
Announce::send_routing_data()
{
  TRACE_ENTER("Announce::send_routing_data");

  boost::shared_ptr<proto::RoutingData> a(new proto::RoutingData());

  a->set_port(direct_link_port);
  
  list<workrave::cloud::ClientInfo> clients = router->get_client_infos();
  for (list<ClientInfo>::iterator i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &client = *i;
      proto::RoutingData::Route *r = a->add_route();

      TRACE_MSG("Route " << client.id.str());

      r->set_id(client.id.raw());
      if (client.id != client.via)
        {
          TRACE_MSG("Via " << client.via.str());
          r->set_via(client.via.raw());
        }
    }

  send_message(a, MessageParams::create());
  
  TRACE_EXIT();
}


void
Announce::send_message(const std::string &message)
{
  TRACE_ENTER("Announce::send_message");
  multicast_server->send(message.c_str(), message.length());
  TRACE_EXIT();
}


void
Announce::send_message(Message::Ptr message, MessageParams::Ptr params)
{
  PacketOut::Ptr packet = PacketOut::create(message);
  packet->sign = params->sign;
  packet->source = myid;
  
  string msg_str = marshaller->marshall(packet);
  send_message(msg_str);
}


void
Announce::process_discover(EphemeralLink::Ptr link, PacketIn::Ptr packet)
{
  TRACE_ENTER("Announce::process_discover");
  
  boost::shared_ptr<proto::Discover> a = boost::dynamic_pointer_cast<proto::Discover>(packet->message);

  if (a)
    {
      TRACE_MSG("Source " << packet->source.str());
      
      switch(state)
        {
        case ANNOUNCE_STATE_HOLD:
          TRACE_MSG("state = hold");
          send_routing_data();
          break;
            
        case ANNOUNCE_STATE_MONITORING:
          TRACE_MSG("state = monitoring");
          if (myid > packet->source)
            {
              TRACE_MSG("taking over.");
              goto_discover(true);
            }
          else
            {
              send_routing_data();
            }
          break;
              
        case ANNOUNCE_STATE_DISCOVER:
          TRACE_MSG("state = discovering");
          if (packet->source > myid)
            {
              TRACE_MSG("conflict -> hold");
              goto_hold();
            }
          break;
            
        case ANNOUNCE_STATE_BUILD:
          TRACE_MSG("state = build");
          break;
        }
    }
  TRACE_EXIT();
}


void
Announce::process_routing_data(EphemeralLink::Ptr link, PacketIn::Ptr packet)
{
  TRACE_ENTER("Announce::process_routing_data");
  
  boost::shared_ptr<proto::RoutingData> a = boost::dynamic_pointer_cast<proto::RoutingData>(packet->message);

  if (a)
    {
      UUID &source_id = packet->source;

      remote_clients.erase(source_id);
      RemoteClient &client = remote_clients[source_id];


      TRACE_MSG("Client " << link->address->str());
      
      client.address = link->address;
      client.port = a->port();
      
      google::protobuf::RepeatedPtrField<proto::RoutingData::Route> routes = a->route();
      for (google::protobuf::RepeatedPtrField<proto::RoutingData::Route>::iterator i = routes.begin(); i != routes.end(); i++)
        {
          proto::RoutingData::Route &route = *i;
          const UUID &id = UUID::from_raw(route.id());
          TRACE_MSG("Route " << id.str());

          if (route.has_via())
            {
              const UUID &via = UUID::from_raw(route.via());
              TRACE_MSG("Via " << via.str());
            }
          
          if (!route.has_via())
            {
              TRACE_MSG("New one direct");
              client.direct_connections.insert(id);
            }
          client.all_connections.insert(id);
        }
    }

  TRACE_EXIT();
}

//!
void
Announce::on_data(gsize size, const gchar *data, NetworkAddress::Ptr na)
{
  TRACE_ENTER_MSG("Announce::on_data", size << " " << na->str());

  EphemeralLink::Ptr link = EphemeralLink::create();
  link->state = Link::CONNECTION_STATE_CONNECTED;
  link->address = na;

  PacketIn::Ptr packet = marshaller->unmarshall(size, data);
  if (packet)
    {
      bool handled = false;
      UUID &source_id = packet->source;
      
      TRACE_MSG("Source " << source_id.str());
      TRACE_MSG("Me " << myid.str());

      if (myid == source_id)
        {
          TRACE_MSG("Own packet ");
          handled = true;
        }
      else if (packet->authentic)
        {
          TRACE_MSG("Authentic");
          if (packet->header->domain() == 0)
            {
              if (packet->header->payload() == proto::Discover::kTypeFieldNumber)
                {
                  process_discover(link, packet);
                  handled = true;
                }
              else if (packet->header->payload() == proto::RoutingData::kTypeFieldNumber)
                {
                  process_routing_data(link, packet);
                  handled = true;
                }
            }
        }

      if (!handled)
        {
          data_signal(link, packet);
        }
    }
  
  TRACE_EXIT();
}


void
Announce::connect_to_clients()
{
  TRACE_ENTER("Announce::connect_to_clients");
  set<UUID> known;
  
  list<workrave::cloud::ClientInfo> clients = router->get_client_infos();
  for (list<ClientInfo>::iterator i = clients.begin(); i != clients.end(); i++)
    {
      ClientInfo &client = *i;
      if (client.id != client.via)
        {
          TRACE_MSG("Known by me:" << myid.str());
          known.insert(client.id);
        }
    }

  for (RemoteClientMapIter i = remote_clients.begin(); i != remote_clients.end(); i++)
    {
      if (known.find(i->first) == known.end())
        {
          RemoteClient &remote = i->second;
          
          for (UUIDSet::iterator j = remote.all_connections.begin(); j != remote.all_connections.end(); j++)
            {
              TRACE_MSG("Known by " << i->first.str() << " : " << j->str());
              known.insert(*j);
            }

          TRACE_MSG("id " << i->first.str());
          TRACE_MSG("Connect " << remote.address->str() << " : " << remote.port);
          router->connect(remote.address, remote.port);
        }
    }

  TRACE_EXIT();
}


Announce::data_signal_type &
Announce::signal_data()
{
  return data_signal;
}


gboolean
Announce::static_on_timer(gpointer data)
{
  Announce *self = (Announce *)data;
  
  self->heartbeat();
  
  return G_SOURCE_CONTINUE;
}
 
