// NetlinkNetworkInterfaceMonitor.cc
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include "debug.hh"

//#include <gio/gunixcredentialsmessage.h>

#include "GIONetworkAddress.hh"
#include "NetlinkNetworkInterfaceMonitor.hh"

using namespace std;


//! Creates a netlink network interface monitor.
NetlinkNetworkInterfaceMonitor::NetlinkNetworkInterfaceMonitor() :
  netlink_socket(NULL),
  netlink_source(NULL),
  sequence_nr(0),
  state(STATE_NONE),
  state_sequence_nr(-1)
{
}


//! Destructs the netlink network interface monitor.
NetlinkNetworkInterfaceMonitor::~NetlinkNetworkInterfaceMonitor()
{
  if (netlink_socket != NULL)
    {
      g_object_unref(netlink_socket);
    }

  if (netlink_source != NULL)
    {
      g_source_destroy(netlink_source);
    }
}


bool
NetlinkNetworkInterfaceMonitor::init()
{
  TRACE_ENTER("NetlinkNetworkInterfaceMonitor::init");
  
  int fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
  if (fd == -1)
    {
      TRACE_RETURN("socket failed");
      return false;
    }

  struct sockaddr_nl sa;
  memset(&sa, 0, sizeof(sa));
  sa.nl_family = AF_NETLINK;
  sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;
  sa.nl_pid = 0; //getpid();
  sa.nl_pad = 0;
  
  int result = bind(fd, (struct sockaddr *) &sa, sizeof(sa));
  if (result == -1)
    {
      close(fd);
      TRACE_RETURN("bind failed");
      return false;
    }

  // gint val = 1;
  // if (setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &val, sizeof(val)) != 0)
  //   {
  //     close(fd);
  //     TRACE_RETURN("setsockopt SO_PASSCRED");
  //     return false;
  //   }
 
  GError *error = NULL;
  netlink_socket = g_socket_new_from_fd(fd, &error);
  if (netlink_socket == NULL)
    {
      close(fd);
      TRACE_RETURN("no socket");
      return false;
    }

  g_socket_set_blocking(netlink_socket, FALSE);
  
  netlink_source = g_socket_create_source(netlink_socket, (GIOCondition)G_IO_IN, NULL);
  g_source_set_callback(netlink_source, (GSourceFunc) static_netlink_data, this, NULL);
  g_source_attach(netlink_source, g_main_context_get_thread_default());

  next_state();
  
  TRACE_RETURN("ok");
  return true;
}


void
NetlinkNetworkInterfaceMonitor::send_request(guint netlink_message, guint flags)
{
  TRACE_ENTER("NetlinkNetworkInterfaceMonitor::send_request");
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;

  struct {
    struct nlmsghdr hdr;
    struct rtgenmsg gen;
  } request;
 
  memset(&request, 0, sizeof(request));
  request.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
  request.hdr.nlmsg_seq = sequence_nr++;
  request.hdr.nlmsg_type = netlink_message;
  request.hdr.nlmsg_flags = NLM_F_REQUEST | flags;
  request.gen.rtgen_family = AF_UNSPEC;

  struct iovec io;
  io.iov_base = &request;
  io.iov_len = request.hdr.nlmsg_len;

  struct msghdr msg;
  memset(&msg, 0, sizeof(msg));
  msg.msg_iov = &io;
  msg.msg_iovlen = 1;
  msg.msg_name = &addr;
  msg.msg_namelen = sizeof(addr);

  int fd = g_socket_get_fd(netlink_socket);
  if (sendmsg(fd, (struct msghdr *) &msg, 0) < 0)
    {
      g_warning("Could not send netlink message: %s", strerror(errno));
    }
  TRACE_EXIT();
}


gboolean
NetlinkNetworkInterfaceMonitor::static_netlink_data(GSocket *socket, GIOCondition condition, gpointer user_data)
{
  TRACE_ENTER("NetlinkNetworkInterfaceMonitor::static_netlink_data");
  
  gboolean ret = TRUE;

  (void) socket;
  (void) condition;

  // check for socket error
  if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
    {
      ret = FALSE;
    }

  // process input
  if (ret && (condition & G_IO_IN))
    {
      TRACE_MSG("in");
      NetlinkNetworkInterfaceMonitor *self = (NetlinkNetworkInterfaceMonitor *)user_data;
      self->process_reply();
    }

  TRACE_RETURN(ret);
  return ret;
}


