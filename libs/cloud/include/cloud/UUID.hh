// UUID.hh
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

#ifndef UUID_HH
#define UUID_HH

#include <sstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace workrave
{
  namespace cloud
  {
    class UUID : public boost::uuids::uuid
    {
    public:
      UUID() : boost::uuids::uuid(boost::uuids::random_generator()())
      {
      }

      static UUID from_raw(const std::string &raw)
      {
        boost::uuids::uuid uuid;
        if (raw.size() == uuid.size())
          {
            memcpy(&uuid, raw.data(), uuid.size());
          }
        return UUID(uuid);
      }

      static UUID from_str(const std::string &str)
      {
        boost::uuids::uuid uuid;
        std::stringstream in(str);
        in >> uuid;
        return UUID(uuid);
      }

      std::string raw() const
      {
        return std::string(reinterpret_cast<const char*>(data), sizeof(data));
      }

      std::string str() const
      {
        std::stringstream out;
        out << *this;
        return out.str();
      }

    private:
      explicit UUID(const boost::uuids::uuid& uuid) : boost::uuids::uuid(uuid) {}
    };
  }
}

#endif // UUID_HH
