// TimerBoxControl.cc --- Timers Widgets
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Rob Caelers & Raymond Penners
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

#include <unistd.h>
#include <iostream>

#include "nls.h"
#include "debug.hh"

#include "TimerBoxControl.hh"
#include "TimeBarInterface.hh"
#include "Util.hh"
#include "Text.hh"

#include "CoreFactory.hh"
#include "TimerInterface.hh"
#include "BreakInterface.hh"
#include "ConfiguratorInterface.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManagerInterface.hh"
#endif


const std::string TimerBoxControl::CFG_KEY_TIMERBOX = "gui/";
const std::string TimerBoxControl::CFG_KEY_TIMERBOX_CYCLE_TIME = "/cycle_time";
const std::string TimerBoxControl::CFG_KEY_TIMERBOX_ENABLED = "/enabled";
const std::string TimerBoxControl::CFG_KEY_TIMERBOX_POSITION = "/position";
const std::string TimerBoxControl::CFG_KEY_TIMERBOX_FLAGS = "/flags";
const std::string TimerBoxControl::CFG_KEY_TIMERBOX_IMMINENT = "/imminent";


//! Constructor.
TimerBoxControl::TimerBoxControl(std::string n, TimerBoxView &v) :
  view(&v),
  cycle_time(10),
  name(n),
  force_duration(0),
  force_empty(false)
{
  init();
}
  


//! Destructor.
TimerBoxControl::~TimerBoxControl()
{
  ConfiguratorInterface *config = CoreFactory::get_configurator();
  config->remove_listener(this);
}


//! Updates the timerbox.
void
TimerBoxControl::update()
{
  CoreInterface *core = CoreFactory::get_core();
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
  view->update();
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

  CoreInterface *core = CoreFactory::get_core();
  
  // Listen for configugration changes.
  ConfiguratorInterface *config = CoreFactory::get_configurator();
  config->add_listener(TimerBoxControl::CFG_KEY_TIMERBOX + name, this);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakInterface *break_data = core->get_break(BreakId(i));
      config->add_listener("gui/breaks/"
                           + break_data->get_name()
                           + "/enabled", this);

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
  bool node_master = true;
  int num_peers = 0;
  
#ifdef HAVE_DISTRIBUTION
  CoreInterface *core = CoreFactory::get_core();
  DistributionManagerInterface *dist_manager = core->get_distribution_manager();
  
  if (dist_manager != NULL)
    {
      node_master = dist_manager->is_master();
      num_peers = dist_manager->get_number_of_peers();
    }
#endif

  //FIXME: duplicate
  char *labels[] = { _("Micro-break"), _("Rest break"), _("Daily limit") };
  string tip = "Workrave";
  
  for (int count = 0; count < BREAK_ID_SIZEOF; count++)
    {
      CoreInterface *core = CoreFactory::get_core();
      BreakInterface *break_data = core->get_break(BreakId(count));
      TimerInterface *timer = break_data->get_timer();

      std::string text;
      TimeBarInterface::ColorId primary_color;
      int primary_val, primary_max;
      TimeBarInterface::ColorId secondary_color;
      int secondary_val, secondary_max;

#ifdef LETS_SEE_HOW_WORKRAVE_BEHAVES_WITHOUT_THIS      
      if (!node_master && num_peers > 0)
        {
          text = _("Inactive");
          primary_color = TimeBarInterface::COLOR_ID_INACTIVE;
          secondary_color = TimeBarInterface::COLOR_ID_INACTIVE;
          primary_val = 0;
          primary_max = 60;
          secondary_val = 0;
          secondary_max = 60;
        }
      else
#endif
        {
          if (timer == NULL)
            {
              continue;
            }
      
          TimerInterface::TimerState timerState = timer->get_state();

          // Collect some data.
          time_t maxActiveTime = timer->get_limit();
          time_t activeTime = timer->get_elapsed_time();
          time_t breakDuration = timer->get_auto_reset();
          time_t idleTime = timer->get_elapsed_idle_time();
          bool overdue = (maxActiveTime < activeTime);

          // Set the text
          if (timer->is_limit_enabled() && maxActiveTime != 0)
            {
              text = Text::time_to_string(maxActiveTime - activeTime);
            }
          else
            {
              text = Text::time_to_string(activeTime);
            }

          tip += "\n";
          tip += labels[count];
          tip += ": " + text;
          
          // And set the bar.
          secondary_val = secondary_max = 0;
          secondary_color = TimeBarInterface::COLOR_ID_INACTIVE;

          if (timerState == TimerInterface::STATE_INVALID)
            {
              primary_color = TimeBarInterface::COLOR_ID_INACTIVE;
              primary_val = 0;
              primary_max = 60;
              text = _("Wait");
            }
          else
            {
              // Timer is running, show elapsed time.
              primary_val = activeTime;
              primary_max = maxActiveTime;
          
              primary_color = overdue
                ? TimeBarInterface::COLOR_ID_OVERDUE : TimeBarInterface::COLOR_ID_ACTIVE;

              if (//timerState == TimerInterface::STATE_STOPPED &&
                  timer->is_auto_reset_enabled() && breakDuration != 0)
                {
                  // resting.
                  secondary_color = TimeBarInterface::COLOR_ID_INACTIVE;
                  secondary_val = idleTime;
                  secondary_max = breakDuration;
                }
            }
        }
      view->set_time_bar(BreakId(count), text,
                         primary_color, primary_val, primary_max,
                         secondary_color, secondary_val, secondary_max);
    }
#waring FIXME: Tooltip code moved to GUI::get_tooltip(), still to be removed here
  view->set_tip(tip);
}


