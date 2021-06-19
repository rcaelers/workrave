// TimerBoxTextView.cc --- Timers Widgets
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

#include <unistd.h>
#include <iostream>

#include "commonui/nls.h"
#include "debug.hh"

#include "TimerBoxTextView.hh"
#include "Util.hh"
#include "Text.hh"
#include "GUI.hh"

#include "CoreFactory.hh"
#include "IBreak.hh"

//! Constructor.
TimerBoxTextView::TimerBoxTextView()
{
  TRACE_ENTER("TimerBoxTextView::TimerBoxTextView");

  string sheep_file = AssetPath::complete_directory("workrave-icon-medium.png", AssetPath::SEARCH_PATH_IMAGES);

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
  (void)id;
  (void)slot;
}

void
TimerBoxTextView::set_time_bar(BreakId id,
                               std::string text,
                               TimerColorId primary_color,
                               int primary_val,
                               int primary_max,
                               TimerColorId secondary_color,
                               int secondary_val,
                               int secondary_max)
{
  TRACE_ENTER("TimerBoxTextView::set_time_bar");

  // Break for which to show the progress.
  TRACE_MSG(id);

  // Text for the break (time as string)
  TRACE_MSG(text);

  // Amount of time the user is active.
  TRACE_MSG(primary_val << " " << primary_max << " " << int(primary_color));

  // Amount of time the user is idle
  TRACE_MSG(secondary_val << " " << secondary_max << " " << int(secondary_color));

  TRACE_EXIT();
}

//! Sets the tooltip
void
TimerBoxTextView::set_tip(string tip)
{
  // XXX: Normally, the tooltip is only shown when
  // the sheep is shown and no timers.
  (void)tip;
}

void
TimerBoxTextView::set_icon(IconType icon)
{
  string file;
  switch (icon)
    {
    case ICON_NORMAL:
      file = AssetPath::complete_directory("workrave-icon-medium.png", AssetPath::SEARCH_PATH_IMAGES);
      break;

    case ICON_QUIET:
      file = AssetPath::complete_directory("workrave-quiet-icon-medium.png", AssetPath::SEARCH_PATH_IMAGES);
      break;

    case ICON_SUSPENDED:
      file = AssetPath::complete_directory("workrave-suspended-icon-medium.png", AssetPath::SEARCH_PATH_IMAGES);
      break;
    }

  // XXX: Do something with file.
  (void)file;
}

void
TimerBoxTextView::update_view()
{
  // XXX: Update the GUI.
}

void
TimerBoxTextView::set_geometry(Orientation orientation, int size)
{
  (void)orientation;
  (void)size;
}

void
TimerBoxTextView::set_enabled(bool enabled)
{
  (void)enabled;
  // Status window disappears, no need to do anything here.
}
