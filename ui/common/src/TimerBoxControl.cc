// TimerBoxControl.cc --- Timers Widgets
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2012 Rob Caelers & Raymond Penners
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

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifdef PLATFORM_OS_MACOS
#  include "MacOSHelpers.hh"
#endif

#include <iostream>

#include "nls.h"
#include "debug.hh"

#include "commonui/TimerBoxControl.hh"
#include "commonui/ITimeBar.hh"
#include "utils/Util.hh"
#include "commonui/Text.hh"

#include "core/CoreFactory.hh"
#include "core/CoreConfig.hh"
#include "core/IBreak.hh"
#include "config/IConfigurator.hh"

using namespace workrave;
using namespace std;

const std::string TimerBoxControl::CFG_KEY_TIMERBOX = "gui/";
const std::string TimerBoxControl::CFG_KEY_TIMERBOX_CYCLE_TIME = "/cycle_time";
const std::string TimerBoxControl::CFG_KEY_TIMERBOX_ENABLED = "/enabled";
const std::string TimerBoxControl::CFG_KEY_TIMERBOX_POSITION = "/position";
const std::string TimerBoxControl::CFG_KEY_TIMERBOX_FLAGS = "/flags";
const std::string TimerBoxControl::CFG_KEY_TIMERBOX_IMMINENT = "/imminent";

//! Constructor.
TimerBoxControl::TimerBoxControl(std::string n, ITimerBoxView &v)
  : view(&v)
  , name(n)
{
  init();
}

//! Destructor.
TimerBoxControl::~TimerBoxControl()
{
  workrave::config::IConfigurator::Ptr config = CoreFactory::get_configurator();
  config->remove_listener(this);
}

//! Updates the timerbox.
void
TimerBoxControl::update()
{
  ICore *core = CoreFactory::get_core();
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
          time_t t = time(nullptr);
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

  // Listen for configugration changes.
  workrave::config::IConfigurator::Ptr config = CoreFactory::get_configurator();
  config->add_listener(TimerBoxControl::CFG_KEY_TIMERBOX + name, this);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      config->add_listener(CoreConfig::CFG_KEY_BREAK_ENABLED % BreakId(i), this);

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
  read_configuration();

  reconfigure = true;

  TRACE_EXIT();
}

//! Updates the main window.
void
TimerBoxControl::update_widgets()
{
  for (int count = 0; count < BREAK_ID_SIZEOF; count++)
    {
      ICore *core = CoreFactory::get_core();
      IBreak *b = core->get_break((BreakId)count);

      std::string text;
      ITimeBar::ColorId primary_color;
      int primary_val, primary_max;
      ITimeBar::ColorId secondary_color;
      int secondary_val, secondary_max;

      if (b == nullptr)
        {
          continue;
        }

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

      primary_color = overdue ? ITimeBar::COLOR_ID_OVERDUE : ITimeBar::COLOR_ID_ACTIVE;

      if (b->is_auto_reset_enabled() && breakDuration != 0)
        {
          // resting.
          secondary_color = ITimeBar::COLOR_ID_INACTIVE;
          secondary_val = (int)idleTime;
          secondary_max = (int)breakDuration;
        }

      view->set_time_bar(
        BreakId(count), text, primary_color, primary_val, primary_max, secondary_color, secondary_val, secondary_max);
    }
}

