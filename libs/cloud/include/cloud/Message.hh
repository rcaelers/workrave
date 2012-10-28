// Message.hh
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

#ifndef WORKRAVE_CLOUD_MESSAGE_HH
#define WORKRAVE_CLOUD_MESSAGE_HH

#include <google/protobuf/message.h>

namespace workrave
{
  namespace cloud
  {
    class Message
    {
    public:
      typedef boost::shared_ptr<google::protobuf::Message> Ptr;
    };
  }
}

#endif // WORKRAVE_CLOUD_MESSAGE_HH
