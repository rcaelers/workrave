// Thread.cc --- Thread class
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-08-17 20:13:46 robc>
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

#include "Thread.hh"

void *
threadHandler(void* arg)
{
    Thread *t = (Thread *)arg;

    if (t != NULL)
      {
        t->internal_run();
      }

    return 0;
}


Thread::Thread()
{
  m_target = NULL;
  m_running = false;
}


Thread::Thread(Runnable *target)
{
  m_target = target;
  m_running = false;
}


Thread::~Thread()
{
  stop();
  wait();
}


void
Thread::yield()
{
}


void
Thread::sleep(long millis, int nanos)
{
#ifdef HAVE_NANOSLEEP
    struct timespec tv;
    tv.tv_sec = millis/1000;
    tv.tv_nsec = (millis% 1000)*1000000 + nanos;
    nanosleep(&tv, 0);
#endif
    // TODO: win32?
}


void
Thread::start()
{
  // FIXME: leak
  pthread_create(&m_threadId, NULL, threadHandler, this);
}


void
Thread::stop()
{
  pthread_cancel(m_threadId);
}


void
Thread::wait()
{
  pthread_join(m_threadId, NULL);
}


void
Thread::run()
{
  if (m_target != NULL)
    {
      m_target->run();
    }
}

void
Thread::internal_run()
{
  m_running = true;
  run();
  m_running = false;
}
