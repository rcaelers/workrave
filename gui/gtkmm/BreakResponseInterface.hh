// BreakResponseInterface.hh --- Generic Interface for breaks
//
// Copyright (C) 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-09-04 22:40:25 robc>
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

#ifndef BREAKRESPONSEINTERFACE_HH
#define BREAKRESPONSEINTERFACE_HH

//! Interface to a Break.
class BreakResponseInterface
{
public:
  //! Postpone the break.
  virtual void postpone_break() = 0;

  //! Skip the break.
  virtual void skip_break() = 0;
};

#endif // BREAKRESPONSEINTERFACE_HH
