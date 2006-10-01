// IBreakResponse.hh --- Generic Interface for breaks
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Rob Caelers <robc@krandor.org>
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

#ifndef IBREAKRESPONSE_HH
#define IBREAKRESPONSE_HH

#include "ICore.hh"

//! Response Interface for a Break.
class IBreakResponse
{
public:
  virtual ~IBreakResponse() {}
  
  //! Request to postpone the break.
  virtual void postpone_break(BreakId break_id) = 0;

  //! Request to skip the break.
  virtual void skip_break(BreakId break_id) = 0;
};

#endif // IBREAKRESPONSE_HH
