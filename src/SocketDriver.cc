// SocketDriver.cc
//
// Copyright (C) 2002 Rob Caelers <robc@krandor.org>
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
//

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#include "SocketDriver.hh"

SocketConnection::SocketConnection()
  : data(NULL)
{
}


SocketConnection::~SocketConnection()
{
}


SocketDriver::SocketDriver()
  : listener(NULL)
{
}


SocketDriver::~SocketDriver()
{
}


void
SocketDriver::set_listener(SocketListener *l)
{
  listener = l;
}

