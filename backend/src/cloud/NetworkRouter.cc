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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "debug.hh"

#include "NetworkRouter.hh"

#include "workrave.pb.h"
#include "cloud.pb.h"

using namespace std;

//! Constructs a new network router
NetworkRouter::NetworkRouter(const WRID &my_id)
  : my_id(my_id)
{
  TRACE_ENTER_MSG("NetworkRouter::NetworkRouter", my_id.str());
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
      cloud::Break b;
      b.set_break_id(1);
      b.set_break_event(cloud::Break_BreakEvent_BREAK_EVENT_NONE);
      send_message(b, NetworkClient::SCOPE_DIRECT);

      once = true;
    }
  
  TRACE_EXIT();
}

void
NetworkRouter::connect(const string &host, int port)
{
  TRACE_ENTER_MSG("NetworkRouter::connect", host << " " << port << " (" << my_id.str() << ")");
  direct_links->connect(host, port);
  TRACE_EXIT();
}

void
NetworkRouter::send_message(google::protobuf::Message &message, NetworkClient::Scope scope)
{
  TRACE_ENTER_MSG("NetworkRouter::send_message", scope << " (" << my_id.str() << ")");
  string str = marshall_message(message);
  
  if ((scope & NetworkClient::SCOPE_DIRECT) == NetworkClient::SCOPE_DIRECT)
    {
      direct_links->send_message(str);
    }
  
  if ((scope & NetworkClient::SCOPE_MULTICAST) == NetworkClient::SCOPE_MULTICAST)
    {
      announce->send_message(str);
    }
      
  TRACE_EXIT();
}

void
NetworkRouter::send_message_to(google::protobuf::Message &message, NetworkClient::Ptr client)
{
  TRACE_ENTER_MSG("NetworkRouter::send_message_to", "(" << my_id.str() << ")");
  string str = marshall_message(message);
  
  direct_links->send_message_to(str, client);
      
  TRACE_EXIT();
}

void
NetworkRouter::send_message_except(google::protobuf::Message &message, NetworkClient::Ptr client)
{
  TRACE_ENTER_MSG("NetworkRouter::send_message_except", "(" << my_id.str() << ")");
  string str = marshall_message(message);
  
  direct_links->send_message_except(str, client);
      
  TRACE_EXIT();
}

void
NetworkRouter::on_data(gsize size, const gchar *data, NetworkClient::Ptr client)
{
  TRACE_ENTER_MSG("NetworkRouter::on_data", " (" << my_id.str() << ")");
  boost::shared_ptr<google::protobuf::Message> message;
  workrave::Header header;
  
  bool ok = unmarshall_message(size, data, message, header);
  if (ok)
    {
      TRACE_MSG(client);
      TRACE_MSG(client->address->str());
      TRACE_MSG(header.DebugString());
      TRACE_MSG(message->DebugString());

      // TODO: only when authenticated.
      send_message_except(*message.get(), client);
    }
  TRACE_EXIT();
}


const string
NetworkRouter::marshall_message(google::protobuf::Message &message)
{
  string ret;
  
  const google::protobuf::Descriptor *ext = message.GetDescriptor();
  if (ext != NULL)
    {
      const google::protobuf::FieldDescriptor *fd = ext->extension(0);
      if (fd != NULL)
        {
          int num = fd->number();

          workrave::Header header;
          header.set_source(my_id.str());
          header.set_payload(num);
          header.set_domain(1);

          stringstream s(ios_base::in | ios_base::out | ios_base::binary);

          boost::shared_ptr<google::protobuf::io::OstreamOutputStream> output(new google::protobuf::io::OstreamOutputStream(&s));
          boost::shared_ptr<google::protobuf::io::CodedOutputStream> coded(new google::protobuf::io::CodedOutputStream(output.get()));

          coded->WriteVarint32(header.ByteSize());
          coded->WriteVarint32(message.ByteSize());
      
          header.SerializeToCodedStream(coded.get());
          message.SerializeToCodedStream(coded.get());

          coded.reset();
          output.reset();
      
          ret = s.str();
        }
    }
  
  return ret;
}


