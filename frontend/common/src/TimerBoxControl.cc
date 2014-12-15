// TimerBoxControl.cc --- Timers Widgets
//
// Copyright (C) 2001 - 2014 Rob Caelers & Raymond Penners
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
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <iostream>

#include "TimerBoxControl.hh"

#include "nls.h"
#include "debug.hh"

#include "ITimeBar.hh"
#include "Text.hh"

#include "CoreFactory.hh"
#include "CoreConfig.hh"
#include "GUIConfig.hh"
#include "IBreak.hh"

using namespace std;
using namespace workrave;
using namespace workrave::config;


//! Constructor.
TimerBoxControl::TimerBoxControl(std::string n, ITimerBoxView &v) :
  view(&v),
  cycle_time(10),
  name(n),
  force_duration(0),
  force_empty(false)
{
  init();
}

//! Updates the timerbox.
void
TimerBoxControl::update()
{
  ICore::Ptr core = CoreFactory::get_core();
  OperationMode mode = core->get_operation_mode();

  if (reconfigure)
    {
      // Configuration was changed. reinit.
      init_table();

      operation_mode = mode;
      init_icon();

      reconfigure = false;
    }
  else
    {
      if (force_duration == 0)
        {
          time_t t = time(NULL);
          if (t % cycle_time == 0)
            {
              init_table();
              cycle_slots();
            }
        }
      else
        {
          force_duration--;
        }
    }

  // Update visual feedback of operating mode.
  if (mode != operation_mode)
    {
      operation_mode = mode;
      init_icon();
    }

  // Update the timer widgets.
  update_widgets();
  view->update_view();
}


void
TimerBoxControl::force_cycle()
{
  force_duration = cycle_time;
  init_table();
  cycle_slots();
}

void
TimerBoxControl::set_force_empty(bool s)
{
  force_empty = s;
}


//! Initializes the timerbox.
void
TimerBoxControl::init()
{
  TRACE_ENTER("TimerBoxControl::init");

  connections.add(GUIConfig::key_timerbox(name).connect(boost::bind(&TimerBoxControl::load_configuration, this)));

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      connections.add(CoreConfig::break_enabled(BreakId(i)).connect(boost::bind(&TimerBoxControl::load_configuration, this)));

      break_position[i] = i;
      break_flags[i] = 0;
      break_imminent_time[i] = 0;

      for (int j = 0; j < BREAK_ID_SIZEOF; j++)
        {
          break_slots[i][j] = -1;
        }
      break_slot_cycle[i] = 0;
    }

  // Load the configuration
  load_configuration();

  TRACE_EXIT();
}




//! Updates the main window.
void
TimerBoxControl::update_widgets()
{
  for (int count = 0; count < BREAK_ID_SIZEOF; count++)
    {
      ICore::Ptr core = CoreFactory::get_core();
      IBreak::Ptr b = core->get_break((BreakId)count);

      std::string text;
      ITimeBar::ColorId primary_color;
      int primary_val, primary_max;
      ITimeBar::ColorId secondary_color;
      int secondary_val, secondary_max;

      // Collect some data.
      time_t maxActiveTime = b->get_limit();
      time_t activeTime = b->get_elapsed_time();
      time_t breakDuration = b->get_auto_reset();
      time_t idleTime = b->get_elapsed_idle_time();
      bool overdue = (maxActiveTime < activeTime);

      // Set the text
      if (b->is_limit_enabled() && maxActiveTime != 0)
        {
          text = Text::time_to_string(maxActiveTime - activeTime);
        }
      else
        {
          text = Text::time_to_string(activeTime);
        }
      // And set the bar.
      secondary_val = secondary_max = 0;
      secondary_color = ITimeBar::COLOR_ID_INACTIVE;

      // Timer is running, show elapsed time.
      primary_val = (int)activeTime;
      primary_max = (int)maxActiveTime;

      primary_color = overdue
        ? ITimeBar::COLOR_ID_OVERDUE : ITimeBar::COLOR_ID_ACTIVE;

      if (b->is_auto_reset_enabled() && breakDuration != 0)
        {
          // resting.
          secondary_color = ITimeBar::COLOR_ID_INACTIVE;
          secondary_val = (int)idleTime;
          secondary_max = (int)breakDuration;
        }

      view->set_time_bar(BreakId(count), text,
                         primary_color, primary_val, primary_max,
                         secondary_color, secondary_val, secondary_max);
    }
}


void
TimerBoxControl::init_icon()
{
  switch (operation_mode)
    {
    case OperationMode::Normal:
      view->set_icon(StatusIconType::Normal);
      break;

    case OperationMode::Suspended:
      view->set_icon(StatusIconType::Suspended);
      break;

    case OperationMode::Quiet:
      view->set_icon(StatusIconType::Quiet);
      break;
    }
}


