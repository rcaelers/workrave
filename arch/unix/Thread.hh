// Thread.hh --- Thread class
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-08-17 20:14:00 robc>
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

#ifndef THREAD_HH
#define THREAD_HH

#include <pthread.h>
#include <signal.h>

#include "Runnable.hh"

#ifdef SIGSTOP
#define _SIG_THREAD_SUSPEND SIGSTOP
#endif

#ifdef SIGCONT
#define _SIG_THREAD_RESUME  SIGCONT
#endif

/*!
 * Thread class.
 */
class Thread : public Runnable
{
public:
  //! Construct a new thread.
  Thread();

  //! Construct a new thread for the supplied runnable object.
  Thread(Runnable *target);

  //! Kill thread.
  virtual ~Thread();
  
  //! Yield processor to another thread.
  void yield();

  //! Suspend this thread.
  void suspend()
  {
#ifdef  _SIG_THREAD_SUSPEND
    pthread_kill(m_threadId, _SIG_THREAD_SUSPEND);
#endif
    // TODO: win32
  }

  //! Resume this thread.
  void resume()
  {
#ifdef  _SIG_THREAD_RESUME
    pthread_kill(m_threadId, _SIG_THREAD_RESUME);
#endif
    // TODO: win32
  }

  bool isRunning() const
  {
    return m_running;
  }
    
  //! Start this thread.
  virtual void start();

  //! Stop this thread.
  virtual void stop();

  //! Wait until thread is dead.
  void wait();
  
  //! thread entry.
  void internal_run();

  //! Method that is run by this thread unless a Runnable object is supplied.
  virtual void run();

  //! Let this thread sleep for the specified time. 
  static void sleep(long millis, int nanos = 0);
  
private:
  //! This thread runnable object. May be zero.
  Runnable *m_target;

  //! POSIX threads thread id.
  pthread_t m_threadId;

  //! Thread is running ?
  bool m_running;
};


#endif // THREAD_HH