void
NetlinkNetworkInterfaceMonitor::process_reply()
{
  TRACE_ENTER("NetlinkNetworkInterfaceMonitor::process_reply");
  char buf[8192]; // FIXME: allocate on heap
  GError *error = NULL;

  // GInputVector iv;
  // iv.buffer = NULL;
  // iv.size = 0;
 
  // gint flags = MSG_PEEK | MSG_TRUNC;
  // gssize len = g_socket_receive_message(netlink_socket, NULL, &iv, 1, NULL, NULL, &flags, NULL, &error);
  // if (len < 0)
  //   {
  //     g_warning ("Error on netlink socket1: %s", error->message);
  //     g_error_free(error);
  //     return;
  //   }

  // GSocketControlMessage **cmsgs = NULL;
  // gint num_cmsgs = 0;
  // iv.buffer = g_malloc(len);
  // iv.size = len;
  // len = g_socket_receive_message(netlink_socket, NULL, &iv, 1, &cmsgs, &num_cmsgs, NULL, NULL, &error);
  // if (len < 0)
  //   {
  //     g_warning("Error on netlink socket: %s", error->message);
  //     g_error_free(error);
  //     return;
  //   }

  //  if (num_cmsgs == 1 && G_IS_UNIX_CREDENTIALS_MESSAGE (cmsgs[0]))
  //    {
  //      GCredentials *creds = g_unix_credentials_message_get_credentials (G_UNIX_CREDENTIALS_MESSAGE (cmsgs[0]));
  //      uid_t sender = g_credentials_get_unix_user(creds, NULL);
  //      if (sender != 0)
  //        {
  //        }
  //    }
   
  int len = g_socket_receive(netlink_socket, buf, sizeof(buf), NULL, &error);
  if (len == -1)
    {
      if (error->code != G_IO_ERROR_WOULD_BLOCK)
        {
          g_warning ("Error receiving netlink message: %s", error->message);
        }
      TRACE_RETURN("receive failed");
      return;
    }

  for (struct nlmsghdr* header = (struct nlmsghdr*)buf; NLMSG_OK(header, len); header = NLMSG_NEXT(header, len)) {
    {
      switch (header->nlmsg_type)
        {
        case RTM_NEWADDR:
          process_addr(header, true);
          break;
        case RTM_DELADDR:
          process_addr(header, false);
          break;
        case RTM_NEWLINK:
          process_newlink(header);
          break;
        case RTM_DELLINK:
          process_dellink(header);
          break;
        case NLMSG_DONE:
          next_state();
          break;
        case NLMSG_ERROR:
          {
            struct nlmsgerr *e = (struct nlmsgerr *)NLMSG_DATA(header);
            if (e->error)
              {
                g_warning("Netlink failed: %s", strerror(-e->error));
              }
          }
          break;
        }
    }
  }

  // for (int i = 0; i < num_cmsgs; i++)
  //   {
  //     g_object_unref(cmsgs[i]);
  //   }
  // g_free(cmsgs);

  // g_free(iv.buffer);
  
  TRACE_EXIT();
}


void
NetlinkNetworkInterfaceMonitor::next_state()
{
  if (state == STATE_NONE || sequence_nr == state_sequence_nr)
    {
      switch (state)
        {
        case STATE_NONE:
          send_request(RTM_GETLINK, NLM_F_DUMP | NLM_F_ACK);
          state_sequence_nr = sequence_nr;
          state = STATE_GETLINK;
          break;

        case STATE_GETLINK:
          send_request(RTM_GETADDR, NLM_F_ROOT | NLM_F_MATCH | NLM_F_ACK);
          state = STATE_GETADDR;
          state_sequence_nr = sequence_nr;
          break;

        case STATE_GETADDR:
          state = STATE_DONE;
          state_sequence_nr = -1;
          break;

        case STATE_DONE:
          break;
        }
    }
}


void
NetlinkNetworkInterfaceMonitor::process_newlink(struct nlmsghdr* header)
{
  TRACE_ENTER("NetlinkNetworkInterfaceMonitor::process_newlink");
  struct ifinfomsg *ifi = (struct ifinfomsg *) NLMSG_DATA(header);

  NetworkAdapter &adapter = adapters[ifi->ifi_index];

  adapter.valid = ( ifi->ifi_flags & IFF_UP &&
                    !(ifi->ifi_flags & IFF_LOOPBACK) &&
                    ifi->ifi_flags & IFF_MULTICAST &&
                    !(ifi->ifi_flags & IFF_POINTOPOINT)
                    );
  
  struct rtattr *rta = IFLA_RTA(ifi);
  uint32_t rtalen = IFLA_PAYLOAD(header);
  
  for (; RTA_OK(rta, rtalen); rta = RTA_NEXT(rta, rtalen))
    {
      if (rta->rta_type == IFLA_IFNAME)
        {
          if (adapter.name != "")
            {
              g_warning("Unsupport network interface name change: %s %s", adapter.name.c_str(), (char*)RTA_DATA(rta)); 
              TRACE_MSG("Unsupport network interface name change:" <<  adapter.name.c_str() << " " << (char*)RTA_DATA(rta));
            }
          adapter.name = (char*)RTA_DATA(rta);
          TRACE_MSG("name " << adapter.name);
        }
    }

  verify_network_adapter(adapter);
  TRACE_EXIT();
}


