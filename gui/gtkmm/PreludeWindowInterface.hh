// PreludeWindowInterface.hh --- base class for the break windows
//
// Copyright (C) 2001, 2002 Rob Caelers & Raymond Penners
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

#ifndef PRELUDEWINDOWINTERFACE_HH
#define PRELUDEWINDOWINTERFACE_HH

#include <stdio.h>

#include "BreakWindowInterface.hh"

class PreludeWindowInterface
{
public:
  enum Stage
  {
    STAGE_INITIAL = 0,
    STAGE_MOVE_OUT,
    STAGE_WARN,
    STAGE_ALERT,
  };
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void destroy() = 0;
  virtual void refresh() = 0;
  virtual void set_progress(int value, int max_value) = 0;
  virtual void set_text(string text) = 0;
  virtual void set_stage(Stage stage) = 0;
  virtual void set_progress_text(string text) = 0;
};

#endif // RESTPRELUDEWINDOWINTERFACE_HH
