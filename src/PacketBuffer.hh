// PacketBufer.hh
//
// Copyright (C) 2002 Rob Caelers <robc@krandor.org>
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
// $Id$
//

#ifndef PACKETBUFER_HH
#define PACKETBUFER_HH

class PacketBuffer
{
private:
  
  struct TLV
  {
    unsigned short type;
    unsigned short len;
    unsigned char *data;
  };

public:
  PacketBuffer();
  PacketBuffer(int size);
  ~PacketBuffer();

  void create(int size = 0);
  void clear() { write_ptr = read_ptr = buffer; }
  void skip(int size) { read_ptr += size; }
                          
  void pack(const char *data, int size);
  void pack_string(const char *data);
  void pack_ushort(unsigned short data);
  void pack_ulong(unsigned long data);
  void pack_char(char data);

  void poke_ushort(int pos, unsigned short data);
  
  char *unpack();
  char *unpack_string();
  unsigned long unpack_ulong();
  unsigned short unpack_ushort();
  char unpack_char();

  char *peek(int pos);
  char *peek_string(int pos);
  unsigned long peek_ulong(int pos);
  unsigned short peek_ushort(int pos);
  char peek_char(int pos);

  int bytes_available() { return write_ptr - read_ptr; }
  int bytes_written() { return write_ptr - buffer; }
  int bytes_read() { return read_ptr - buffer; }
  
public:
  char *buffer;
  char *write_ptr;
  char *read_ptr;
  int buffer_size;
};


#endif
