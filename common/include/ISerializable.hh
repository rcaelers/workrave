// ISerializable.hh --- Interface definition for a Workrave link server
//
// Copyright (C) 2007 Rob Caelers <robc@krandor.nl>
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
// $Id: IInputMonitor.hh 1090 2006-10-01 20:49:47Z dotsphinx $
//

#ifndef ISERIALIZABLE_HH
#define ISERIALIZABLE_HH

#include <string>

namespace workrave
{
  namespace serialization
  {
    class Target;

    //! Communication link server
    class ISerializable
    {
    public:
      virtual ~ISerializable() {}

      virtual std::string str() const = 0;
      virtual std::string class_name() const = 0;
      virtual void serialize(workrave::serialization::Target *s) = 0;
    };
  };
};

#endif // ISERIALIZABLE_HH
