// IArchive.hh --- Serialization archive
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

#ifndef IARCHIVE_HH
#define IARCHIVE_HH

#include <string>

#include "UUID.hh"
#include "ByteArray.hh"
#include "Exception.hh"

namespace workrave {

  namespace serialization {

    class IArchive
    {
    public:
      IArchive() {}
      virtual ~IArchive() {}

      // Save
      virtual void start_class(std::string name, int version) = 0;
      virtual void start_container(std::string name, std::string type) = 0;
      virtual void end_container() = 0;
      virtual void add_primitive(std::string name, const UUID &id) = 0;
      virtual void add_primitive(std::string name, const int i) = 0;
      virtual void add_primitive(std::string name, const guint8 i) = 0;
      virtual void add_primitive(std::string name, const guint16 i) = 0;
      virtual void add_primitive(std::string name, const guint32 i) = 0;
      virtual void add_primitive(std::string name, const guint64 i) = 0;
      virtual void add_primitive(std::string name, const bool i) = 0;
      virtual void add_primitive(std::string name, const std::string s) = 0;
      virtual void add_primitive(std::string name, const ByteArray &array) = 0;

      // Load
      virtual void get_class(std::string &name) = 0;
      virtual void get_container(std::string name, std::string type, int version) = 0;
      virtual bool get_next_in_container(std::string name, std::string type, int version) = 0;

      virtual void get_primitive(std::string name, UUID &id, int version) = 0;
      virtual void get_primitive(std::string name, int &i, int version) = 0;
      virtual void get_primitive(std::string name, guint8 &i, int version) = 0;
      virtual void get_primitive(std::string name, guint16 &i, int version) = 0;
      virtual void get_primitive(std::string name, guint32 &i, int version) = 0;
      virtual void get_primitive(std::string name, guint64 &i, int version) = 0;
      virtual void get_primitive(std::string name, bool &i, int version) = 0;
      virtual void get_primitive(std::string name, std::string &s, int version) = 0;
      virtual void get_primitive(std::string name, ByteArray &array, int version) = 0;
    };

    class ArchiveException : public Exception
    {
    public:
      explicit ArchiveException(const std::string &detail) :
        Exception(detail)
      {
      }
      virtual ~ArchiveException() throw()
      {
      }
    };
  };
};

#endif // IARCHIVE_HH
