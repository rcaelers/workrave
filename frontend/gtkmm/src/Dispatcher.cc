// Dispatcher.cc --- Inter-thread dispatcher
//
// Copyright (C) 2003, 2004 Rob Caelers
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"

#ifdef HAVE_GTKMM24
#include <sigc++/compatibility.h>
#endif

#include "Dispatcher.hh"

#include "nls.h"
#include "debug.hh"
#include "errno.h"

#include <glib.h>
#include <fcntl.h>

#ifdef WIN32
#include <windows.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

struct DispatchData
{
  unsigned long tag;
};


//! Constructs a new inter-thread dispatcher.
Dispatcher::Dispatcher() :
#ifdef WIN32
  event_handle(0),
  queue(NULL)
#else
  send_fd(-1),
  receive_fd(-1)
#endif
{
  create_thread_pipe();
}


//! Destructs the inter-thread dispatcher.
Dispatcher::~Dispatcher()
{
  io_connection.disconnect();

#ifdef WIN32
  if (event_handle != 0)
  {
    CloseHandle(event_handle);
  }

  g_async_queue_unref(queue);
  
#else
  if (send_fd != -1)
    {
      close(send_fd);
    }
  if (receive_fd != -1)
    {
      close(receive_fd);
    }
#endif
}
  

#ifdef WIN32
//! Creates a new inter-thread pipe
bool
Dispatcher::create_thread_pipe()
{
  event_handle = CreateEvent (NULL, FALSE, FALSE, NULL);  
  if (event_handle)
    {
      queue =  g_async_queue_new();
      io_connection = Glib::signal_io().connect(SigC::slot_class(*this, &Dispatcher::io_handler),
                                                (int)event_handle,
                                                Glib::IO_IN);

    }
  
  return event_handle != 0;
}

void
Dispatcher::send_notification()
{
  TRACE_ENTER("Dispatcher::send_notification");
  DispatchData *data = new DispatchData();

  data->tag        = 0xdeadbeef;

  g_async_queue_push(queue, data);

  SetEvent(event_handle);
  TRACE_EXIT();
}


//! Process incoming data from the inter-thread pipe.
bool
Dispatcher::io_handler(Glib::IOCondition)
{
  TRACE_ENTER("Dispatcher::io_handler");
  DispatchData *data = NULL;
  
  while ((data = (DispatchData*)g_async_queue_try_pop(queue)))
    {
      DispatchData local_data = *data;
      delete data;
      
      g_return_val_if_fail(local_data.tag == 0xdeadbeef, true);

      signal();
    }

  return true;
}
#endif

#ifndef WIN32

//! Make sure the file descriptor is not leaked.
void
Dispatcher::fd_set_close_on_exec(int fd)
{
  const int flags = fcntl(fd, F_GETFD, 0);
  g_return_if_fail(flags >= 0);

  fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
}

//! Creates a new inter-thread pipe
bool
Dispatcher::create_thread_pipe()
{
  bool ret = false;
  int filedes[2] = { -1, -1 };

  if (pipe(filedes) >= 0)
  {
    receive_fd = filedes[0];
    send_fd  = filedes[1];
    
    fd_set_close_on_exec(receive_fd);
    fd_set_close_on_exec(send_fd);
    
    io_connection = Glib::signal_io().connect(SigC::slot_class(*this, &Dispatcher::io_handler),
                                              (int)receive_fd,
                                              Glib::IO_IN);

    ret = true;
  }

  return ret;
}

void
Dispatcher::send_notification()
{
  DispatchData data =
  {
    0xdeadbeef,
  };

  gsize n_written = 0;
  do
  {
    void *const buffer = reinterpret_cast<guint8*>(&data) + n_written;
    const gssize result = write(send_fd, buffer, sizeof(data) - n_written);
    if (result < 0)
    {
      if(errno == EINTR)
        continue;

      return;
    }

    n_written += result;
  }
  while(n_written < sizeof(data));
}


//! Process incoming data from the inter-thread pipe.
bool
Dispatcher::io_handler(Glib::IOCondition)
{
  DispatchData data = { 0 };
  gsize n_read = 0;

  do
  {
    void * const buffer = reinterpret_cast<guint8*>(&data) + n_read;
    const gssize result = read(receive_fd, buffer, sizeof(data) - n_read);

    if (result < 0)
    {
      if (errno == EINTR)
        continue;

      return true;
    }

    n_read += result;
  }
  while(n_read < sizeof(data));

  g_return_val_if_fail(data.tag == 0xdeadbeef, true);

  signal();
  
  return true;
}

#endif


SigC::Connection
Dispatcher::connect(const SigC::Slot0<void>& slot)
{
  return signal.connect(slot);
}

