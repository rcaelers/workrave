// PreludeWindow.cc
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

#include "debug.hh"

#include "PreludeWindow.hh"

//! Construct a new Micropause window.
/*!
 *  \param control Interface to the controller.
 *  \param timer Interface to the restbreak timer.
 */
PreludeWindow::PreludeWindow()
{
}


//! Destructor.
PreludeWindow::~PreludeWindow()
{
}



//! Starts the micropause.
void
PreludeWindow::start()
{
  TRACE_ENTER("PreludeWindow::start");
  TRACE_EXIT();
}


//! Self-Destruct
/*!
 *  This method MUST be used to destroy the objects through the
 *  PreludeWindowInterface. it is NOT possible to do a delete on
 *  this interface...
 */
void
PreludeWindow::destroy()
{
  delete this;
}


//! Stops the micropause.
void
PreludeWindow::stop()
{
  TRACE_ENTER("PreludeWindow::stop");
  TRACE_EXIT();
}


//! Refresh window.
void
PreludeWindow::refresh()
{
}


void
PreludeWindow::set_progress(int value, int max_value)
{
  TRACE_ENTER_MSG("PreludeWindow::set_progress", value << " " << max_value);
  TRACE_EXIT()
}


void
PreludeWindow::set_text(string text)
{
}


void
PreludeWindow::set_progress_text(string text)
{
}


void
PreludeWindow::set_stage(Stage stage)
{
  TRACE_ENTER_MSG("PreludeWindow::set_stage", stage);
  TRACE_EXIT();
}
