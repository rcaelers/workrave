// MicroPauseWindow.cc --- window for the micropause
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

#include "MicroPauseWindow.hh"
#include "BreakResponseInterface.hh"


//! Construct a new Micropause window.
MicroPauseWindow::MicroPauseWindow(TimerInterface *timer, bool ignorable)
{
}


//! Destructor.
MicroPauseWindow::~MicroPauseWindow()
{
}


//! Starts the micropause.
void
MicroPauseWindow::start()
{
  TRACE_ENTER("MicroPauseWindow::start");
  TRACE_EXIT();
}


//! Stops the micropause.
void
MicroPauseWindow::stop()
{
  TRACE_ENTER("MicroPauseWindow::stop");
  TRACE_EXIT();
}


//! Self-Destruct
/*!
 *  This method MUST be used to destroy the objects through the
 *  BreakWindowInterface. it is NOT possible to do a delete on
 *  this interface...
 */
void
MicroPauseWindow::destroy()
{
  delete this;
}


//! Updates the main window.
void
MicroPauseWindow::heartbeat()
{
  refresh();
}



//! Refresh window.
void
MicroPauseWindow::refresh()
{
}


void
MicroPauseWindow::set_progress(int value, int max_value)
{
}


void
MicroPauseWindow::set_insist_break(bool insist)
{
  insist_break = insist;
}
