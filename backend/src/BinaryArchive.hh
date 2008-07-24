// BinaryArchive.hh --- Binary Serialization
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

#ifndef BINARYARCHIVE_HH
#define BINARYARCHIVE_HH

#include <string>
#include <glib.h>

#include "IArchive.hh"

using namespace workrave;

class ByteStream;

class BinaryArchive : public workrave::serialization::IArchive
{
public:
  BinaryArchive();
  BinaryArchive(ByteStream *stream);
  virtual ~BinaryArchive();

  void start_class(std::string name, int version);
  void start_container(std::string name, std::string type);
  void end_container();

  void add_primitive(std::string name, const UUID &id);
  void add_primitive(std::string name, const int i);
  void add_primitive(std::string name, const guint8 i);
  void add_primitive(std::string name, const guint16 i);
  void add_primitive(std::string name, const guint32 i);
  void add_primitive(std::string name, const guint64 i);
  void add_primitive(std::string name, const bool i);
  void add_primitive(std::string name, const std::string s);
  void add_primitive(std::string name, const ByteArray &array);

  void get_class(std::string &name);
  void get_container(std::string name, std::string type, int version);
  bool get_next_in_container(std::string name, std::string type, int version);

  void get_primitive(std::string name, UUID &id, int version);
  void get_primitive(std::string name, int &i, int version);
  void get_primitive(std::string name, guint8 &i, int version);
  void get_primitive(std::string name, guint16 &i, int version);
  void get_primitive(std::string name, guint32 &i, int version);
  void get_primitive(std::string name, guint64 &i, int version);
  void get_primitive(std::string name, bool &i, int version);
  void get_primitive(std::string name, std::string &s, int version);
  void get_primitive(std::string name, ByteArray &array, int version);

  int get_data_size() const;
  guint8 *get_data() const;
  void rewind();

private:
  //! Version being read.
  int read_version;

  //! Read/Write from/to this stream.
  ByteStream *stream;

  //! Do I own this stream
  bool own_stream;
};

#endif // BINARYARCHIVE_HH
