// Thread.hh --- Thread class
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
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
  //! Constructs a new thread.
  Thread(bool auto_delete = false);

  //! Constructs a new thread for the supplied runnable object.
  Thread(Runnable *target);

  //! Kills the thread.
  virtual ~Thread();

  //! Yields processor to another thread.
  void yield();

  //! Suspends the thread.
  void suspend()
  {
#ifdef  _SIG_THREAD_SUSPEND
    pthread_kill(thread_id, _SIG_THREAD_SUSPEND);
#endif
  }

  //! Resumes this thread.
  void resume()
  {
#ifdef  _SIG_THREAD_RESUME
    pthread_kill(thread_id, _SIG_THREAD_RESUME);
#endif
  }

  //! Returns \c true if the thread is running.
  bool isRunning() const
  {
    return running;
  }

  //! Starts the thread.
  virtual void start();

  //! Stops the thread.
  virtual void stop();

  //! Waits until the thread is dead.
  void wait();

  //! thread entry.
  void internal_run();

  //! Method that is run by this thread unless a Runnable object is supplied.
  virtual void run();

  //! Lets this thread sleep for the specified time.
  static void sleep(long millis, int nanos = 0);

private:
  static void *thread_handler(void* arg);

  //! This thread runnable object. May be zero.
  Runnable *target;

  //! POSIX threads thread id.
  pthread_t thread_id;

  //! Thread is running ?
  bool running;

  //! Auto-delete thread ?
  bool auto_delete;
};


#endif // THREAD_HH
