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

Announce::Ptr
Announce::create(Marshaller::Ptr marshaller)
{
  return Announce::Ptr(new Announce(marshaller));
}


Announce::Announce(Marshaller::Ptr marshaller) : marshaller(marshaller)
{
  TRACE_ENTER("Announce::Announce");
  multicast_server = MulticastSocketServer::create();

  state = ANNOUNCE_STATE_IDLE;

  TRACE_EXIT();
}


Announce::~Announce()
{
  TRACE_ENTER("Announce::~Announce");
  TRACE_EXIT();
}


void
Announce::init(int port, UUID &id)
{
  TRACE_ENTER("Announce::init");
  myid = id;
  
  multicast_server->init("239.160.181.73", "ff15::1:145", port);
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
    case ANNOUNCE_STATE_IDLE:
      break;

    case ANNOUNCE_STATE_WAIT_FOR_ANNOUNCE:
      if (TimeSource::get_monotonic_time() > wait_until_time)
        {
          goto_state(ANNOUNCE_STATE_ANNOUNCING);
        }
      break;
      
    case ANNOUNCE_STATE_ANNOUNCING:
      if (announce_left > 0)
        {
          announce_left--;
          send_announce();
        }
      else
        {
          goto_state(ANNOUNCE_STATE_CONNECTING);
        }
      break;
      
    case ANNOUNCE_STATE_CONNECTING:
      break;
    }
  TRACE_EXIT();
}


void
Announce::start()
{
  TRACE_ENTER("Announce::start");
  goto_state(ANNOUNCE_STATE_WAIT_FOR_ANNOUNCE);

  GSource *source = g_timeout_source_new(500);
  g_source_set_callback(source, static_on_timer, this, NULL);
  g_source_attach(source, g_main_context_get_thread_default());
  
  TRACE_EXIT();
}


void
Announce::goto_state(AnnounceState new_state)
{
  switch(new_state)
    {
    case ANNOUNCE_STATE_IDLE:
      break;
              
    case ANNOUNCE_STATE_WAIT_FOR_ANNOUNCE:
      wait_until_time = TimeSource::get_monotonic_time() + 5 * TimeSource::USEC_PER_SEC;
      break;

    case ANNOUNCE_STATE_ANNOUNCING:
      announce_left = 5;
      wait_until_time = TimeSource::get_monotonic_time() + TimeSource::USEC_PER_SEC;
      break;

    case ANNOUNCE_STATE_CONNECTING:
      break;
    }
  state = new_state;
}


void
Announce::send_announce()
{
  TRACE_ENTER("Announce::send_announce");

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
Announce::process_announce(Link::Ptr link, PacketIn::Ptr packet)
{
  TRACE_ENTER("Router::process_announce");
  
  // boost::shared_ptr<proto::Announce> a = boost::dynamic_pointer_cast<proto::Announce>(packet->message);

  // if (a)
  //   {
  //     //google::protobuf::RepeatedPtrField<string> known = a->known();

  //     TRACE_MSG("Source " << packet->source.str());
  //     // for (google::protobuf::RepeatedPtrField<string>::iterator i = path.begin(); i != path.end(); i++)
  //     //   {
  //     //     TRACE_MSG("Known " << UUID::from_raw(*i).str());
  //     //   }

  //     switch(state)
  //       {
  //       case ANNOUNCE_STATE_IDLE:
  //         break;
              
  //       case ANNOUNCE_STATE_WAIT_FOR_ANNOUNCE:
  //         goto_state(ANNOUNCE_STATE_WAIT_FOR_ANNOUNCE);
  //         break;

  //       case ANNOUNCE_STATE_ANNOUNCING:
  //         if (packet->source > myid)
  //           {
  //             goto_state(ANNOUNCE_STATE_WAIT_FOR_ANNOUNCE);
  //             break;
  //           }
  //         else
  //           {
  //             if (accounce_left < 2 )
  //               {
  //                 accounce_left = 2;
  //               }
  //           }
  //       }
  TRACE_EXIT();
}


//!
void
Announce::on_data(gsize size, const gchar *data, NetworkAddress::Ptr na)
{
  TRACE_ENTER("Router::on_multicast_data");

  EphemeralLink::Ptr link = EphemeralLink::create();
  link->state = Link::CONNECTION_STATE_CONNECTED;
  link->address = na;

  PacketIn::Ptr packet = marshaller->unmarshall(size, data);
  if (packet)
    {
      bool handled = false;
      
      TRACE_MSG("Source " << packet->source.str());

      if (packet->authentic)
        {
          if (packet->header->domain() == 0 && packet->header->payload() == proto::Alive::kTypeFieldNumber)
            {
              process_announce(link, packet);
              handled = true;
            }
        }

      if (!handled)
        {
          data_signal(link, packet);
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
 
