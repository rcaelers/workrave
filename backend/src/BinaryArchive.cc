// BinaryArchive>.cc --- Binary Serialization
//
// Copyright (C) 2007, 2008, 2009 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <string.h>

#include "BinaryArchive.hh"
#include "ByteStream.hh"

using namespace workrave::serialization;
using namespace std;

#define GROW_SIZE (4096)

enum TLVTypes
  {
    TYPE_TOPCLASS,
    TYPE_CLASS,
    TYPE_BOOL,
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_INT,
    TYPE_WRID,
    TYPE_STRING,
    TYPE_DOUBLE,
    TYPE_LIST,
    TYPE_MAP,
    TYPE_VECTOR,
    TYPE_RAW,
    TYPE_END,
  };

BinaryArchive::BinaryArchive()
  : read_version(0)
{
  stream = new ByteStream();
  stream->init(128, 64);
  own_stream = true;
}


BinaryArchive::BinaryArchive(ByteStream *stream)
  : read_version(0), stream(stream), own_stream(false)
{
}

BinaryArchive::~BinaryArchive()
{
  if (own_stream)
    {
      delete stream;
      own_stream = false;
    }
}


void
BinaryArchive::start_class(string name, int version)
{
  // Header
  stream->put_u16(0x5752);
  stream->put_u8(1);
  stream->put_u8(0);

  // TLV - classname
  stream->put_u8(TYPE_TOPCLASS);
  stream->put_u8(0);
  stream->put_u32(version);
  stream->put_u32(name.size() + 1);
  stream->put_string(name);
}


void
BinaryArchive::start_container(string name, string type)
{
  (void) name;

  // TLV - classname
  if (type == "struct")
    {
      stream->put_u8(TYPE_CLASS);
      stream->put_u8(0);
      stream->put_u32(0);
    }
  else if (type == "list")
    {
      stream->put_u8(TYPE_LIST);
      stream->put_u8(0);
      stream->put_u32(0);
    }
  else if (type == "map")
    {
      stream->put_u8(TYPE_MAP);
      stream->put_u8(0);
      stream->put_u32(0);
    }
  else if (type == "vector")
    {
      stream->put_u8(TYPE_VECTOR);
      stream->put_u8(0);
      stream->put_u32(0);
    }
  else
    {
      ;
    }
}

void
BinaryArchive::end_container()
{
  stream->put_u8(TYPE_END);
  stream->put_u8(0);
  stream->put_u32(0);
}

void
BinaryArchive::add_primitive(string name, int i)
{
  (void) name;

  stream->put_u8(TYPE_INT);
  stream->put_u8(0);
  stream->put_u32(4);
  stream->put_u32(i);
}

void
BinaryArchive::add_primitive(string name, guint8 i)
{
  (void) name;

  stream->put_u8(TYPE_U8);
  stream->put_u8(0);
  stream->put_u32(1);
  stream->put_u8(i);
}

void
BinaryArchive::add_primitive(string name, guint16 i)
{
  (void) name;

  stream->put_u8(TYPE_U16);
  stream->put_u8(0);
  stream->put_u32(2);
  stream->put_u16(i);
}

void
BinaryArchive::add_primitive(string name, guint32 i)
{
  (void) name;

  stream->put_u8(TYPE_U32);
  stream->put_u8(0);
  stream->put_u32(4);
  stream->put_u32(i);
}

void
BinaryArchive::add_primitive(string name, guint64 i)
{
  (void) name;

  stream->put_u8(TYPE_U64);
  stream->put_u8(0);
  stream->put_u32(8);
  stream->put_u64(i);
}

void
BinaryArchive::add_primitive(string name, bool b)
{
  (void) name;

  stream->put_u8(TYPE_BOOL);
  stream->put_u8(0);
  stream->put_u32(1);
  stream->put_u8(b);
}

void
BinaryArchive::add_primitive(string name, string s)
{
  (void) name;

  stream->put_u8(TYPE_STRING);
  stream->put_u8(0);
  stream->put_u32(s.size() + 1);
  stream->put_string(s);
}

void
BinaryArchive::add_primitive(string name, const WRID &id)
{
  (void) name;

  string s = id.str();

  id.raw();

  stream->put_u8(TYPE_WRID);
  stream->put_u8(0);
  stream->put_u32(WRID::RAW_LENGTH);
  stream->put_uuid(id);
}


void
BinaryArchive::add_primitive(string name, const ByteArray &array)
{
  (void) name;

  stream->put_u8(TYPE_RAW);
  stream->put_u8(0);
  stream->put_u32(array.size);
  stream->put_raw(array.size, array.data);
}


void
BinaryArchive::get_class(string &name)
{
  // Header
  int magic = stream->get_u16();
  if (magic != 0x5752)
    {
      throw ArchiveException("incorrect magic");
    }

  int version = stream->get_u8();
  if (version != 1)
    {
      throw ArchiveException("incorrect version");
    }

  int reserved1 = stream->get_u8();
  (void) reserved1;

  // TLV - classname
  TLVTypes type = (TLVTypes) stream->get_u8();
  if (type != TYPE_TOPCLASS)
    {
      throw ArchiveException("incorrect type");
    }

  int reserved2 = stream->get_u8();
  (void) reserved2;

  read_version = stream->get_u32();

  /* int len = */stream->get_u32();
  name = stream->get_string();
}

