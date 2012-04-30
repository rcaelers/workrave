// ByteStream.cc --- Stream of bytes
//
// Copyright (C) 2007, 2008, 2009, 2012 Rob Caelers <robc@krandor.nl>
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
ByteStream::ByteStream(gsize size, gchar *data)
  : data(data), data_owned(false), max_size(size), write_pos(size), read_pos(0), last_returned_size(0)
{
}


//! Create buffer and allocate memory.
ByteStream::ByteStream(gsize size)
  : data_owned(true), max_size(size), write_pos(0), read_pos(0), last_returned_size(0)
{
  data = (gchar *)g_malloc(size);
}


//! Destruct buffer
ByteStream::~ByteStream()
{
  if (data_owned && data != NULL)
    {
      g_free(data);
    }
}

//! Initialize the buffer
void
ByteStream::init(gsize size)
{
  if (max_size != size)
    {
      if (data_owned && data != NULL)
        {
          g_free(data);
        }

      data = (gchar *)g_malloc(size);
      data_owned = true;
      max_size = size;
    }
  read_pos = 0;
  write_pos = 0;
  last_returned_size = 0;
}

//! Initialize the buffer
void
ByteStream::reset()
{
  read_pos = 0;
  write_pos = 0;
  last_returned_size = 0;
}

bool
ByteStream::Next(const void** data, int* size)
{
  if (read_pos < write_pos)
    {
      last_returned_size = write_pos - read_pos;
      *data = data + read_pos;
      *size = last_returned_size;
      read_pos += last_returned_size;
      return true;
    }
  else
    {
      last_returned_size = 0;
      return false;
    }
}

void ByteStream::BackUp(int count)
{
  if (last_returned_size > 0 && count <= last_returned_size && count >= 0)
    {
      read_pos -= count;
      last_returned_size = 0;
    }
}


bool ByteStream::Skip(int count)
{
  if (count >= 0)
    {
      last_returned_size = 0;
      if (count > write_pos - read_pos)
        {
          read_pos = write_pos;
          return false;
        }
      else
        {
          read_pos += count;
          return true;
        }
    }
  return false;
}


google::protobuf::int64 ByteStream::ByteCount() const
{
  return read_pos;
}

gchar *
ByteStream::get_read_buffer()
{
  if (read_pos < write_pos)
    {
      return (gchar *)(data + read_pos);
    }
  
  return NULL;
}

gchar *
ByteStream::get_write_buffer()
{
  if ((gsize) write_pos < max_size)
    {
      return (gchar *)(data + write_pos);
    }
  
  return NULL;
}

void
ByteStream::advance_write_buffer(gsize size)
{
  if ((gsize)(write_pos + size) <= max_size)
    {
      write_pos += size;
    }
  else
    {
      write_pos = max_size;
    }
}


void
ByteStream::advance_read_buffer(gsize size)
{
  if ((gsize)(read_pos + size) <= (gsize) write_pos)
    {
      read_pos += size;
    }
  else
    {
      read_pos = write_pos;
    }
}

gsize
ByteStream::get_read_buffer_size() const
{
  return write_pos - read_pos;
}

gsize
ByteStream::get_write_buffer_size() const
{
  return max_size - write_pos;
}

void
ByteStream::resize(gsize new_size)
{
  if (new_size > max_size)
    {
      data = (gchar*) g_realloc(data, new_size);
      max_size = new_size;
    }
}
