// DistributionLink.hh
//
// Copyright (C) 2002, 2003, 2005 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef DISTRIBUTIONLINK_HH
#define DISTRIBUTIONLINK_HH

#include <string>

class DistributionLinkListener;
class DistributionClientMessageInterface;
class PacketBuffer;

#include "DistributionClientMessageInterface.hh"

class DistributionLink
{
public:
  virtual ~DistributionLink() {}
  
  //! Returns the ID of the node.
  virtual string get_id() const = 0;

  //! Returns the number of remote peers.
  virtual int get_number_of_peers() = 0;

  //! Sets the callback interface to the distribution manager.
  // virtual void set_distribution_manager(DistributionLinkListener *dll) = 0;

  //! Enable/Disable distributed operation.
  virtual bool set_enabled(bool enabled) = 0;

  //! Periodic heartbeat.
  virtual void heartbeat() = 0;

  //! Sets the username and password.
  virtual void set_user(string username, string password) = 0;

  //! Connects to a certain host.
  virtual void connect(string url) = 0;

  //! Disconnect the specified client.
  virtual void disconnect(string id) = 0;

  //! Request to become master.
  virtual bool claim() = 0;

  //! Locks the current master status.
  /*! If locked, requests from remote hosts to become master will be denied. */
  virtual bool set_lock_master(bool lock) = 0;

  //! Registers a client message callback.
  virtual bool register_client_message(DistributionClientMessageID id,
                                       DistributionClientMessageType type,
                                       DistributionClientMessageInterface *callback) = 0;

  //! Unregisters a client message callback.
  virtual bool unregister_client_message(DistributionClientMessageID id) = 0;

  //! Sends a client message to all remote hosts.
  virtual bool broadcast_client_message(DistributionClientMessageID id,
                                        PacketBuffer &buffer) = 0;

  //! Disconnects from all remote clients.
  virtual bool disconnect_all() = 0;

  //! Reconnects to all remote clients.
  virtual bool reconnect_all() = 0;
};

#endif // DISTRIBUTIONLINK_HH
