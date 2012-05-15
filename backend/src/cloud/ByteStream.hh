// ByteStream.hh
//
// Copyright (C) 2007, 2009, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef BYTESTREAM_HH
#define BYTESTREAM_HH

#include <string>
#include <glib.h>

#include <boost/shared_ptr.hpp>
#include <google/protobuf/io/zero_copy_stream.h>

#define BYTESTREAM_GROW_SIZE (1024)

template<class T>
class ByteStreamImpl
{
public:
  ByteStreamImpl(T *data, gsize size) : data(data), data_owned(false), max_size(size), position(0)
  {
  }

  ByteStreamImpl(gsize size) : data_owned(true), max_size(size), position(0)
  {
    data = (T *)g_malloc(size * sizeof(T));
  }
  
  virtual ~ByteStreamImpl()
  {
    if (data_owned && data != NULL)
      {
        g_free((void *)data);
      }
  }

  void init(gsize size)
  {
    if (max_size != size)
      {
        if (data_owned && data != NULL)
          {
            g_free((void *)data);
          }

        data = (T *)g_malloc(size * sizeof(T));
        data_owned = true;
        max_size = size;
      }
    position = 0;
  }

  void resize(gsize new_size)
  {
    if (new_size > max_size)
      {
        data = (T *) g_realloc((void *)data, new_size);
        max_size = new_size;
      }
  }
  
  void rewind()
  {
    position = 0;
  }
  
  T *get_start()
  {
    return data;
  }

  T *get_ptr()
  {
    if ((gsize) position < max_size)
      {
        return (T *)(data + position);
      }
  
    return NULL;
  }
  
  gsize get_available() const
  { 
    return max_size - position;
  }

  gsize get_max_size() const
  { 
    return max_size;
  }

  gsize get_position() const
  { 
    return position;
  }
  
  void advance_buffer(gsize size)
  {
    if (position + size <= max_size)
      {
        position += size;
      }
    else
      {
        position = max_size;
      }
  }
  
  const std::string get_buffer_as_string()
  {
    return std::string((const char *)(data), position * sizeof(T));
  }

  bool read_u16(guint16 &value, int offset = -1)
  {
    bool peek = true;
    if (offset == -1)
      {
        offset = position;
        peek = false;
      }

    bool ok = max_size - offset >= sizeof(guint16);
  
    if (ok)
      {
        value = GINT16_FROM_BE(*((guint16 *)(data + offset)));
        if (!peek)
          {
            position += sizeof(guint16);
          }
      }

    return ok;
  }
  
  void write_u16(guint16 value)
  {
    if (get_available() < sizeof(guint16))
      {
        resize(max_size + BYTESTREAM_GROW_SIZE);
      }
      
    *((guint16 *)(data + position)) = GINT16_TO_BE(value);
    position += sizeof(guint16);
  }
  
protected:
  //! Start of the bytestream buffer
  T *data;

  //! Whether we own the data
  bool data_owned;
  
  //! Total size of the buffer
  gsize max_size;

  //! Position in the the buffer
  int position;
};

class ByteStream : public ByteStreamImpl<char>
{
public:
  ByteStream(gsize size) : ByteStreamImpl(size)
  {
  }
};


class ByteStreamOutput : public ByteStreamImpl<char>, public google::protobuf::io::ZeroCopyOutputStream
{
public:
  
  ByteStreamOutput(gsize size) : ByteStreamImpl(size), last_returned_size(0)
  {
  }
  
  bool Next(void** ptr, int* size)
  {
    if (max_size - position < BYTESTREAM_GROW_SIZE)
      {
        resize(max_size + BYTESTREAM_GROW_SIZE);
      }
    
    last_returned_size = max_size - position;
    *ptr = data + position;
    *size = last_returned_size;
    position += last_returned_size;
    return true;
  }
  
  void BackUp(int count)
  {
    if (last_returned_size > 0 && count <= last_returned_size && count >= 0)
      {
        position -= count;
        last_returned_size = 0;
      }
  }
  

  google::protobuf::int64 ByteCount() const
  {
    return position;
  }
  
private:
  int last_returned_size;
};

class ByteStreamInput : public ByteStreamImpl<const char>, public google::protobuf::io::ZeroCopyInputStream
{
public:
  ByteStreamInput(const char *data, gsize size) : ByteStreamImpl(data, size), last_returned_size(0)
  {
  }
  
  bool Next(const void** ptr, int* size)
  {
    if ((gsize)position < max_size)
      {
        last_returned_size = max_size - position;
        *ptr = data + position;
        *size = last_returned_size;
        position += last_returned_size;
        return true;
      }
    else
      {
        last_returned_size = 0;
        return false;
      }
  }
  
  void BackUp(int count)
  {
    if (last_returned_size > 0 && count <= last_returned_size && count >= 0)
      {
        position -= count;
        last_returned_size = 0;
      }
  }
  
  google::protobuf::int64 ByteCount() const
  {
    return position;
  }
  

  bool Skip(int count)
  {
    if ((gsize)(position + count) <= max_size)
      {
        position += count;
        return true;
      }
    else
      {
        position = max_size;
        return false;
      }
  }

private:
  int last_returned_size;
};

#endif

