// MessageContext.hh
//
// Copyright (C) 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_FOG_MESSAGECONTEXT_HH
#define WORKRAVE_FOG_MESSAGECONTEXT_HH

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include "fog/Scope.hh"
#include "fog/UUID.hh"

namespace workrave
{
  namespace fog
  {
    class MessageContext : public boost::noncopyable
    {
      MessageContext() : scope(workrave::fog::SCOPE_DIRECT) {}

    public:
      typedef boost::shared_ptr<MessageContext> Ptr;

      static Ptr create()
      {
        return Ptr(new MessageContext());
      }
      
      workrave::fog::Scope scope;
      workrave::fog::UUID source;
    };
  }
}

#endif // WORKRAVE_FOG_MESSAGECONTEXT_HH
