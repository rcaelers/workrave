// DailyLimitWindow.cc --- window for the daily limit
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "DailyLimitWindow.hh"
#include "TimerInterface.hh"
#include "BreakResponseInterface.hh"
#include "Util.hh"


//! Construct a new Micropause window.
DailyLimitWindow::DailyLimitWindow(bool ignorable)
{
}


//! Destructor.
DailyLimitWindow::~DailyLimitWindow()
{
}


//! Starts the micropause.
void
DailyLimitWindow::start()
{
}


//! Stops the micropause.
void
DailyLimitWindow::stop()
{
}


//! Self-Destruct
/*!
 *  This method MUST be used to destroy the objects through the
 *  BreakWindowInterface. it is NOT possible to do a delete on
 *  this interface...
 */
void
DailyLimitWindow::destroy()
{
  delete this;
}


void
DailyLimitWindow::set_progress(int value, int max_value)
{
  (void) value;
  (void) max_value;
}


void
DailyLimitWindow::set_insist_break(bool insist)
{
  insist_break = insist;
}


//! Refreshes
void
DailyLimitWindow::refresh()
{
}