void
TimerBoxControl::init_icon()
{
  switch (operation_mode)
    {
    case OPERATION_MODE_NORMAL:
      view->set_icon(TimerBoxView::ICON_NORMAL);
      break;
      
    case OPERATION_MODE_SUSPENDED:
      view->set_icon(TimerBoxView::ICON_SUSPENDED);
      break;
      
    case OPERATION_MODE_QUIET:
      view->set_icon(TimerBoxView::ICON_QUIET);
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
      CoreInterface *core = CoreFactory::get_core();
      BreakInterface *break_data = core->get_break(BreakId(i));

      bool on = break_data->get_break_enabled();
      
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

      CoreInterface *core = CoreFactory::get_core();
      BreakInterface *break_data = core->get_break(BreakId(id));
      TimerInterface *timer = break_data->get_timer();

      time_t time_left = timer->get_limit() - timer->get_elapsed_time();
        
      // Exclude break if not imminent.
      if (flags & BREAK_WHEN_IMMINENT && time_left > break_imminent_time[id] &&
          force_duration == 0)
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
      if (break_slot_cycle[i] >= BREAK_ID_SIZEOF
          || break_slots[i][break_slot_cycle[i]] == -1)
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
      BreakId bid = (BreakId) i;
      
      break_position[i] = get_timer_slot(name, bid);;
      break_flags[i] = get_timer_flags(name, bid);
      break_imminent_time[i] = get_timer_imminent_time(name, bid);
    }
  view->set_enabled(is_enabled(name));
  TRACE_EXIT();
}


//! Callback that the configuration has changed.
void
TimerBoxControl::config_changed_notify(string key)
{
  (void) key;

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
  if (! CoreFactory::get_configurator()
      ->get_value(TimerBoxControl::CFG_KEY_TIMERBOX + name + TimerBoxControl::CFG_KEY_TIMERBOX_CYCLE_TIME, &ret))
    {
      ret = 10;
    }
  return ret;
}


void
TimerBoxControl::set_cycle_time(string name, int time)
{
  CoreFactory::get_configurator()
    ->set_value(TimerBoxControl::CFG_KEY_TIMERBOX + name + TimerBoxControl::CFG_KEY_TIMERBOX_CYCLE_TIME, time);
}


const string
TimerBoxControl::get_timer_config_key(string name, BreakId timer, const string &key)
{
  CoreInterface *core = CoreFactory::get_core();
  BreakInterface *break_data = core->get_break(BreakId(timer));

  return string(CFG_KEY_TIMERBOX) + name + "/" + break_data->get_name() + key;
}


int
TimerBoxControl::get_timer_imminent_time(string name, BreakId timer)
{
  const string key = get_timer_config_key(name, timer, CFG_KEY_TIMERBOX_IMMINENT);
  int ret;
  if (! CoreFactory::get_configurator()
      ->get_value(key, &ret))
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
  if (! CoreFactory::get_configurator()
      ->get_value(key, &ret))
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
  if (! CoreFactory::get_configurator()
      ->get_value(key, &ret))
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
  if (! CoreFactory::get_configurator()
      ->get_value(CFG_KEY_TIMERBOX + name + CFG_KEY_TIMERBOX_ENABLED, &ret))
    {
      ret = true;
    }
  return ret;
}


void
TimerBoxControl::set_enabled(string name, bool enabled)
{
  CoreFactory::get_configurator()
    ->set_value(CFG_KEY_TIMERBOX + name + CFG_KEY_TIMERBOX_ENABLED, enabled);
}
