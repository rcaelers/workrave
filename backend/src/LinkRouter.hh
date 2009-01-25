// LinkRouter.hh --- Networking link server
//
// Copyright (C) 2007, 2008, 2009 Rob Caelers <robc@krandor.nl>
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

#ifndef LINKROUTER_HH
#define LINKROUTER_HH

#include <list>
#include <map>
#include <string>

#include "ILinkServerListener.hh"
#include "ILinkListener.hh"
#include "ILink.hh"
#include "WRID.hh"

using namespace workrave;

namespace workrave {
  class ILinkEventListener;
}

class ILinkServer;

class LinkRouter
  : public ILinkServerListener,
    public ILinkListener
{
public:
  LinkRouter(const WRID &my_id);
  virtual ~LinkRouter();

  void init();
  bool listen(int port);
  void stop_listening();
  bool connect(const std::string &host, int port, std::string &link_id);
  bool disconnect(const std::string &link_id);

  // INetwork
  virtual void connect(std::string url);
  virtual void disconnect_all();
  virtual bool send_event(LinkEvent *event);
  virtual bool subscribe(const std::string eventid, ILinkEventListener *listener);
  virtual bool unsubscribe(const std::string eventid, ILinkEventListener *listener);

  // Internal
  virtual bool send_event_to_link(const WRID &link_id, LinkEvent *event);
  virtual bool send_event_locally(LinkEvent *event);

private:
  enum LinkState {
    LINK_DOWN = 1,
    LINK_UP,
    LINK_GARBAGE
  };

  struct LinkInfo
  {
    LinkInfo()
      : link(NULL), state(LINK_DOWN)
    {
    }

    LinkInfo(WRID &id, ILink *link)
      : id(id), link(link), state(LINK_DOWN)
    {
    }

    WRID id;
    ILink *link;
    LinkState state;
  };

  // ILinkServerListener
  virtual void new_link(ILink *link);

  // ILinkListener
  virtual void event_received(const WRID &id, LinkEvent *event);
  virtual void link_up(const WRID &id);
  virtual void link_down(const WRID &id);

  // Internal
  void fire_event(LinkEvent *event);

  // Known links
  typedef std::map<WRID, LinkInfo> Links;
  typedef std::map<WRID, LinkInfo>::iterator LinkIter;
  typedef std::map<WRID, LinkInfo>::const_iterator LinkCIter;

  // Event subscriptions
  typedef std::list<std::pair<std::string, ILinkEventListener *> > Listeners;
  typedef std::list<std::pair<std::string, ILinkEventListener *> >::iterator ListenerIter;
  typedef std::list<std::pair<std::string, ILinkEventListener *> >::const_iterator ListenerCIter;

private:
  //! My ID
  const WRID my_id;

  //! Default server
  ILinkServer *default_link;

  //! Known links
  Links links;

  //! Link event listeners.
  Listeners listeners;
};


#endif // LINKROUTER_HH
