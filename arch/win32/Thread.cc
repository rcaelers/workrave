// Thread.hh --- Thread class
//
// Copyright (C) 2002 Rob Caelers & Raymond Penners
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

#include <windows.h>
#include "Thread.hh"


Thread::Thread(bool del)
{
  auto_delete = del;
  thread_handle = NULL;
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
      SECURITY_ATTRIBUTES sa;
      sa.nLength = sizeof(sa);
      sa.bInheritHandle = TRUE;
      sa.lpSecurityDescriptor = NULL;
      DWORD id;
      thread_handle = CreateThread(&sa, 0, thread_handler, this, 0, &id);
    }
}


void
Thread::wait()
{
  if (thread_handle != NULL)
    {
      WaitForSingleObject(thread_handle, INFINITE);
      thread_handle = NULL;
    }
}

void
Thread::run()
{
}

void
Thread::sleep(long millis, int nanos = 0)
{
  Sleep(millis);
}


DWORD WINAPI
Thread::thread_handler(LPVOID lpParameter)
{
  Thread *t = (Thread *) lpParameter;
  if (t != NULL)
    {
      t->run();
      if (t->auto_delete)
        delete t;
    }
}

