// ByteStream.cc --- Stream of bytes
//
// Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.nl>
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

static const char rcsid[] = "$Id: ByteStream.cc 896 2005-04-23 19:53:01Z rcaelers $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <glib.h>

#include "ByteStream.hh"


//! Create buffer and use existing data.
/*! Note: Ownership is the data is transfered to the buffer
 */
ByteStream::ByteStream(int size, guint8 *data)
  : data_start(data), size(size), grow_step(0)
{
  data_ptr = data_start;
}



//! Create buffer and allocate memory.
ByteStream::ByteStream(int size, int grow_step)
  : size(size), grow_step(grow_step)
{
  data_start = (guint8 *)g_malloc(size);
  data_ptr = data_start;
}


//! Create empty buffer.
ByteStream::ByteStream()
  : data_start(NULL), data_ptr(NULL), size(0), grow_step(256)
{
}


//! Destruct buffer
ByteStream::~ByteStream()
{
  g_free(data_start);
}


//! Initialize the buffer
void
ByteStream::init(int size, int grow_step)
{
  if (this->size != size || data_start != NULL)
    {
      if (data_start != NULL)
        {
          g_free(data_start);
        }

      this->data_start = (guint8 *)g_malloc(size);
      this->data_ptr = data_start;
      this->size = size;
    }

  this->grow_step = grow_step;
}


//! Return the next 8 bit from the byte stream
guint8
ByteStream::get_u8()
{
  check_delta(sizeof(guint8));

  guint8 ret = *((guint8 *)(data_ptr));
  data_ptr += sizeof(guint8);

  return ret;
}


//! Return the next 16 bit from the byte stream
guint16
ByteStream::get_u16()
{
  check_delta(sizeof(guint16));

  guint16 ret = GINT16_FROM_BE(*((guint16 *)(data_ptr)));
  data_ptr += sizeof(guint16);

  return ret;
}


//! Return the next 32 bit from the byte stream
guint32
ByteStream::get_u32()
{
  check_delta(sizeof(guint32));

  guint32 ret = GINT32_FROM_BE(*((guint32 *)(data_ptr)));
  data_ptr += sizeof(guint32);

  return ret;
}


//! Return the next 64 bit from the byte stream
guint64
ByteStream::get_u64()
{
  check_delta(sizeof(guint64));

  guint64 ret = GINT64_FROM_BE(*((guint64 *)(data_ptr)));
  data_ptr += sizeof(guint64);

  return ret;
}


//! Return the next raw value of specified sizes in bytes
guint8 *
ByteStream::get_raw(int len)
{
  check_delta(len);
  guint8 *ret = data_ptr;
  data_ptr += len;

  return ret;
}


//! Return the next UUID from the byte stream
UUID
ByteStream::get_uuid()
{
  UUID id;

  guint8 *uuid = id.raw();

  check_delta(UUID::RAW_LENGTH);

  memcpy(uuid, data_ptr, UUID::RAW_LENGTH);
  data_ptr += UUID::RAW_LENGTH;

  return id;
}


//! Return the next string from the byte stream
gchar  *
ByteStream::get_string()
{
  check_delta(1);

  gchar *str = (gchar *)data_ptr;
  while (data_ptr < data_start + size &&
         data_ptr[0] != '\0')
    {
      data_ptr++;
    }

  data_ptr++;

  return str;
}


//! Return a 8 bit value from the byte stream.
/*! \param pos return value at this position or at current read position if -1
 */
guint8
ByteStream::peek_u8(int pos)
{
  check(pos, sizeof(guint8));

  if (pos == -1)
    {
      return *((guint8 *)(data_ptr));
    }
  else
    {
      return *((guint8 *)(data_start + pos));
    }
}


//! Return a 16 bit value from the byte stream.
/*! \param pos return value at this position or at current read position if -1
 */
guint16
ByteStream::peek_u16(int pos)
{
  check(pos, sizeof(guint16));

  if (pos == -1)
    {
      return GINT16_FROM_BE(*((guint16 *)(data_ptr)));
    }
  else
    {
      return GINT16_FROM_BE(*((guint16 *)(data_start + pos)));
    }
}


