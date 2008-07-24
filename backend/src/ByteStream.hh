// ByteStream.hh
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
// $Id: ByteStream.hh 687 2004-01-28 12:16:46Z dotsphinx $
//

#ifndef BYTESTREAM_HH
#define BYTESTREAM_HH

#include <string>
#include <glib.h>

#include "UUID.hh"
#include "Exception.hh"

using namespace workrave;

//! Streaming byte buffer.
class ByteStream
{
public:
  ByteStream();
  ByteStream(int size, guint8 *data);
  ByteStream(int len, int grow_step = 1024);
  ~ByteStream();

  void init(int len, int grow_step = 1024);
  void rewind();
  void advance(int delta);
  void grow(int size);

  int get_available() const;
  int get_offset() const;
  int get_size() const;

  guint8 *get_data_ptr() const;
  guint8 *get_data() const;

  guint8 *take_data();


  guint8  get_u8();
  guint16 get_u16();
  guint32 get_u32();
  guint64 get_u64();
  guint8 *get_raw(int len);
  gchar  *get_string();
  UUID    get_uuid();
  guint8  peek_u8(int pos = -1);
  guint16 peek_u16(int pos = -1);
  guint32 peek_u32(int pos = -1);
  guint64 peek_u64(int pos = -1);

  void    put_u8(guint8 v);
  void    put_u16(guint16 v);
  void    put_u32(guint32 v);
  void    put_u64(guint64 v);
  void    put_raw(int len, const guint8 *raw);
  void    put_string(const gchar *str);
  void    put_string(const std::string data);
  void    put_uuid(const UUID &id);

private:
  void grow_delta(int size);
  void check_delta(int delta);
  void check(int pos, int delta);

private:
  //! Start of the bytestream buffer
  guint8 *data_start;

  //! Current read/write pointer in the buffer
  guint8 *data_ptr;

  //! Total size of the buffer
  int size;

  //! Number of additional bytes to allocate when buffer is full.
  int grow_step;
};


class ByteStreamException : public Exception
{
public:
  explicit ByteStreamException(const std::string &detail) :
    Exception(detail)
  {
  }
  virtual ~ByteStreamException() throw()
  {
  }
};

#include "ByteStream.icc"

#endif
