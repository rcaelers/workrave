// Win32InputMonitor.hh --- ActivityMonitor for Win32
//
// Copyright (C) 2002, 2004 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef WIN32INPUTMONITOR_HH
#define WIN32INPUTMONITOR_HH

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <windows.h>
#include "InputMonitorInterface.hh"

//! Activity monitor for a local X server.
class Win32InputMonitor :
  public InputMonitorInterface
{
public:
  //! Constructor.
  Win32InputMonitor();

  //! Destructor.
  virtual ~Win32InputMonitor();

  void init(InputMonitorListenerInterface *);
  void terminate() ;

private:
  static LRESULT CALLBACK keyboard_hook(int code, WPARAM wparam, LPARAM lparam);
  static LRESULT CALLBACK keyboard_ll_hook(int code, WPARAM wparam, LPARAM lparam);
  static LRESULT CALLBACK mouse_hook(int code, WPARAM wparam, LPARAM lparam);
  static InputMonitorListenerInterface *listener;
};

#endif // WIN32INPUTMONITOR_HH