void
TimerBoxControl::init_icon()
{
  switch (operation_mode)
    {
    case OPERATION_MODE_NORMAL:
      view->set_icon(ITimerBoxView::ICON_NORMAL);
      break;

    case OPERATION_MODE_SUSPENDED:
      view->set_icon(ITimerBoxView::ICON_SUSPENDED);
      break;

    case OPERATION_MODE_QUIET:
      view->set_icon(ITimerBoxView::ICON_QUIET);
      break;

    default:
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
      ICore *core = CoreFactory::get_core();
      IBreak *b = core->get_break(BreakId(i));

      bool on = b->is_enabled();

      if (on && break_position[i] == slot && !(break_flags[i] & BREAK_HIDE))
        {
          breaks_id[count] = i;
          break_flags[i] &= ~BREAK_SKIP;
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

      ICore *core = CoreFactory::get_core();
      IBreak *b = core->get_break((BreakId)i);

      time_t time_left = b->get_limit() - b->get_elapsed_time();

      // Exclude break if not imminent.
      if (flags & BREAK_WHEN_IMMINENT && time_left > break_imminent_time[id] && force_duration == 0)
        {
          break_flags[id] |= BREAK_SKIP;
        }

      // update first imminent timer.
      if (!(flags & BREAK_SKIP) && (first_id == -1 || time_left < first))
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

      if (!(flags & BREAK_SKIP))
        {
          if (flags & BREAK_WHEN_FIRST && first_id != id && force_duration == 0)
            {
              break_flags[id] |= BREAK_SKIP;
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

      if (!(flags & BREAK_SKIP))
        {
          if (flags & BREAK_EXCLUSIVE && have_one && force_duration == 0)
            {
              break_flags[id] |= BREAK_SKIP;
            }

          have_one = true;
        }

      if (!(flags & BREAK_SKIP))
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

          if (flags & BREAK_DEFAULT && flags & BREAK_SKIP)
            {
              break_flags[id] &= ~BREAK_SKIP;
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

      if (!(flags & BREAK_SKIP))
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
      if (break_slot_cycle[i] >= BREAK_ID_SIZEOF || break_slots[i][break_slot_cycle[i]] == -1)
        {
          break_slot_cycle[i] = 0;
        }
    }
}

//! Reads the applet configuration.
void
TimerBoxControl::read_configuration()
{
  TRACE_ENTER("TimerBoxControl::read_configuration");
  cycle_time = get_cycle_time(name);
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakId bid = (BreakId)i;

      break_position[i] = get_timer_slot(name, bid);
      break_flags[i] = get_timer_flags(name, bid);
      break_imminent_time[i] = get_timer_imminent_time(name, bid);
    }
  TRACE_EXIT();
}

//! Callback that the configuration has changed.
void
TimerBoxControl::config_changed_notify(const string &key)
{
  (void)key;

  read_configuration();
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      break_slot_cycle[i] = 0;
    }

  reconfigure = true;
}

int
TimerBoxControl::get_cycle_time(string name)
{
  int ret;
  if (!CoreFactory::get_configurator()->get_value(
        TimerBoxControl::CFG_KEY_TIMERBOX + name + TimerBoxControl::CFG_KEY_TIMERBOX_CYCLE_TIME, ret))
    {
      ret = 10;
    }
  return ret;
}

void
TimerBoxControl::set_cycle_time(string name, int time)
{
  CoreFactory::get_configurator()->set_value(
    TimerBoxControl::CFG_KEY_TIMERBOX + name + TimerBoxControl::CFG_KEY_TIMERBOX_CYCLE_TIME, time);
}

const string
TimerBoxControl::get_timer_config_key(string name, BreakId timer, const string &key)
{
  ICore *core = CoreFactory::get_core();
  IBreak *break_data = core->get_break(BreakId(timer));

  return string(CFG_KEY_TIMERBOX) + name + "/" + break_data->get_name() + key;
}

int
TimerBoxControl::get_timer_imminent_time(string name, BreakId timer)
{
  const string key = get_timer_config_key(name, timer, CFG_KEY_TIMERBOX_IMMINENT);
  int ret;
  if (!CoreFactory::get_configurator()->get_value(key, ret))
    {
      ret = 30;
    }
  return ret;
}

void
TimerBoxControl::set_timer_imminent_time(string name, BreakId timer, int time)
{
  const string key = get_timer_config_key(name, timer, CFG_KEY_TIMERBOX_IMMINENT);
  CoreFactory::get_configurator()->set_value(key, time);
}

int
TimerBoxControl::get_timer_slot(string name, BreakId timer)
{
  const string key = get_timer_config_key(name, timer, CFG_KEY_TIMERBOX_POSITION);
  int ret;
  if (!CoreFactory::get_configurator()->get_value(key, ret))
    {
      if (name == "applet")
        {
          // All in one slot is probably the best default since we cannot assume
          // any users panel is large enough to hold all timers.
          ret = 0;
        }
      else
        {
          ret = timer;
        }
    }
  return ret;
}

void
TimerBoxControl::set_timer_slot(string name, BreakId timer, int slot)
{
  const string key = get_timer_config_key(name, timer, CFG_KEY_TIMERBOX_POSITION);
  CoreFactory::get_configurator()->set_value(key, slot);
}

int
TimerBoxControl::get_timer_flags(string name, BreakId timer)
{
  const string key = get_timer_config_key(name, timer, CFG_KEY_TIMERBOX_FLAGS);
  int ret;
  if (!CoreFactory::get_configurator()->get_value(key, ret))
    {
      ret = 0;
    }
  return ret;
}

void
TimerBoxControl::set_timer_flags(string name, BreakId timer, int flags)
{
  const string key = get_timer_config_key(name, timer, CFG_KEY_TIMERBOX_FLAGS);
  CoreFactory::get_configurator()->set_value(key, flags);
}

bool
TimerBoxControl::is_enabled(string name)
{
  bool ret = true;
  if (!CoreFactory::get_configurator()->get_value(CFG_KEY_TIMERBOX + name + CFG_KEY_TIMERBOX_ENABLED, ret))
    {
      ret = true;
    }
  return ret;
}

void
TimerBoxControl::set_enabled(string name, bool enabled)
{
  CoreFactory::get_configurator()->set_value(CFG_KEY_TIMERBOX + name + CFG_KEY_TIMERBOX_ENABLED, enabled);
}
