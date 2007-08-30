// UnixMutex.hh --- Mutex synchronization
//
// Copyright (C) 2001, 2002, 2003, 2005, 2006 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2007-08-30 16:03:07 robc>
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

#ifndef UNIXMUTEX_HH
#define UNIXMUTEX_HH

#include <pthread.h>

#ifndef PTHREAD_MUTEX_RECURSIVE_NP
#define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
#endif

/*!
 * A Mutex class.
 */
class Mutex
{
protected:
  //! The POSIX Threads mutex.
  pthread_mutex_t m_mutex;

public:
  //! Constructor.
  Mutex()
  {
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&m_mutex, &attr);
  }

  //! Destructor
  ~Mutex()
  {
    pthread_mutex_destroy(&m_mutex);
  }

  //! Locks the mutex.
  void lock()
  {
    pthread_mutex_lock(&m_mutex);
  }

  //! Unlocks the mutex.
  void unlock()
  {
    pthread_mutex_unlock(&m_mutex);
  }
};


#endif // UNIXMUTEX_HH
