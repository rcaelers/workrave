// BreakWindow.cc --- base class for the break windows
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2007 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "preinclude.h"
#include "debug.hh"
#include "commonui/nls.h"

#include <math.h>

#include "BreakWindow.hh"
#include "IBreakResponse.hh"
#include "System.hh"
#include "Util.hh"

//! Constructor
BreakWindow::BreakWindow(BreakId break_id, bool ignorable, GUI::BlockMode mode)
  : block_mode(mode)
  , ignorable_break(ignorable)
  , break_response(NULL)
{
  this->break_id = break_id;
}

//! Destructor.
BreakWindow::~BreakWindow()
{
  TRACE_ENTER("BreakWindow::~BreakWindow");
  TRACE_EXIT();
}

//! Break response
void
BreakWindow::set_response(IBreakResponse *bri)
{
  break_response = bri;

  // The Break Window can use this interface to skip and postpone the break
  // by calling
  //   break_response->postpone_break(break_id);
  // or
  //   break_response->skip_break(break_id);
}

//! Starts the break.
void
BreakWindow::start()
{
  TRACE_ENTER("BreakWindow::start");

  // XXX: Initialize GUI
  // XXX: Center Window
  // XXX: Show Window

  TRACE_EXIT();
}

//! Stops the break.
void
BreakWindow::stop()
{
  TRACE_ENTER("BreakWindow::stop");

  // XXX: Hide window

  TRACE_EXIT();
}

//! Self-Destruct
/*!
 *  This method MUST be used to destroy the objects through the
 *  IBreakWindow. it is NOT possible to do a delete on
 *  this interface...
 */
void
BreakWindow::destroy()
{
  delete this;
}

//! Refresh window.
void
BreakWindow::refresh()
{
  cout << "BREAK " << break_id << " : " << progress_value << "/" << progress_max_value << endl;
}

//! Sets the break progress.
void
BreakWindow::set_progress(int value, int max_value)
{
  progress_max_value = max_value;
  progress_value = value;
}
