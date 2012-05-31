// GLibMutex.hh
//
// Copyright (C) 2007, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef GLIBMUTEX_HH
#define GLIBMUTEX_HH

#include <glib.h>

#if GLIB_CHECK_VERSION(2, 31, 18)

class Mutex
{
public:

  Mutex()
  {
    g_rec_mutex_init(&gmutex);
  };

  ~Mutex()
  {
    g_rec_mutex_clear(&gmutex);
  };

  void lock()
  {
    g_rec_mutex_lock(&gmutex);
  }

  void unlock()
  {
    g_rec_mutex_unlock(&gmutex);
  }
private:
  GRecMutex gmutex;
};

#else

class Mutex
{
public:

  Mutex()
  {
    g_static_rec_mutex_init(&gmutex);
  };

  ~Mutex()
  {
    g_static_rec_mutex_free(&gmutex);
  };

  void lock()
  {
    g_static_rec_mutex_lock(&gmutex);
  }

  void unlock()
  {
    g_static_rec_mutex_unlock(&gmutex);
  }
private:
  GStaticRecMutex gmutex;
};


#endif

#endif // GLIBMUTEX_HH