bool
NetworkRouter::unmarshall_message(gsize size, const gchar *data,
                                  boost::shared_ptr<google::protobuf::Message> &message,
                                  workrave::Header &header)
{
  TRACE_ENTER_MSG("NetworkRouter::unmarshall_message", " (" << my_id.str() << ")");
  bool result = false;
  
  try
    {
      stringstream s(std::string(data, size), ios_base::in | ios_base::out | ios_base::binary);

      boost::shared_ptr<google::protobuf::io::IstreamInputStream> input(new google::protobuf::io::IstreamInputStream(&s));
      boost::shared_ptr<google::protobuf::io::CodedInputStream> coded(new google::protobuf::io::CodedInputStream(input.get()));

      bool header_ok = true;;
      google::protobuf::uint32 header_size;
      google::protobuf::uint32 msg_size;
      const google::protobuf::Descriptor *base = NULL;
      const google::protobuf::FieldDescriptor *field = NULL;
      const google::protobuf::Descriptor *ext = NULL;

      header_ok = header_ok && coded->ReadVarint32(&header_size);
      header_ok = header_ok && coded->ReadVarint32(&msg_size);

      if (header_ok)
        {
          coded.reset();
          header_ok = header.ParseFromBoundedZeroCopyStream(input.get(), header_size);
        }
      
      if (header_ok && size >= header_size + msg_size)
        {
          const string domain_name = get_namespace_of_domain(header.domain()) + ".Domain";
          base = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(domain_name);
        }

      if (base != NULL)
        {
          field = google::protobuf::DescriptorPool::generated_pool()->FindExtensionByNumber(base, header.payload());
        }
      
      if (field != NULL)
        {
          ext = field->extension_scope();
        }
      
      if (ext != NULL)
        {
          google::protobuf::Message *msg = google::protobuf::MessageFactory::generated_factory()->GetPrototype(ext)->New();
          
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
  TRACE_ENTER_MSG("NetworkRouter::on_client_changed", "(" << my_id.str() << ")");
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

// //! Connect to a remote workrave
// void
// NetworkRouter::connect(string url)
// {
//   TRACE_ENTER_MSG("NetworkRouter::connect", url);
//   std::string::size_type pos = url.find("://");
//   std::string hostport;
      
//   if (pos == std::string::npos)
//     {
//       hostport = url;
//     }
//   else
//     {
//       hostport = url.substr(pos + 3);
//     }
  
//   pos = hostport.find(":");
//   std::string host;
//   std::string port = "0";
  
//   if (pos == std::string::npos)
//     {
//       host = hostport;
//     }
//   else
//     {
//       host = hostport.substr(0, pos);
//       port = hostport.substr(pos + 1);
//     }
  
//   string linkid;
//   connect(host.c_str(), atoi(port.c_str()), linkid);

//   TRACE_EXIT();
// }


// //! Disconnect the all links
// void
// NetworkRouter::disconnect_all()
// {
//   TRACE_ENTER("NetworkRouter::disconnect_all");

//   NetworkIter i = links.begin();
//   while (i != links.end())
//     {
//       NetworkIter next = i;
//       next++;

//       NetworkInfo &info = i->second;
//       delete info.link;

//       link_down(info.link->get_link_id());

//       i = next;
//     }

//   TRACE_EXIT();
// }


// bool
// NetworkRouter::listen(int port)
// {
//   TcpNetworkServer *server = new TcpNetworkServer(port, this);
//   default_link = server;

//   return server->init();
// }


// void
// NetworkRouter::stop_listening()
// {
//   default_link->terminate();
//   delete default_link;
//   default_link = NULL;

// }



// bool
// NetworkRouter::disconnect(const string &link_id)
// {
//   TRACE_ENTER_MSG("NetworkRouter::disconnect", link_id);
//   bool ret = false;
//   WRID id(link_id);

//   NetworkIter it = links.find(id);
//   if (it != links.end())
//     {
//       ret = true;

//       NetworkInfo &info = it->second;

//       delete info.link;

//       link_down(id);
//     }

//   TRACE_EXIT();
//   return ret;
// }


// bool
// NetworkRouter::send_event(NetworkEvent *event)
// {
//   TRACE_ENTER_MSG("NetworkRouter::send_event", event->str());
//   bool rc = true;

//   try
//     {
//       event->set_source(my_id);

//       // Fire event internally. FIXME: why removed?
//       // fire_event(event);

//       // Fire to remote workraves.
//       for (NetworkIter i = links.begin(); i != links.end(); i++)
//         {
//           NetworkInfo &info = i->second;

//           if (info.state == LINK_UP)
//             {
//               TRACE_MSG("send to " << info.id.str());
//               info.link->send_event(event);
//             }
//         }
//     }
//   catch(Exception &e)
//     {
//       TRACE_MSG("Exception " << e.details());
//       rc = false;
//     }

//   TRACE_EXIT();
//   return rc;
// }


// bool
// NetworkRouter::send_event_to_link(const WRID &link_id, NetworkEvent *event)
// {
//   TRACE_ENTER_MSG("NetworkRouter::send_event_to_link", link_id.str() << " " << event->str());
//   bool rc = true;

//   try
//     {
//       event->set_source(my_id);

//       // Fire to remote workraves.
//       NetworkIter it = links.find(link_id);
//       if (it != links.end())
//         {
//           NetworkInfo &info = it->second;

//           if (info.state == LINK_UP)
//             {
//               TRACE_MSG("send to " << info.id.str());
//               info.link->send_event(event);
//             }
//         }
//     }
//   catch(Exception &e)
//     {
//       TRACE_MSG("Exception " << e.details());
//       rc = false;
//     }

//   TRACE_EXIT();
//   return rc;
// }


// bool
// NetworkRouter::send_event_locally(NetworkEvent *event)
// {
//   TRACE_ENTER_MSG("NetworkRouter::send_event_locally", event->str());
//   bool rc = true;

//   try
//     {
//       event->set_source(my_id);

//       // Fire event internally.
//       fire_event(event);
//     }
//   catch(Exception &e)
//     {
//       TRACE_MSG("Exception " << e.details());
//       rc = false;
//     }

//   TRACE_EXIT();
//   return rc;
// }


// void
// NetworkRouter::new_link(INetwork *link)
// {
//   TRACE_ENTER("NetworkRouter::new_link");

//   WRID id = link->get_link_id();
//   NetworkInfo info(id, link);

//   TRACE_MSG("id " << id.str());

//   info.state = LINK_DOWN;

//   links[id] = info;

//   link->set_link_listener(this);

//   TRACE_EXIT();
// }


// void
// NetworkRouter::event_received(const WRID &id, NetworkEvent *event)
// {
//   TRACE_ENTER_MSG("NetworkRouter::event_received", id.str()
//                   << event->str());

//   TRACE_MSG("event " << event->get_source().str());
//   TRACE_MSG("me " << my_id.str());

//   const WRID &event_id = event->get_source();
//   if (my_id != event_id)
//     {
//       fire_event(event);

//       NetworkIter it = links.find(id);
//       if (it != links.end() && it->second.state == LINK_UP)
//         {
//           for (NetworkIter i = links.begin(); i != links.end(); i++)
//             {
//               NetworkInfo &info = i->second;

//               if (info.id != id && info.state == LINK_UP)
//                 {
//                   TRACE_MSG("forward to " << info.id.str());
//                   info.link->send_event(event);
//                 }
//             }
//         }
//       else
//         {
//           TRACE_MSG("link not found or not authenticated");
//         }
//     }
//   else
//     {
//       TRACE_MSG("FIXME: handle cycle");
//     }
//   TRACE_EXIT();
// }


// void
// NetworkRouter::link_down(const WRID &id)
// {
//   TRACE_ENTER_MSG("NetworkRouter::link_down", id.str());

//   NetworkIter it = links.find(id);
//   if (it != links.end())
//     {
//       NetworkInfo &info = it->second;

//       info.state = LINK_GARBAGE;
//       // FIXME: cleanup

//       NetworkStateNetworkEvent event(id, NetworkStateNetworkEvent::LINKSTATE_DOWN);
//       fire_event(&event);
//     }

//   links.erase(id);
//   TRACE_EXIT()
// }


// void
// NetworkRouter::link_up(const WRID &id)
// {
//   TRACE_ENTER_MSG("NetworkRouter::link_up", id.str());

//   NetworkIter it = links.find(id);
//   if (it != links.end())
//     {
//       NetworkInfo &info = it->second;

//       info.state = LINK_UP;

//       NetworkStateNetworkEvent event(id, NetworkStateNetworkEvent::LINKSTATE_UP);
//       fire_event(&event);
//     }

//   TRACE_EXIT()
// }


// //! Subscribe to the specified link event.
// bool
// NetworkRouter::subscribe(string eventid, INetworkEventListener *listener)
// {
//   bool ret = true;

//   ListenerIter i = listeners.begin();
//   while (ret && i != listeners.end())
//     {
//       if (eventid == i->first && listener == i->second)
//         {
//           // Already added. Skip
//           ret = false;
//         }

//       i++;
//     }

//   if (ret)
//     {
//       // not found -> add
//       listeners.push_back(make_pair(eventid, listener));
//     }

//   return ret;
// }


// //! Unsubscribe from the specified link event
// bool
// NetworkRouter::unsubscribe(string eventid, INetworkEventListener *listener)
// {
//   bool ret = false;

//   ListenerIter i = listeners.begin();
//   while (i != listeners.end())
//     {
//       if (eventid == i->first && listener == i->second)
//         {
//           // Found. Remove
//           i = listeners.erase(i);
//           ret = true;
//         }
//       else
//         {
//           i++;
//         }
//     }

//   return ret;
// }


// //! Fires a link event.
// void
// NetworkRouter::fire_event(NetworkEvent *event)
// {
//   TRACE_ENTER("NetworkEvent::fire_event");
//   string eventid = event->get_eventid();

//   ListenerIter i = listeners.begin();
//   while (i != listeners.end())
//     {
//       if (eventid == i->first)
//         {
//           INetworkEventListener *l = i->second;
//           if (l != NULL)
//             {
//               l->event_received(event);
//             }
//         }

//       i++;
//     }
//   TRACE_EXIT();
// }

