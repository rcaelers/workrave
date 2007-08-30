// Thread.cc --- Thread class
//
// Copyright (C) 2001, 2002, 2003, 2007 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include "Thread.hh"

void *
Thread::thread_handler(void* arg)
{
  Thread *t = (Thread *)arg;

  if (t != NULL)
    {
      t->internal_run();
      if (t->auto_delete)
        delete t;
    }
  return 0;
}


Thread::Thread(bool del)
{
  target = NULL;
  running = false;
  auto_delete = del;
}


Thread::Thread(Runnable *t)
{
  target = t;
  running = false;
  auto_delete = true;
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
}


void
Thread::start()
{
  pthread_create(&thread_id, NULL, thread_handler, this);
}


void
Thread::stop()
{
  pthread_cancel(thread_id);
}


void
Thread::wait()
{
  pthread_join(thread_id, NULL);
}


void
Thread::run()
{
  if (target != NULL)
    {
      target->run();
    }
}


void
Thread::internal_run()
{
  running = true;
  run();
  running = false;
}
