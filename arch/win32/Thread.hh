// Thread.hh --- Thread class
//
// Copyright (C) 2002, 2003 Rob Caelers & Raymond Penners
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

#include <windows.h>
#include "Runnable.hh"


/*!
 * Thread class.
 */
class Thread : public Runnable
{
public:
  Thread(bool auto_delete = false);
  virtual ~Thread();
  
  virtual void start();
  void wait();
  virtual void run();

  static void sleep(long millis, int nanos = 0);
  
private:
  static DWORD WINAPI thread_handler(LPVOID lpParameter);

  HANDLE thread_handle;
  bool auto_delete;
};


#endif // THREAD_HH
