// debug.cc 
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-08-22 23:19:18 robc>
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


#include "Mutex.hh"

Mutex g_logMutex;

#ifdef CWDEBUG

namespace workrave
{
  namespace debug
  {
    namespace channels
    {
      namespace dc
      {
	::libcw::debug::channel_ct trace("TRACE");
      }
    }
  }
}

#endif // CWDEBUG
