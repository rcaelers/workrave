// BreakResponseInterface.hh --- Generic Interface for breaks
//
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
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

#ifndef BREAKRESPONSEINTERFACE_HH
#define BREAKRESPONSEINTERFACE_HH

#include "CoreInterface.hh"

//! Response Interface for a Break.
class BreakResponseInterface
{
public:
  //! Request to postpone the break.
  virtual void postpone_break(BreakId break_id) = 0;

  //! Request to skip the break.
  virtual void skip_break(BreakId break_id) = 0;

  //! Request to stop the prelude.
  virtual void stop_prelude(BreakId break_id) = 0;
};

#endif // BREAKRESPONSEINTERFACE_HH
