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
// $Id: ByteStream.hh 687 2004-01-28 12:16:46Z dotsphinx $
//

#ifndef BYTESTREAM_HH
#define BYTESTREAM_HH

#include <string>
#include <glib.h>

#include <boost/shared_ptr.hpp>

#include "WRID.hh"
#include "Exception.hh"

using namespace workrave;

#include <google/protobuf/io/zero_copy_stream.h>

class ByteStream  : public google::protobuf::io::ZeroCopyInputStream
{
public:
  ByteStream(gsize size, gchar *data);
  ByteStream(gsize len);
  virtual ~ByteStream();

  void init(gsize len);
  void reset();
  gchar *get_write_buffer();
  gchar *get_read_buffer();
  void advance_read_buffer(gsize size);
  void advance_write_buffer(gsize size);
  gsize get_read_buffer_size() const;
  gsize get_write_buffer_size() const;
  void resize(gsize new_size);
  
  // implements ZeroCopyInputStream
  virtual bool Next(const void **data, int *size);
  virtual void BackUp(int count);
  virtual bool Skip(int count);
  virtual google::protobuf::int64 ByteCount() const;
  
private:
  //! Start of the bytestream buffer
  gchar *data;

  //! Whether we own the data
  bool data_owned;
  
  //! Total size of the buffer
  gsize max_size;

  //! Number of byte stored in the the buffer
  int write_pos;

  //! Current position in the buffer
  int read_pos;

  //!  Number of bytes returned in last  Next() call.
  int last_returned_size;
};

#endif