void
BinaryArchive::get_container(string name, string type, int version)
{
  (void) name;

  if (version > read_version)
    {
      return;
    }

  int t = (TLVTypes) stream->get_u8();

  if ( (t == TYPE_CLASS && type != "struct") ||
       (t == TYPE_LIST &&  type != "list") ||
       (t == TYPE_MAP &&  type != "map") ||
       (t == TYPE_VECTOR &&  type != "vector") )
    {
      throw ArchiveException("incorrect container type");
    }

  int reserved = stream->get_u8();
  (void) reserved;

  int len = stream->get_u32();

  if (len != 0)
    {
      throw ArchiveException("incorrect length");
    }
}

bool
BinaryArchive::get_next_in_container(string name, string type, int version)
{
  (void) name;
  (void) type;

  if (version > read_version)
    {
      return true;
    }

  int t = (TLVTypes) stream->peek_u8();

  if (t == TYPE_END)
    {
      t = stream->get_u8();

      int reserved = stream->get_u8();
      (void) reserved;

      int len = stream->get_u32();
      if (len != 0)
        {
          throw ArchiveException("incorrect length");
        }
    }

  return t != TYPE_END;
}

void
BinaryArchive::get_primitive(string name, int &i, int version)
{
  (void) name;

  if (version > read_version)
    {
      return;
    }

  int t = stream->get_u8();
  if (t != TYPE_INT)
    {
      throw ArchiveException("incorrect type");
    }

  int reserved = stream->get_u8();
  (void) reserved;

  int len = stream->get_u32();
  if (len != 4)
    {
      throw ArchiveException("incorrect length");
    }

  i = stream->get_u32();
}


void
BinaryArchive::get_primitive(string name, guint8 &i, int version)
{
  (void) name;

  if (version > read_version)
    {
      return;
    }

  int t = stream->get_u8();
  if (t != TYPE_U8)
    {
      throw ArchiveException("incorrect type");
    }

  int reserved = stream->get_u8();
  (void) reserved;

  int len = stream->get_u32();
  if (len != 1)
    {
      throw ArchiveException("incorrect length");
    }

  i = stream->get_u8();
}


void
BinaryArchive::get_primitive(string name, guint16 &i, int version)
{
  (void) name;

  if (version > read_version)
    {
      return;
    }

  int t = stream->get_u8();
  if (t != TYPE_U16)
    {
      throw ArchiveException("incorrect type");
    }

  int reserved = stream->get_u8();
  (void) reserved;

  int len = stream->get_u32();
  if (len != 2)
    {
      throw ArchiveException("incorrect length");
    }

  i = stream->get_u16();
}


void
BinaryArchive::get_primitive(string name, guint32 &i, int version)
{
  (void) name;

  if (version > read_version)
    {
      return;
    }

  int t = stream->get_u8();
  if (t != TYPE_U32)
    {
      throw ArchiveException("incorrect type");
    }

  int reserved = stream->get_u8();
  (void) reserved;

  int len = stream->get_u32();
  if (len != 4)
    {
      throw ArchiveException("incorrect length");
    }

  i = stream->get_u8();
}


void
BinaryArchive::get_primitive(string name, guint64 &i, int version)
{
  (void) name;

  if (version > read_version)
    {
      return;
    }

  int t = stream->get_u8();
  if (t != TYPE_U64)
    {
      throw ArchiveException("incorrect type");
    }

  int reserved = stream->get_u8();
  (void) reserved;

  int len = stream->get_u32();
  if (len != 8)
    {
      throw ArchiveException("incorrect length");
    }

  i = stream->get_u64();
}


void
BinaryArchive::get_primitive(string name, bool &b, int version)
{
  (void) name;

  if (version > read_version)
    {
      return;
    }

  int t = stream->get_u8();
  if (t != TYPE_BOOL)
    {
      throw ArchiveException("incorrect type");
    }

  int reserved = stream->get_u8();
  (void) reserved;

  int len = stream->get_u32();
  if (len != 1)
    {
      throw ArchiveException("incorrect length");
    }

  b = stream->get_u8();
}

void
BinaryArchive::get_primitive(string name, string &s, int version)
{
  (void) name;

  if (version > read_version)
    {
      return;
    }

  int t = stream->get_u8();
  if (t != TYPE_STRING)
    {
      throw ArchiveException("incorrect type");
    }

  int reserved = stream->get_u8();
  (void) reserved;

  /* int len = */ stream->get_u32();

  s = stream->get_string();
}


void
BinaryArchive::get_primitive(string name, WRID &id, int version)
{
  (void) name;

  if (version > read_version)
    {
      return;
    }

  int t = stream->get_u8();
  if (t != TYPE_WRID)
    {
      throw ArchiveException("incorrect type");
    }

  int reserved = stream->get_u8();
  (void) reserved;

  /* int len = */ stream->get_u32();

  id = stream->get_uuid();
}


void
BinaryArchive::get_primitive(string name, ByteArray &array, int version)
{
  (void) name;

  if (version > read_version)
    {
      return;
    }

  int t = stream->get_u8();
  if (t != TYPE_RAW)
    {
      throw ArchiveException("incorrect type");
    }

  int reserved = stream->get_u8();
  (void) reserved;

  int len = stream->get_u32();
  guint8 *raw = stream->get_raw(len);

  array.alloc(len);
  memcpy(array.data, raw, len);
}


int
BinaryArchive::
get_data_size() const
{
  return stream->get_offset();
}


guint8 *
BinaryArchive::get_data() const
{
  return stream->get_data();
}


void
BinaryArchive::rewind()
{
  stream->rewind();
}
