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
      delete [] buffer;
    }
}

void
PacketBuffer::create(int size)
{
  if (buffer != NULL)
    {
      delete [] buffer;
    }
  
  if (size == 0)
    {
      size = 1024;
    }
  
  buffer = new char[size];
  read_ptr = buffer;
  write_ptr = buffer;
  buffer_size = size;
}



void
PacketBuffer::pack(const char *data, int size)
{
  char *ret = NULL;
  
  if (write_ptr + size + 2 <= buffer + buffer_size)
  {
    pack_ushort(size);

    memcpy(write_ptr, data, size);
    write_ptr += size;
  }
}


void
PacketBuffer::pack_string(const char *data)
{
  char *ret = NULL;

  int size = strlen(data);
  
  if (write_ptr + size + 2 <= buffer + buffer_size)
  {
    pack_ushort(size);

    memcpy(write_ptr, data, size);
    write_ptr += size;
  }
}


void
PacketBuffer::pack_ushort(unsigned short data)
{
  if (write_ptr + 2 <= buffer + buffer_size)
    {
      unsigned char *w = (unsigned char *)write_ptr;
      w[0] = ((data & 0x0000ff00) >> 8);
      w[1] = ((data & 0x000000ff));
      
      write_ptr += 2;
    }
}


void
PacketBuffer::pack_ulong(unsigned long data)
{
  if (write_ptr + 4 <= buffer + buffer_size)
    {
      unsigned char *w = (unsigned char *)write_ptr;
      w[0] = ((data & 0xff000000) >> 24);
      w[1] = ((data & 0x00ff0000) >> 16);
      w[2] = ((data & 0x0000ff00) >> 8);
      w[3] = ((data & 0x000000ff));
      
      write_ptr += 4;
    }
}


void
PacketBuffer::pack_char(char data)
{
  if (write_ptr + 1 <= buffer + buffer_size )
    {
      write_ptr[0] = data;

      write_ptr ++;
    }
}


void
PacketBuffer::poke_ushort(int pos, unsigned short data)
{
  if (pos + 2 <= buffer_size )
    {
      unsigned char *w = (unsigned char *)buffer;

      w[pos] = ((data & 0x0000ff00) >> 8);
      w[pos + 1] = ((data & 0x000000ff));
    }
}


char *
PacketBuffer::unpack()
{
  char *ret = NULL;
  int size = unpack_ushort();

  unsigned char *r = (unsigned char *)read_ptr;
  
  if (read_ptr + size <= buffer + buffer_size)
  {
    ret = new char[size];
    memcpy(ret, r, size);
    read_ptr += size;
  }
  
  return ret;
}


char *
PacketBuffer::unpack_string()
{
  char *str = NULL;
  
  if (read_ptr + 2 <= buffer + buffer_size)
  {
    int length = unpack_ushort();

    if (read_ptr + length <= buffer + buffer_size)
      {
        str = new char[length + 1];
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


unsigned long
PacketBuffer::unpack_ulong()
{
  unsigned long ret = 0;
  unsigned char *r = (unsigned char *)read_ptr;

  if (read_ptr + 4 <= buffer + buffer_size)
  {
    ret = (((unsigned long)(r[0]) << 24) +
           ((unsigned long)(r[1]) << 16) +
           ((unsigned long)(r[2]) << 8) +
           ((unsigned long)(r[3])));
    read_ptr +=4;
  }
  
  return ret;
}


unsigned short
PacketBuffer::unpack_ushort()
{
  unsigned short ret = 0;
  unsigned char *r = (unsigned char *)read_ptr;

  if (read_ptr + 2 <= write_ptr)
  {
    ret = (r[0] << 8) + r[1];
    read_ptr += 2;
  }
  return ret;
}


char
PacketBuffer::unpack_char()
{
  char ret = 0;

  if (read_ptr + 1 <= write_ptr)
  {
    ret = read_ptr[0];
    read_ptr ++;
  }
  return ret;
}



char *
PacketBuffer::peek(int pos)
{
  char *ret = NULL;
  int size = peek_ushort(pos);
  
  if (read_ptr + 2 + pos + size <= buffer + buffer_size)
  {
    ret = new char[size];
    memcpy(ret, read_ptr + 2 + pos, size);
  }
  
  return ret;
}


char *PacketBuffer::peek_string(int pos)
{
  char *str = NULL;
  
  if (read_ptr + pos + 2 <= buffer + buffer_size)
  {
    int length = peek_ushort(pos);

    if (read_ptr + 2 + pos + length <= buffer + buffer_size)
      {
        str = new char[length + 1];
        memcpy(str, read_ptr + pos + 2, length);
        str[length] = '\0';
      }
  }
  
  return str;
}


unsigned long PacketBuffer::peek_ulong(int pos)
{
  unsigned long ret = 0;
  if (read_ptr + pos + 4 <= buffer + buffer_size)
  {
    unsigned char *r = (unsigned char *)read_ptr;
    
    ret = (((unsigned long)(r[pos]) << 24) +
           ((unsigned long)(r[pos + 1]) << 16) +
           ((unsigned long)(r[pos + 2]) << 8) +
           ((unsigned long)(r[pos + 3])));
  }
  
  return ret;
}


unsigned short
PacketBuffer::peek_ushort(int pos)
{
  unsigned short ret = 0;
  if (read_ptr + pos + 2 <= write_ptr)
  {
    unsigned char *r = (unsigned char *)read_ptr;
    ret = (r[pos] << 8) + r[pos + 1];
  }
  return ret;
}


char
PacketBuffer::peek_char(int pos)
{
  char ret = 0;
  if (read_ptr + pos + 1 <= buffer + buffer_size)
  {
    ret = read_ptr[pos];
  }
  return ret;
}
