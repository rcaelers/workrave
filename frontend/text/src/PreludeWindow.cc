// PreludeWindow.cc
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Rob Caelers & Raymond Penners
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

#include "preinclude.h"

#include "debug.hh"
#include "nls.h"

#include "Text.hh"
#include "Util.hh"

#include "CoreFactory.hh"
#include "CoreInterface.hh"

#include "BreakResponseInterface.hh"
#include "PreludeWindow.hh"

//! Construct a new Microbreak window.
PreludeWindow::PreludeWindow(BreakId break_id)
  : break_id(break_id),
    progress_value(0),
    progress_max_value(0),
    prelude_response(NULL)
{

  // XXX: Add window initialization.
  
  switch (break_id)
    {
    case BREAK_ID_MICRO_BREAK:
      cout << _("Time for a micro-break?") << endl;
      break;
        
    case BREAK_ID_REST_BREAK:
      cout << _("You need a rest break...") << endl;
      break;
          
    case BREAK_ID_DAILY_LIMIT:
      cout << _("You should stop for today...") << endl;
      break;

    default:
      break;
    }
}


//! Destructor.
PreludeWindow::~PreludeWindow()
{
}



//! Starts the microbreak.
void
PreludeWindow::start()
{
  TRACE_ENTER("PreludeWindow::start");

  // XXX: Set windows hints (ie. skip_winlist, always on top)
  // XXX: Show prelude window
  // XXX: Center prelude window

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
  TRACE_ENTER("PreludeWindow::destroy");
  delete this;
  TRACE_EXIT();
}


//! Stops the microbreak.
void
PreludeWindow::stop()
{
  TRACE_ENTER("PreludeWindow::stop");

  // XXX: Hide the window

  TRACE_EXIT();
}


//! Refresh window.
void
PreludeWindow::refresh()
{
  // XXX: Show progress

  string s;
  int tminus = progress_max_value - progress_value;
  if (tminus >= 0)
    {
      if (tminus < 0)
        tminus = 0;
      s = progress_text + " " + Text::time_to_string(tminus);
    }

  cout << "PRELUDE " << break_id << " : " << s << endl;
}


//! Sets the prelude progress.
void
PreludeWindow::set_progress(int value, int max_value)
{
  progress_value = value;
  progress_max_value = max_value;
  refresh();
}


//! Set the progress text.
void
PreludeWindow::set_progress_text(AppInterface::PreludeProgressText text)
{
  switch (text)
    {
      // Workrave will force a break when the prelude window is removed.
      // This is done after 'maximum number of prompts' in the preferences.
    case AppInterface::PROGRESS_TEXT_BREAK_IN:
      progress_text = _("Break in");
      break;

      // Workrave will remove the prelude window and try again later.
    case AppInterface::PROGRESS_TEXT_DISAPPEARS_IN:
      progress_text = _("Disappears in");
      break;

      // Workrave will remove the prelude windows and will NOT try again.
      // I wonder if it is still possible to configure this using the GUI.
    case AppInterface::PROGRESS_TEXT_SILENT_IN:
      progress_text = _("Silent in");
      break;
    }
}


//! Sets the prelude stage.
void
PreludeWindow::set_stage(AppInterface::PreludeStage stage)
{
  switch(stage)
    {
      // Initial stage, 'friendly prelude'
    case AppInterface::STAGE_INITIAL:
      break;

      // Move prelude window out-of-the-way. e.g. to top of screen.
    case AppInterface::STAGE_MOVE_OUT:
      break;
      
      // Less friendly prelude
    case AppInterface::STAGE_WARN:
      break;

      // Even less friendly prelude.
    case AppInterface::STAGE_ALERT:
      break;
    }
}
