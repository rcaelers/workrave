// PreludeResponseInterface.hh --- Generic Interface for breaks
//
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2003-03-09 16:41:46 robc>
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

#ifndef PRELUDERESPONSEINTERFACE_HH
#define PRELUDERESPONSEINTERFACE_HH

//! Interface to a Prelude.
class PreludeResponseInterface
{
public:
  //! 
  virtual void prelude_stopped() = 0;
};

#endif // PRELUDERESPONSEINTERFACE_HH