void
NetlinkNetworkInterfaceMonitor::process_dellink(struct nlmsghdr* header)
{
  struct ifinfomsg *ifi = (struct ifinfomsg *) NLMSG_DATA(header);

  NetworkAdapter &adapter = adapters[ifi->ifi_index];
  adapter.valid = false;
  verify_network_adapter(adapter);

  adapters.erase(ifi->ifi_index);
}


void
NetlinkNetworkInterfaceMonitor::process_addr(struct nlmsghdr* header, bool add)
{
  TRACE_ENTER_MSG("NetlinkNetworkInterfaceMonitor::process_addr", add);
  struct ifaddrmsg *ifa = (struct ifaddrmsg *) NLMSG_DATA(header);  

  struct rtattr *rta_address = NULL;
  struct rtattr *rta = IFA_RTA(ifa);
  uint32_t rtalen = IFA_PAYLOAD(header);

  for (; RTA_OK(rta, rtalen); rta = RTA_NEXT(rta, rtalen))
    {
      TRACE_MSG("entty");
      switch (rta->rta_type)
        {
        case IFA_ADDRESS:
          TRACE_MSG("address");
          if (rta_address == NULL)
            {
              rta_address = rta;
            }
          break;

        case IFA_LOCAL:
          TRACE_MSG("local");
          rta_address = rta;
          break;

        default:
          break;
        }
    }

  if (rta_address != NULL)
    {
      GSocketFamily family = (GSocketFamily) ifa->ifa_family;
      NetworkAdapter &adapter = adapters[ifa->ifa_index];

      if (adapter.interfaces.find(family) == adapter.interfaces.end())
        {
          NetworkInterface &interface = adapter.interfaces[family];
          interface.adapter = &adapter;
        }
      
      NetworkInterface &interface = adapter.interfaces[family];
      GInetAddress *addr = create_inet_address(family, rta_address);

      if (addr != NULL)
        {
          if (add)
            {
              interface.address = addr;
            }
          else
            {
              if (g_inet_address_equal(interface.address, addr))
                {
                  g_object_unref(interface.address);
                }
              g_object_unref(addr);
            }

          verify_network_interface(interface);
        }
    }
  TRACE_EXIT();
}


void
NetlinkNetworkInterfaceMonitor::verify_network_adapter(NetworkAdapter &network_hardware)
{
  TRACE_ENTER("NetlinkNetworkInterfaceMonitor::verify_network_adapter");
  for (NetworkInterfacesByFamilyList::iterator i = network_hardware.interfaces.begin(); i != network_hardware.interfaces.end(); i++)
    {
      verify_network_interface(i->second);
    }
  TRACE_EXIT();
}


void
NetlinkNetworkInterfaceMonitor::verify_network_interface(NetworkInterface &network_interface)
{
  TRACE_ENTER("NetlinkNetworkInterfaceMonitor::verify_network_interface");
  bool valid = network_interface.adapter->valid && network_interface.address != NULL;

  TRACE_MSG("valid " << valid << " " << network_interface.valid);
  if (network_interface.valid != valid)
    {
      network_interface.valid = valid;
    
      NetworkInterfaceInfo event;
  
      event.address = GIONetworkAddress::Ptr(new GIONetworkAddress(network_interface.address));
      event.name = network_interface.adapter->name;
      event.valid = valid;

      TRACE_MSG("name " << event.name);
      interface_changed_signal(event);
    }
  TRACE_EXIT();
}


GInetAddress *
NetlinkNetworkInterfaceMonitor::create_inet_address(GSocketFamily family, struct rtattr *rta)
{
  if ((family == AF_INET6 && RTA_PAYLOAD(rta) == 16) || (family == AF_INET && RTA_PAYLOAD(rta) == 4))
    {
      return g_inet_address_new_from_bytes((guint8 *)RTA_DATA(rta), family);
    }
  return NULL;
}
