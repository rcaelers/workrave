// System.hh
//
// Copyright (C) 2002, 2003, 2004 Rob Caelers & Raymond Penners
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

#ifndef SYSTEM_HH
#define SYSTEM_HH

#include "config.h"
#include <glib.h>

#if defined(WIN32)
#include <windows.h>
#endif

class System
{
public:
  static bool is_lockable();
  static bool is_shutdown_supported();
  static void lock();
  static void shutdown();

  static void init(
#if defined(HAVE_X)
                   const char *display
#endif
                   );

#if defined(HAVE_X)
  static bool is_kde() { return kde; }
#endif
  
private:
#if defined(HAVE_X)
  static void init_kde(const char *display);
  
  static gchar *xlock;
  static bool kde;
  
#elif defined(WIN32)
  static bool shutdown_helper(bool for_real);

  typedef HRESULT (FAR PASCAL *LockWorkStationFunc)(void);
  static LockWorkStationFunc lock_func;
  static HINSTANCE user32_dll;
  static bool shutdown_supported;
#endif  
};

#endif // SYSTEM_HH
