//
// Copyright (C) 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef NETLINKNETWORKINTERFACEMONITOR_HH
#define NETLINKNETWORKINTERFACEMONITOR_HH

#include <string>
#include <list>
#include <map>

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "NetworkInterfaceMonitorBase.hh"
#include "SocketDriver.hh"

using namespace workrave;

//! Listen socket implementation using GIO
class NetlinkNetworkInterfaceMonitor : public NetworkInterfaceMonitorBase
{
public:
  NetlinkNetworkInterfaceMonitor();
  virtual ~NetlinkNetworkInterfaceMonitor();

  virtual bool init();

private:
  enum State { STATE_NONE, STATE_GETLINK, STATE_GETADDR, STATE_DONE };

  class NetworkAdapter;
  
  class NetworkInterface
  {
  public:
    NetworkInterface() : adapter(NULL), address(NULL), valid(false) {}
    ~NetworkInterface()
    {
      if (address != NULL)
        {
          g_object_unref(address);
        }
    }
    
    bool operator==(const NetworkInterface &other)
    {
      return g_inet_address_equal(address, other.address);
    }

    NetworkAdapter *adapter;
    GInetAddress *address;
    bool valid;
  };

  typedef std::map<GSocketFamily, NetworkInterface> NetworkInterfacesByFamilyList;

  class NetworkAdapter
  {
  public:
    NetworkAdapter() : valid(false) {}
    
    std::string name;
    NetworkInterfacesByFamilyList interfaces;
    bool valid;
  };

  typedef std::map<guint, NetworkAdapter> NetworkAdapterList;
  typedef NetworkAdapterList::const_iterator NetworkAdapterCIter;

private:
  void send_request(guint netlink_message, guint flags);
  void process_reply();
  void process_newlink(struct nlmsghdr *);
  void process_dellink(struct nlmsghdr *);
  void process_addr(struct nlmsghdr *, bool);
  void next_state();

  void verify_network_adapter(NetworkAdapter &);
  void verify_network_interface(NetworkInterface &);

  GInetAddress *create_inet_address(GSocketFamily famility, struct rtattr *rta);
                                    
  static gboolean static_netlink_data(GSocket *socket, GIOCondition condition, gpointer user_data);
  
private:
  NetworkAdapterList adapters;

  GSocket *netlink_socket;
  GSource *netlink_source;

  int sequence_nr;
  State state;
  int state_sequence_nr;
};

#endif
