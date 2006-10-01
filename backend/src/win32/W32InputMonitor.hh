// W32InputMonitor.hh --- ActivityMonitor for W32
//
// Copyright (C) 2002, 2004, 2006 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef W32INPUTMONITOR_HH
#define W32INPUTMONITOR_HH

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
#include "IInputMonitor.hh"

typedef union HarpoonEventUnion HarpoonEvent;

//! Activity monitor for a local X server.
class W32InputMonitor :
  public IInputMonitor
{
public:
  //! Constructor.
  W32InputMonitor();

  //! Destructor.
  virtual ~W32InputMonitor();

  void init(IInputMonitorListener *);
  void terminate() ;

private:
  static void on_harpoon_event(HarpoonEvent *event);
  static IInputMonitorListener *listener;
};

#endif // W32INPUTMONITOR_HH
