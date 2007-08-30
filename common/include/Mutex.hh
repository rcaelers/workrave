// Mutex.hh
//
// Copyright (C) 2006 Raymond Penners <raymond@dotsphinx.com>
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
// $Id: debug.hh 650 2006-09-20 17:26:32Z rcaelers $
//

#ifndef MUTEX_HH
#define MUTEX_HH

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_QT
#include "QtMutex.hh"
#elif defined(WIN32)
#include "W32Mutex.hh"
#elif defined(HAVE_UNIX)
#include "UnixMutex.hh"
#else
#error Port missing
#endif

#endif // MUTEX_HH
