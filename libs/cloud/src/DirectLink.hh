// DirectLink.hh --- ing network server
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

#ifndef DIRECTLINK_HH
#define DIRECTLINK_HH

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include "network/Socket.hh"

#include "Link.hh"
#include "ByteStream.hh"

using namespace workrave;
using namespace workrave::network;
using namespace workrave::cloud;

class DirectLink : public Link
{
public:
  typedef boost::shared_ptr<DirectLink> Ptr;

  struct BoolOrCombiner
  {
    typedef bool result_type;
    template <typename InputIterator>
    result_type operator()(InputIterator first, InputIterator last) const
    {
        result_type val = true;
        for (; first != last && val; first++)
          {
            val = *first;            
          }
        return val;
    }
  };
  
public:
  static Ptr create(Marshaller::Ptr marshaller);
  static Ptr create(Marshaller::Ptr marshaller, Socket::Ptr socket);

  DirectLink(Marshaller::Ptr marshaller);
  DirectLink(Marshaller::Ptr marshaller, Socket::Ptr socket);
  virtual ~DirectLink();

  void init(int port);
  void terminate();

  void connect(const std::string &host, int port);
  void send_message(const std::string &message);
  
  boost::signals2::signal<bool(PacketIn::Ptr), BoolOrCombiner> &signal_data();
  boost::signals2::signal<void()> &signal_state();
  
private:
  void on_connected();
  void on_disconnected();
  void on_data();
  void close();
  
private:

  
  //!
  boost::signals2::signal<bool(PacketIn::Ptr), BoolOrCombiner> data_signal;

  //!
  boost::signals2::signal<void()> state_signal;

  Marshaller::Ptr marshaller;
  Socket::Ptr socket;
  boost::shared_ptr<ByteStream> stream;
};


#endif // DIRECTLINK_HH
