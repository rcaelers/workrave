// TimerBoxTextView.cc --- Timers Widgets
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

#include <unistd.h>
#include <iostream>

#include "nls.h"
#include "debug.hh"

#include "TimerBoxTextView.hh"
#include "Util.hh"
#include "Text.hh"
#include "GUI.hh"

#include "CoreFactory.hh"
#include "BreakInterface.hh"


//! Constructor.
TimerBoxTextView::TimerBoxTextView()
{
  TRACE_ENTER("TimerBoxTextView::TimerBoxTextView");

  string sheep_file = Util::complete_directory("workrave-icon-medium.png",
                                               Util::SEARCH_PATH_IMAGES);

  // XXX: Initialize widgets.

  TRACE_EXIT();
}



//! Destructor.
TimerBoxTextView::~TimerBoxTextView()
{
}


//! Indicates that break 'id' must be shown on position 'slot'
void
TimerBoxTextView::set_slot(BreakId id, int slot)
{
  (void) id;
  (void) slot;
}

void
TimerBoxTextView::set_time_bar(BreakId id,
                              std::string text,
                              TimeBarInterface::ColorId primary_color,
                              int primary_val, int primary_max,
                              TimeBarInterface::ColorId secondary_color,
                              int secondary_val, int secondary_max)
{
  TRACE_ENTER("TimerBoxTextView::set_time_bar");

  // Break for which to show the progress.
  TRACE_MSG(id);

  // Text for the break (time as string)
  TRACE_MSG(text);

  // Amount of time the user is active.
  TRACE_MSG(primary_val << " " << primary_max << " " << int(primary_color));

  // Amount of time the user is idle
  TRACE_MSG(secondary_val << " " << secondary_max <<" " << int(secondary_color));

  TRACE_EXIT();
}


//! Sets the tooltip
void
TimerBoxTextView::set_tip(string tip)
{
  // XXX: Normally, the tooltip is only shown when
  // the sheep is shown and no timers.
  (void) tip;
}


void
TimerBoxTextView::set_icon(IconType icon)
{
  string file;
  switch (icon)
    {
    case ICON_NORMAL:
      file = Util::complete_directory("workrave-icon-medium.png",
                                      Util::SEARCH_PATH_IMAGES);
      break;

    case ICON_QUIET:
      file = Util::complete_directory("workrave-quiet-icon-medium.png",
                                             Util::SEARCH_PATH_IMAGES);
      break;

    case ICON_SUSPENDED:
      file = Util::complete_directory("workrave-suspended-icon-medium.png",
                                      Util::SEARCH_PATH_IMAGES);
      break;
    }

  // XXX: Do something with file.
  (void) file;
}

void
TimerBoxTextView::update()
{
  // XXX: Update the GUI.
}

void
TimerBoxTextView::set_enabled(bool enabled)
{
  (void) enabled;
  // Status window disappears, no need to do anything here.
}
