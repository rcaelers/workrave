// RestBreakWindow.hh --- window for the micropause
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

#ifndef RESTBREAKWINDOW_HH
#define RESTBREAKWINDOW_HH

#include "BreakInterface.hh"
#include "BreakWindowInterface.hh"

class RestBreakWindow :
  public BreakWindowInterface
{
public:
  RestBreakWindow(bool ignorable);
  virtual ~RestBreakWindow();

  void start();
  void stop();
  void destroy();
  void set_progress(int value, int max_value);
  void set_insist_break(bool insist);
  void refresh();
  
protected:
  //!
  bool insist_break;
};

#endif // RESTBREAKWINDOW_HH
