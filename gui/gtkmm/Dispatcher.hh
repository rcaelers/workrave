// Dispatcher.cc --- Inter-thread dispatcher
//
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
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

#ifndef DISPATCHER_HH
#define DISPATCHER_HH

#include <sigc++/object.h>
#include <sigc++/slot.h>
#include <sigc++/class_slot.h>
#include <glibmm.h>

class Dispatcher
{
private:
#ifdef WIN32
  //!
  HANDLE event_handle;

  //! Asynchronous queue.
  GAsyncQueue *queue;
  
#else
  //!
  int send_fd;

  //!
  int receive_fd;

#endif

  //! I/O Connection
  SigC::Connection io_connection;

  //!
  SigC::Signal0<void> signal;

public:
  Dispatcher();
  ~Dispatcher();
  SigC::Connection connect(const SigC::Slot0<void>& slot);
  void send_notification();

private:

#ifndef WIN32
  void fd_set_close_on_exec(int fd);
#endif
  bool create_thread_pipe();
  bool io_handler(Glib::IOCondition condition);
};

#endif