//! Return a 32 bit value from the byte stream.
/*! \param pos return value at this position or at current read position if -1
 */
guint32
ByteStream::peek_u32(int pos)
{
  check(pos, sizeof(guint32));

  if (pos == -1)
    {
      return GINT32_FROM_BE(*((guint32 *)(data_ptr)));
    }
  else
    {
      return GINT32_FROM_BE(*((guint32 *)(data_start + pos)));
    }
}


//! Return a 64 bit value from the byte stream.
/*! \param pos return value at this position or at current read position if -1
 */
guint64
ByteStream::peek_u64(int pos)
{
  check(pos, sizeof(guint64));

  if (pos == -1)
    {
      return GINT64_FROM_BE(*((guint64 *)(data_ptr)));
    }
  else
    {
      return GINT64_FROM_BE(*((guint64 *)(data_start + pos)));
    }
}


//! Write a 8 bit value to the byte stream
void
ByteStream::put_u8(guint8 v)
{
  grow_delta(sizeof(guint8));

  *((guint8 *)(data_ptr)) = v;
  data_ptr += sizeof(guint8);

}


//! Write a 16 bit value to the byte stream
void
ByteStream::put_u16(guint16 v)
{
  grow_delta(sizeof(guint16));

  *((guint16 *)(data_ptr)) = GINT16_TO_BE(v);
  data_ptr += sizeof(guint16);
}


//! Write a 32 bit value to the byte stream
void
ByteStream::put_u32(guint32 v)
{
  grow_delta(sizeof(guint32));

  *((guint32 *)(data_ptr)) = GINT32_TO_BE(v);
  data_ptr += sizeof(guint32);
}


//! Write a 64 bit value to the byte stream
void
ByteStream::put_u64(guint64 v)
{
  grow_delta(sizeof(guint64));

  *((guint64 *)(data_ptr)) = GINT64_TO_BE(v);
  data_ptr += sizeof(guint64);
}


//! Write a string to the byte stream
void
ByteStream::put_string(const std::string data)
{
  put_string(data.c_str());
}


//! Write a string value to the byte stream
void
ByteStream::put_string(const gchar *data)
{
  int size = 0;
  if (data != NULL)
    {
      size = strlen(data) + 1;
    }

  grow_delta(size);

  if (size > 0)
    {
      memcpy(data_ptr, data, size);
      data_ptr += size;
    }
}


//! Write a raw byte array to the byte stream
void
ByteStream::put_raw(int len, const guint8 *raw)
{
  grow_delta(len);

  if (len > 0)
    {
      memcpy(data_ptr, raw, len);
      data_ptr += len;
    }
}


//! Write a UUID to the byte stream
void
ByteStream::put_uuid(const UUID &id)
{
  guint8 *uuid = id.raw();

  put_raw(UUID::RAW_LENGTH, uuid);
}


//! Grow buffer if the specified amount of bytes do not fit
void
ByteStream::grow_delta(int delta)
{
  if ((data_ptr - data_start) + delta >= size)
    {
      if (grow_step == 0)
        {
          throw ByteStreamException("cannot grow");
        }

      int offset = (data_ptr - data_start);
      int grow = grow_step * ((delta + grow_step - 1) / grow_step);
      data_start = (guint8*) g_realloc(data_start, size + grow);
      data_ptr = data_start + offset;
      size += grow_step;
    }
}


//! Check if at least delta free bytes are left
void
ByteStream::check_delta(int delta)
{
  if ((data_ptr - data_start) + delta > size)
    {
      throw ByteStreamException("out of data");
    }
}

//! Check if the least delta free bytes are left from position pos
void
ByteStream::check(int pos, int delta)
{
  if (pos == -1)
    {
      if ((data_ptr - data_start) + delta >= size)
        {
          throw ByteStreamException("cannot peek at current pos");
        }
    }
  else
    {
      if (pos + delta > size)
        {
          throw ByteStreamException("cannot peek at specified pos");
        }
    }
}


//! Grow the buffer
void
ByteStream::grow(int new_size)
{
  if (new_size > size)
    {
      int offset = (data_ptr - data_start);
      data_start = (guint8*) g_realloc(data_start, new_size);
      data_ptr = data_start + offset;
      size = new_size;
    }
}
