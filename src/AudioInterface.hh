// AudioInterface.hh
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-09-06 22:56:07 robc>
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

#ifndef AUDIOINTERFACE_HH
#define AUDIOINTERFACE_HH

#include <string.h>

class AudioInterface
{
public:
  virtual void init() = 0;
  virtual bool play(string file) = 0;
};

#endif // AUDIOINTERFACE_HH
