// Control.cc --- The main controller
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <assert.h>

#include "PacketBuffer.hh"

PacketBuffer::PacketBuffer() :
  buffer(NULL),
  read_ptr(NULL),
  write_ptr(NULL),
  buffer_size(0)
{
}


PacketBuffer::~PacketBuffer()
{
  if (buffer != NULL)
    {
      g_free(buffer);
    }
}

void
PacketBuffer::create(int size)
{
  if (buffer != NULL)
    {
      g_free(buffer);
    }
  
  if (size == 0)
    {
      size = 1024;
    }
  
  buffer = g_new(guint8, size);
  read_ptr = buffer;
  write_ptr = buffer;
  buffer_size = size;
}



void
PacketBuffer::pack(const guint8 *data, int size)
{
  guint8 *ret = NULL;
  
  if (write_ptr + size + 2 <= buffer + buffer_size)
  {
    pack_ushort(size);

    memcpy(write_ptr, data, size);
    write_ptr += size;
  }
}



void
PacketBuffer::pack_raw(const guint8 *data, int size)
{
  guint8 *ret = NULL;
  
  if (write_ptr + size <= buffer + buffer_size)
  {
    memcpy(write_ptr, data, size);
    write_ptr += size;
  }
}


void
PacketBuffer::pack_string(const gchar *data)
{
  gchar *ret = NULL;

  int size = 0;  
  if (data != NULL)
    {
      size = strlen(data);
    }
  
  if (write_ptr + size + 2 <= buffer + buffer_size)
  {
    pack_ushort(size);

    if (size > 0)
      {
        memcpy(write_ptr, data, size);
        write_ptr += size;
      }
  }
}


void
PacketBuffer::pack_ushort(guint16 data)
{
  if (write_ptr + 2 <= buffer + buffer_size)
    {
      guint8 *w = (guint8 *)write_ptr;
      w[0] = ((data & 0x0000ff00) >> 8);
      w[1] = ((data & 0x000000ff));
      
      write_ptr += 2;
    }
}


void
PacketBuffer::pack_ulong(guint32 data)
{
  if (write_ptr + 4 <= buffer + buffer_size)
    {
      guint8 *w = (guint8 *)write_ptr;
      w[0] = ((data & 0xff000000) >> 24);
      w[1] = ((data & 0x00ff0000) >> 16);
      w[2] = ((data & 0x0000ff00) >> 8);
      w[3] = ((data & 0x000000ff));
      
      write_ptr += 4;
    }
}


void
PacketBuffer::pack_byte(guint8 data)
{
  if (write_ptr + 1 <= buffer + buffer_size )
    {
      write_ptr[0] = data;

      write_ptr ++;
    }
}


void
PacketBuffer::poke_ushort(int pos, guint16 data)
{
  if (pos + 2 <= buffer_size )
    {
      guint8 *w = (guint8 *)buffer;

      w[pos] = ((data & 0x0000ff00) >> 8);
      w[pos + 1] = ((data & 0x000000ff));
    }
}


int
PacketBuffer::unpack(guint8 **data)
{
  g_assert(data != NULL);
  
  int size = unpack_ushort();

  guint8 *r = (guint8 *)read_ptr;
  
  if (read_ptr + size <= buffer + buffer_size)
    {
      *data = g_new(guint8, size);
      memcpy(*data, r, size);
      read_ptr += size;
    }
  else
    {
      size = 0;
    }
  
  return size;
}


int
PacketBuffer::unpack_raw(guint8 **data, int size)
{
  g_assert(data != NULL);
  
  guint8 *r = (guint8 *)read_ptr;
  
  if (read_ptr + size <= buffer + buffer_size)
    {
      *data = g_new(guint8, size);
      memcpy(*data, r, size);
      read_ptr += size;
    }
  else
    {
      size = 0;
    }
  
  return size;
}


gchar *
PacketBuffer::unpack_string()
{
  gchar *str = NULL;
  
  if (read_ptr + 2 <= buffer + buffer_size)
    {
      int length = unpack_ushort();

      if (read_ptr + length <= buffer + buffer_size)
        {
          str = g_new(gchar, length + 1);
          for (int i = 0; i < length; i++)
            {
              str[i] = *read_ptr;
              read_ptr++;
            }
        
          str[length] = '\0';
        }
    }
  
  return str;
}


guint32
PacketBuffer::unpack_ulong()
{
  guint32 ret = 0;
  guint8 *r = (guint8 *)read_ptr;

  if (read_ptr + 4 <= buffer + buffer_size)
    {
      ret = (((guint32)(r[0]) << 24) +
             ((guint32)(r[1]) << 16) +
             ((guint32)(r[2]) << 8) +
             ((guint32)(r[3])));
      read_ptr +=4;
    }
  
  return ret;
}


guint16
PacketBuffer::unpack_ushort()
{
  guint16 ret = 0;
  guint8 *r = (guint8 *)read_ptr;

  if (read_ptr + 2 <= write_ptr)
  {
    ret = (r[0] << 8) + r[1];
    read_ptr += 2;
  }
  return ret;
}


guint8
PacketBuffer::unpack_byte()
{
  guint8 ret = 0;

  if (read_ptr + 1 <= write_ptr)
    {
      ret = read_ptr[0];
      read_ptr ++;
    }
  return ret;
}



int
PacketBuffer::peek(int pos, guint8 **data)
{
  g_assert(data != NULL);
  
  int size = peek_ushort(pos);
  
  if (read_ptr + 2 + pos + size <= buffer + buffer_size)
    {
      *data = g_new(guint8, size);
      memcpy(*data, read_ptr + 2 + pos, size);
    }
  else
    {
      size = 0;
    }
  
  return size;
}


gchar *
PacketBuffer::peek_string(int pos)
{
  gchar *str = NULL;
  
  if (read_ptr + pos + 2 <= buffer + buffer_size)
    {
      int length = peek_ushort(pos);

      if (read_ptr + 2 + pos + length <= buffer + buffer_size)
        {
          str = g_new(gchar, length + 1);
          memcpy(str, read_ptr + pos + 2, length);
          str[length] = '\0';
        }
    }
  
  return str;
}


guint32 PacketBuffer::peek_ulong(int pos)
{
  guint32 ret = 0;
  if (read_ptr + pos + 4 <= buffer + buffer_size)
    {
      guint8 *r = (guint8 *)read_ptr;
    
      ret = (((guint32)(r[pos]) << 24) +
             ((guint32)(r[pos + 1]) << 16) +
             ((guint32)(r[pos + 2]) << 8) +
             ((guint32)(r[pos + 3])));
    }
  
  return ret;
}


guint16
PacketBuffer::peek_ushort(int pos)
{
  guint16 ret = 0;
  if (read_ptr + pos + 2 <= write_ptr)
    {
      guint8 *r = (guint8 *)read_ptr;
      ret = (r[pos] << 8) + r[pos + 1];
    }
  return ret;
}


guint8
PacketBuffer::peek_byte(int pos)
{
  guint8 ret = 0;
  if (read_ptr + pos + 1 <= buffer + buffer_size)
    {
      ret = read_ptr[pos];
    }
  return ret;
}


void
PacketBuffer::reserve_size(int &pos)
{
  pos = bytes_written();
  pack_ushort(0);
}

void 
PacketBuffer::update_size(int pos)
{
  poke_ushort(pos, bytes_written() - pos - 2);
}


int
PacketBuffer::read_size(int &pos)
{
  int size = unpack_ushort();

  pos = bytes_read() + size;

  return size;
}


void
PacketBuffer::skip_size(int &pos)
{
  int size = (pos - bytes_read());
  skip(size);
}
