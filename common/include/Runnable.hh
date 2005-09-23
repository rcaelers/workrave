// Runnable.hh --- Runnable interface
//
// Copyright (C) 2001, 2002, 2003, 2005 Rob Caelers <robc@krandor.org>
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

#ifndef RUNNABLE_HH
#define RUNNABLE_HH

class Runnable
{
public:
  virtual ~Runnable() {}
  virtual void run() = 0;
};


#endif // RUNNABLE_HH
