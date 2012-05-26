// MessageContext.hh
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

#ifndef MESSAGECONTEXT_HH
#define MESSAGECONTEXT_HH

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include "cloud/Scope.hh"
#include "cloud/UUID.hh"

namespace workrave
{
  namespace cloud
  {
    class MessageContext : public boost::noncopyable
    {
      MessageContext() : valid_signature(false), scope(workrave::cloud::SCOPE_DIRECT) {}

    public:
      typedef boost::shared_ptr<MessageContext> Ptr;

      static Ptr create()
      {
        return Ptr(new MessageContext());
      }
      
      bool valid_signature;
      workrave::cloud::Scope scope;
      workrave::cloud::UUID source;
    };
  }
}

#endif // MESSAGECONTEXT_HH
