// LinkRouter.cc
//
// Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.nl>
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
#include <assert.h>
#include <string.h>

#include <gnet.h>

#include "LinkRouter.hh"

#include "ILinkEventListener.hh"

#include "TcpLinkServer.hh"
#include "TcpLink.hh"
#include "LinkEvent.hh"
#include "LinkStateLinkEvent.hh"

using namespace std;

//! Constructs a new link router
LinkRouter::LinkRouter(const UUID &my_id)
  : my_id(my_id), default_link(NULL)
{
}


//! Destructs the link router.
LinkRouter::~LinkRouter()
{
  delete default_link;

  for (LinkIter i = links.begin(); i != links.end(); i++)
    {
      delete i->second.link;
    }

  links.clear();
}


//! Initializes the router.
void
LinkRouter::init()
{
  TRACE_ENTER("LinkRouter::init");
  TRACE_EXIT();
}


//! Connect to a remote workrave
void
LinkRouter::connect(string uri)
{
  TRACE_ENTER_MSG("LinkRouter::connect", uri);
  GURI *guri = gnet_uri_new(uri.c_str());

  if (guri != NULL)
    {
      if (strcmp(guri->scheme, "tcp") == 0)
        {
          string linkid;
          connect(guri->hostname, guri->port, linkid);
        }

      gnet_uri_delete(guri);
    }
  TRACE_EXIT();
}


//! Disconnect the all links
void
LinkRouter::disconnect_all()
{
  TRACE_ENTER("LinkRouter::disconnect_all");

  LinkIter i = links.begin();
  while (i != links.end())
    {
      LinkIter next = i;
      next++;

      LinkInfo &info = i->second;
      delete info.link;

      link_down(info.link->get_link_id());

      i = next;
    }

  TRACE_EXIT();
}


bool
LinkRouter::listen(int port)
{
  TcpLinkServer *server = new TcpLinkServer(port, this);
  default_link = server;

  return server->init();
}


void
LinkRouter::stop_listening()
{
  default_link->terminate();
  delete default_link;
  default_link = NULL;

}

bool
LinkRouter::connect(const string &host, int port, string &link_id)
{
  TRACE_ENTER_MSG("LinkRouter::connect", host << " " << port);

  TcpLink *link = new TcpLink();
  UUID id = link->get_link_id();

  TRACE_MSG(links.size());

  LinkInfo info(id, link);

  info.state = LINK_DOWN;

  links[id] = info;

  link->set_link_listener(this);

  bool b = link->connect(host, port);
  if (b)
    {
      link_id = id.str();
    }

  TRACE_EXIT();

  return b;
}


bool
LinkRouter::disconnect(const string &link_id)
{
  TRACE_ENTER_MSG("LinkRouter::disconnect", link_id);
  bool ret = false;
  UUID id(link_id);

  LinkIter it = links.find(id);
  if (it != links.end())
    {
      ret = true;

      LinkInfo &info = it->second;

      delete info.link;

      link_down(id);
    }

  TRACE_EXIT();
  return ret;
}


bool
LinkRouter::send_event(LinkEvent *event)
{
  TRACE_ENTER_MSG("LinkRouter::send_event", event->str());
  bool rc = true;

  try
    {
      event->set_source(my_id);

      // Fire event internally. FIXME: why removed?
      // fire_event(event);

      // Fire to remote workraves.
      for (LinkIter i = links.begin(); i != links.end(); i++)
        {
          LinkInfo &info = i->second;

          if (info.state == LINK_UP)
            {
              TRACE_MSG("send to " << info.id.str());
              info.link->send_event(event);
            }
        }
    }
  catch(Exception &e)
    {
      TRACE_MSG("Exception " << e.details());
      rc = false;
    }

  TRACE_EXIT();
  return rc;
}


bool
LinkRouter::send_event_to_link(const UUID &link_id, LinkEvent *event)
{
  TRACE_ENTER_MSG("LinkRouter::send_event_to_link", link_id.str() << " " << event->str());
  bool rc = true;

  try
    {
      event->set_source(my_id);

      // Fire to remote workraves.
      LinkIter it = links.find(link_id);
      if (it != links.end())
        {
          LinkInfo &info = it->second;

          if (info.state == LINK_UP)
            {
              TRACE_MSG("send to " << info.id.str());
              info.link->send_event(event);
            }
        }
    }
  catch(Exception &e)
    {
      TRACE_MSG("Exception " << e.details());
      rc = false;
    }

  TRACE_EXIT();
  return rc;
}