//! Initializes the applet.
void
TimerBoxControl::init_table()
{
  TRACE_ENTER("TimerBoxControl::init_table");

  if (force_empty)
    {
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          view->set_slot(BREAK_ID_NONE, i);
        }
    }
  else
    {
      // Determine what breaks to show.
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          init_slot(i);
        }

      // New content.
      int slot = 0;
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          int cycle = break_slot_cycle[i];
          int id = break_slots[i][cycle]; // break id
          if (id != -1)
            {
              view->set_slot(BreakId(id), slot);
              slot++;
            }
        }
      for (int i = slot; i < BREAK_ID_SIZEOF; i++)
        {
          view->set_slot(BREAK_ID_NONE, i);
        }
    }
  TRACE_EXIT();
}


//! Compute what break to show on the specified location.
void
TimerBoxControl::init_slot(int slot)
{
  // TRACE_ENTER_MSG("TimerBoxControl::init_slot", slot);
  int count = 0;
  int breaks_id[BREAK_ID_SIZEOF];

  // Collect all timers for this slot.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      ICore::Ptr core = CoreFactory::get_core();
      IBreak::Ptr b = core->get_break(BreakId(i));

      bool on = b->is_enabled();

      if (on && break_position[i] == slot && !(break_flags[i] & GUIConfig::BREAK_HIDE))
        {
          breaks_id[count] = i;
          break_flags[i] &= ~GUIConfig::BREAK_SKIP;
          count++;
        }
    }

  // Compute timer that will elapse first.
  time_t first = 0;
  int first_id = -1;

  for (int i = 0; i < count; i++)
    {
      int id = breaks_id[i];
      int flags = break_flags[id];

      ICore::Ptr core = CoreFactory::get_core();
      IBreak::Ptr b = core->get_break((BreakId)i);

      time_t time_left = b->get_limit() - b->get_elapsed_time();

      // Exclude break if not imminent.
      if (flags & GUIConfig::BREAK_WHEN_IMMINENT && time_left > break_imminent_time[id] &&
          force_duration == 0)
        {
          break_flags[id] |= GUIConfig::BREAK_SKIP;
        }

      // update first imminent timer.
      if (!(flags & GUIConfig::BREAK_SKIP) && (first_id == -1 || time_left < first))
        {
          first_id = id;
          first = time_left;
        }
    }


  // Exclude break if not first.
  for (int i = 0; i < count; i++)
    {
      int id = breaks_id[i];
      int flags = break_flags[id];

      if (!(flags & GUIConfig::BREAK_SKIP))
        {
          if (flags & GUIConfig::BREAK_WHEN_FIRST && first_id != id && force_duration == 0)
            {
              break_flags[id] |= GUIConfig::BREAK_SKIP;
            }
        }
    }


  // Exclude breaks if not exclusive.
  bool have_one = false;
  int breaks_left = 0;
  for (int i = 0; i < count; i++)
    {
      int id = breaks_id[i];
      int flags = break_flags[id];

      if (!(flags & GUIConfig::BREAK_SKIP))
        {
          if (flags & GUIConfig::BREAK_EXCLUSIVE && have_one && force_duration == 0)
            {
              break_flags[id] |= GUIConfig::BREAK_SKIP;
            }

          have_one = true;
        }

      if (!(flags & GUIConfig::BREAK_SKIP))
        {
          breaks_left++;
        }
    }

  if (breaks_left == 0)
    {
      for (int i = 0; i < count; i++)
        {
          int id = breaks_id[i];
          int flags = break_flags[id];

          if (flags & GUIConfig::BREAK_DEFAULT && flags & GUIConfig::BREAK_SKIP)
            {
              break_flags[id] &= ~GUIConfig::BREAK_SKIP;
              breaks_left = 1;
              break;
            }
        }
    }

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      break_slots[slot][i] = -1;
    }

  int new_count = 0;
  for (int i = 0; i < count; i++)
    {
      int id = breaks_id[i];
      int flags = break_flags[id];

      if (!(flags & GUIConfig::BREAK_SKIP))
        {
          break_slots[slot][new_count] = id;
          new_count++;
        }
    }
}


//! Cycles through the breaks.
void
TimerBoxControl::cycle_slots()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      break_slot_cycle[i]++;
      if (break_slot_cycle[i] >= BREAK_ID_SIZEOF
          || break_slots[i][break_slot_cycle[i]] == -1)
        {
          break_slot_cycle[i] = 0;
        }
    }
}


//! Reads the applet configuration.
void
TimerBoxControl::load_configuration()
{
  TRACE_ENTER("TimerBoxControl::load_configuration");
  cycle_time = GUIConfig::timerbox_cycle_time(name)();
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakId bid = (BreakId) i;

      break_position[i] = GUIConfig::timerbox_slot(name, bid)();
      break_flags[i] = GUIConfig::timerbox_flags(name, bid)();
      break_imminent_time[i] = GUIConfig::timerbox_imminent(name, bid)();

      break_slot_cycle[i] = 0;
    }
  view->set_enabled(GUIConfig::timerbox_enabled(name)());
  reconfigure = true;
  TRACE_EXIT();
}
