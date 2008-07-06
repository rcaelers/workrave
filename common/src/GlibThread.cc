// GlibThread.hh --- GlibThread class
//
// Copyright (C) 2002, 2005, 2007, 2008 Rob Caelers & Raymond Penners
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
// $Id: GlibThread.cc 927 2005-10-26 18:51:24Z rcaelers $
//

#include "GlibThread.hh"

#include <glib.h>

Thread::Thread(Runnable *runnable, bool joinable)
{
  thread_handle = NULL;
  this->runnable = runnable;
  this->joinable = joinable;
}


Thread::Thread(bool joinable)
{
  thread_handle = NULL;
  this->runnable = NULL;
  this->joinable = joinable;
}

Thread::~Thread()
{
  wait();
}


void
Thread::start()
{
  if (thread_handle == NULL)
    {
      GError *error = NULL;

      thread_handle = g_thread_create(thread_handler,
                                      this,
                                      joinable ? TRUE : FALSE,
                                      &error);
      if (error != NULL)
        {
          g_error_free(error);
        }
    }
}


void
Thread::wait()
{
  if (thread_handle != NULL && joinable)
    {
      g_thread_join(thread_handle);
      thread_handle = NULL;
    }
}


void
Thread::run()
{
  if (runnable != NULL)
    {
      runnable->run();
    }
}

void
Thread::internal_run()
{
  run();
  thread_handle = NULL;
}


gpointer
Thread::thread_handler(gpointer data)
{
  Thread *t = (Thread *) data;
  if (t != NULL)
    {
      t->internal_run();
      if (!t->joinable)
        {
          delete t;
        }
    }

  return 0;
}

