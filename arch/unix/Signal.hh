// Signal.hh --- Signal synchronization
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2003-01-05 00:36:08 robc>
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

#ifndef SIGNAL_HH
#define SIGNAL_HH

#include <pthread.h>
#include <semaphore.h>

/*!
 * A Signal class.
 */
class Signal 
{
protected:
  //! The POSIX Threads mutex.
  sem_t m_semaphore;

public:
  //! Constructor.
  Signal(int initial = 0)
  {
    sem_init(&m_semaphore, 0, initial);
  }

  //! Destructor
  ~Signal()
  {
    sem_destroy(&m_semaphore);
  }

  //! Locks the mutex.
  void wait()
  {
    sem_wait(&m_semaphore);
  }

  //! Unlocks the mutex.
  void signal()
  {
    sem_post(&m_semaphore);
  }
};


#endif // MUTEX_HH