bool
LinkRouter::send_event_locally(LinkEvent *event)
{
  TRACE_ENTER_MSG("LinkRouter::send_event_locally", event->str());
  bool rc = true;

  try
    {
      event->set_source(my_id);

      // Fire event internally.
      fire_event(event);
    }
  catch(Exception &e)
    {
      TRACE_MSG("Exception " << e.details());
      rc = false;
    }

  TRACE_EXIT();
  return rc;
}


void
LinkRouter::new_link(ILink *link)
{
  TRACE_ENTER("LinkRouter::new_link");

  UUID id = link->get_link_id();
  LinkInfo info(id, link);

  TRACE_MSG("id " << id.str());

  info.state = LINK_DOWN;

  links[id] = info;

  link->set_link_listener(this);

  TRACE_EXIT();
}


void
LinkRouter::event_received(const UUID &id, LinkEvent *event)
{
  TRACE_ENTER_MSG("LinkRouter::event_received", id.str()
                  << event->str());

  TRACE_MSG("event " << event->get_source().str());
  TRACE_MSG("me " << my_id.str());

  const UUID &event_id = event->get_source();
  if (my_id != event_id)
    {
      fire_event(event);

      LinkIter it = links.find(id);
      if (it != links.end() && it->second.state == LINK_UP)
        {
          for (LinkIter i = links.begin(); i != links.end(); i++)
            {
              LinkInfo &info = i->second;

              if (info.id != id && info.state == LINK_UP)
                {
                  TRACE_MSG("forward to " << info.id.str());
                  info.link->send_event(event);
                }
            }
        }
      else
        {
          TRACE_MSG("link not found or not authenticated");
        }
    }
  else
    {
      TRACE_MSG("FIXME: handle cycle");
    }
  TRACE_EXIT();
}


void
LinkRouter::link_down(const UUID &id)
{
  TRACE_ENTER_MSG("LinkRouter::link_down", id.str());

  LinkIter it = links.find(id);
  if (it != links.end())
    {
      LinkInfo &info = it->second;

      info.state = LINK_GARBAGE;
      // FIXME: cleanup

      LinkStateLinkEvent event(id, LinkStateLinkEvent::LINKSTATE_DOWN);
      fire_event(&event);
    }

  links.erase(id);
  TRACE_EXIT()
}


void
LinkRouter::link_up(const UUID &id)
{
  TRACE_ENTER_MSG("LinkRouter::link_up", id.str());

  LinkIter it = links.find(id);
  if (it != links.end())
    {
      LinkInfo &info = it->second;

      info.state = LINK_UP;

      LinkStateLinkEvent event(id, LinkStateLinkEvent::LINKSTATE_UP);
      fire_event(&event);
    }

  TRACE_EXIT()
}


//! Subscribe to the specified link event.
bool
LinkRouter::subscribe(string eventid, ILinkEventListener *listener)
{
  bool ret = true;

  ListenerIter i = listeners.begin();
  while (ret && i != listeners.end())
    {
      if (eventid == i->first && listener == i->second)
        {
          // Already added. Skip
          ret = false;
        }

      i++;
    }

  if (ret)
    {
      // not found -> add
      listeners.push_back(make_pair(eventid, listener));
    }

  return ret;
}


//! Unsubscribe from the specified link event
bool
LinkRouter::unsubscribe(string eventid, ILinkEventListener *listener)
{
  bool ret = false;

  ListenerIter i = listeners.begin();
  while (i != listeners.end())
    {
      if (eventid == i->first && listener == i->second)
        {
          // Found. Remove
          i = listeners.erase(i);
          ret = true;
        }
      else
        {
          i++;
        }
    }

  return ret;
}


//! Fires a link event.
void
LinkRouter::fire_event(LinkEvent *event)
{
  TRACE_ENTER("LinkEvent::fire_event");
  string eventid = event->get_eventid();

  ListenerIter i = listeners.begin();
  while (i != listeners.end())
    {
      if (eventid == i->first)
        {
          ILinkEventListener *l = i->second;
          if (l != NULL)
            {
              l->event_received(event);
            }
        }

      i++;
    }
  TRACE_EXIT();
}
