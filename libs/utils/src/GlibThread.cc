// GlibThread.hh --- GlibThread class
//
// Copyright (C) 2002, 2005, 2007, 2008, 2012 Rob Caelers & Raymond Penners
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

#include "utils/GlibThread.hh"

#include <glib.h>

Thread::Thread(Runnable *runnable, bool autodelete)
{
  thread_handle = nullptr;
  this->runnable = runnable;
  this->autodelete = autodelete;
}

Thread::Thread(bool autodelete)
{
  thread_handle = nullptr;
  this->runnable = nullptr;
  this->autodelete = autodelete;
}

Thread::~Thread()
{
  wait();
}

void
Thread::start()
{
  if (thread_handle == nullptr)
    {
      GError *error = nullptr;

#if GLIB_CHECK_VERSION(2, 31, 18)
      thread_handle = g_thread_try_new("workrave", thread_handler, this, &error);
#else
      thread_handle = g_thread_create(thread_handler, this, TRUE, &error);
#endif
      if (error != nullptr)
        {
          g_error_free(error);
        }
    }
}

void
Thread::wait()
{
  if (thread_handle != nullptr)
    {
      g_thread_join(thread_handle);
      thread_handle = nullptr;
    }
}

void
Thread::run()
{
  if (runnable != nullptr)
    {
      runnable->run();
    }
}

void
Thread::internal_run()
{
  run();
  thread_handle = nullptr;
}

gpointer
Thread::thread_handler(gpointer data)
{
  Thread *t = (Thread *)data;
  if (t != nullptr)
    {
      t->internal_run();
      if (t->autodelete)
        {
          delete t;
        }
    }

  return nullptr;
}
