// ByteArray.hh
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
// $Id$
//

#ifndef BYTEARRAY_HH
#define BYTEARRAY_HH

#include <glib.h>

namespace workrave
{
  struct ByteArray
  {
    ByteArray()
      : size(0), data(NULL), refcount(0)
    {
    }

    ByteArray(int size)
      : size(size), refcount(1)
    {
      data = g_new(guint8, size);
    }

    ByteArray(int size, guint8 *data)
      : size(size), data(data), refcount(1)
    {
    }


    ~ByteArray()
    {
      unref();
    }


    void alloc(int newsize)
    {
      if (data != NULL)
        {
          g_free(data);
        }
      data = g_new(guint8, newsize);
      size = newsize;
      refcount = 1;
    }

    void ref()
    {
      refcount++;
    }

    void unref()
    {
      if (refcount == 0)
        {
          // FIXME: abort?
        }
      else if (--refcount == 0)
        {
          g_free(data);
          data = NULL;
          size = 0;
        }
    }

    int size;
    guint8 *data;
    int refcount;

  private:
    ByteArray(const ByteArray &)
    {
    }

    ByteArray& operator=(const ByteArray &)
    {
      return *this;
    }
  };
}

#endif // BYTEARRAY_HH
