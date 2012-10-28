// ICloud.hh
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

#ifndef WORKRAVE_CLOUD_ICLOUD_HH
#define WORKRAVE_CLOUD_ICLOUD_HH

#include <list>
#include <map>
#include <string>

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "cloud/Message.hh"
#include "cloud/MessageParams.hh"
#include "cloud/MessageContext.hh"

namespace workrave
{
  namespace cloud
  {
    class ICloud
    {
    public:
      typedef boost::shared_ptr<ICloud> Ptr;
      typedef boost::signals2::signal<void(Message::Ptr, MessageContext::Ptr)> MessageSignal;
      
      virtual ~ICloud() {}
      
      virtual void init(int port, std::string username, std::string secret) = 0;
      virtual void terminate() = 0;
      virtual void heartbeat() = 0;
      virtual void connect(const std::string &host, int port) = 0;
      virtual void send_message(Message::Ptr msg, MessageParams::Ptr param) = 0;
      virtual MessageSignal &signal_message(int domain, int id) = 0;

      static Ptr create();
    };

    class ICloudTest
    {
    public:
      typedef boost::shared_ptr<ICloudTest> Ptr;

      virtual UUID get_id() const = 0;
      virtual std::list<UUID> get_clients() const = 0;
      virtual std::list<UUID> get_direct_clients() const = 0;
      virtual int get_cycle_failures() const = 0;
      virtual void start_announce() = 0;
      virtual void disconnect(UUID id) = 0;
    };
  }
}


#endif // WORKRAVE_CLOUD_ICLOUD_HH
