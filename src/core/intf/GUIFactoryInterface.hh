// GUIFactoryInterface.hh
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
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

#ifndef GUIFACTORYINTERFACE_HH
#define GUIFACTORYINTERFACE_HH

#include "CoreInterface.hh"

class PreludeWindowInterface;
class BreakWindowInterface;

class GUIFactoryInterface
{
public:
  //! Returns a Prelude window.
  virtual PreludeWindowInterface *create_prelude_window(BreakId break_id) = 0;

  //! Returns a break window of the specified break type.
  virtual BreakWindowInterface *create_break_window(BreakId break_id, bool ignorable, bool insist) = 0;
};

#endif // RESTGUIFACTORYINTERFACE_HH
