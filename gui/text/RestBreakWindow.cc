// RestBreakWindow.cc --- window for the micropause
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
// All rights reserved.
//
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

const int TIMEOUT = 1000;

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "RestBreakWindow.hh"


//! Constructor
/*!
 *  \param control The controller.
 */
RestBreakWindow::RestBreakWindow(bool ignorable) :
  insist_break(true)
{
}


//! Destructor.
RestBreakWindow::~RestBreakWindow()
{
  TRACE_ENTER("RestBreakWindow::~RestBreakWindow");
  TRACE_EXIT();
}


//! Starts the restbreak.
void
RestBreakWindow::start()
{
  TRACE_ENTER("RestBreakWindow::start");
  TRACE_EXIT();
}


//! Stops the restbreak.
void
RestBreakWindow::stop()
{
  TRACE_ENTER("RestBreakWindow::stop");
  TRACE_EXIT();
}


//! Self-Destruct
/*!
 *  This method MUST be used to destroy the objects through the
 *  BreakWindowInterface. it is NOT possible to do a delete on
 *  this interface...
 */
void
RestBreakWindow::destroy()
{
  delete this;
}


//! Period timer callback.
void
RestBreakWindow::refresh()
{
}


void
RestBreakWindow::set_progress(int value, int max_value)
{
}


void
RestBreakWindow::set_insist_break(bool insist)
{
  insist_break = insist;
}
