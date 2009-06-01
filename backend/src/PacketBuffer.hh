// PacketBufer.hh
//
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
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

#ifndef PACKETBUFER_HH
#define PACKETBUFER_HH

#include "glib.h"

#define GROW_SIZE (4096)

class PacketBuffer
{
public:
  PacketBuffer();
  PacketBuffer(int size);
  ~PacketBuffer();

  void create(int size = 0);
  void resize(int size);
  void grow(int size);
  void narrow(int pos, int size);

  void clear() { narrow(0, -1); write_ptr = read_ptr = buffer; }
  void skip(int size) { read_ptr += size; }
  void insert(int pos, int size);

  void pack(const guint8 *data, int size);
  void pack_raw(const guint8 *data, int size);
  void pack_raw(PacketBuffer &buffer);
  void pack_string(const gchar *data);
  void pack_ushort(guint16 data);
  void pack_ulong(guint32 data);
  void pack_byte(guint8 data);

  void poke_byte(int pos, guint8 data);
  void poke_ushort(int pos, guint16 data);
  void poke_string(int pos, const gchar *data);

  int unpack(guint8 **data);
  int unpack_raw(guint8 **data, int size);
  gchar *unpack_string();
  guint32 unpack_ulong();
  guint16 unpack_ushort();
  guint8 unpack_byte();

  int peek(int pos, guint8 **data);
  gchar *peek_string(int pos);
  guint32 peek_ulong(int pos);
  guint16 peek_ushort(int pos);
  guint8 peek_byte(int pos);

  void reserve_size(int &pos);
  void update_size(int pos);
  int read_size(int &pos);
  void skip_size(int &pos);

  int bytes_available() { return write_ptr - read_ptr; }
  int bytes_written() { return write_ptr - buffer; }
  int bytes_read() { return read_ptr - buffer; }
  gchar *get_buffer() { return (gchar *) buffer; }
  gchar *get_write_ptr() { return (gchar *)write_ptr; }
  int get_buffer_size() { return buffer_size; }
  void restart_read() { read_ptr = buffer; }

public:
  guint8 *buffer;
  guint8 *read_ptr;
  guint8 *write_ptr;
  int buffer_size;

  guint8 *original_buffer;
  int original_buffer_size;
};


#endif
