// GUIControl.cc
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
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

#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>

#include "GUIControl.hh"
#include "Statistics.hh"
#include "BreakWindowInterface.hh"
#include "PreludeWindowInterface.hh"

#include "Util.hh"

#include "Configurator.hh"
#include "ControlInterface.hh"
#include "TimerInterface.hh"
#include "Control.hh"

#include "BreakControl.hh"

const string GUIControl::CFG_KEY_BREAKS = "gui/breaks";
const string GUIControl::CFG_KEY_BREAK = "gui/breaks/";
const string GUIControl::CFG_KEY_BREAK_MAX_PRELUDES = "/max_preludes";
const string GUIControl::CFG_KEY_BREAK_FORCE_AFTER_PRELUDES = "/force_after_preludes";
const string GUIControl::CFG_KEY_BREAK_IGNORABLE = "/ignorable_break";
const string GUIControl::CFG_KEY_BREAK_INSISTING = "/insist_break";
const string GUIControl::CFG_KEY_MAIN_WINDOW = "gui/main_window";
const string GUIControl::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP = "gui/main_window/always_on_top";

GUIControl *GUIControl::instance = NULL;

struct ConfigCheck
{
  const char *id;
  const char *name;
  int limit;
  int autoreset;
  bool countdown;
  bool countactivity;
  int snooze;
  string icon;
  string resetpred;

  int max_preludes;
  bool force_after_preludes;
  bool insist_break;
  bool ignorable_break;
  
} configCheck[] =
  {
    { "micro_pause",
      "Micro-pause",
      150, 30, false,	true,	150,
      "timer-micropause.png", "" ,
      3, true, false, false
    },
    {
      "rest_break",
      "Rest break",
      1200, 600, false,	true, 150,
      "timer-restbreak.png", "",
      3, true, true, true
    },
    {
      "daily_limit",
      "Daily limit",
      14400, 0,	false,	true, 150,
      "timer-daily.png", "day/4:00",
      0, true, true, true
    },
  };



//! GUIControl Constructor.
/*!
 *  \param controller interface to the controller.
 */
GUIControl::GUIControl(GUIFactoryInterface *factory, ControlInterface *controller)
{
  TRACE_ENTER("GUIControl:GUIControl");

  assert(! instance);
  instance = this;
  
  configurator = NULL;

  restbreak_control = NULL;
  micropause_control = NULL;

  be_quiet = false;
  gui_factory = factory;
  core_control = controller;

  TRACE_EXIT();
}


//! Destructor.
GUIControl::~GUIControl()
{
  TRACE_ENTER("GUIControl:~GUIControl");
  assert(instance);
  instance = NULL;

  if (restbreak_control != NULL)
    {
      delete restbreak_control;
    }

  if (micropause_control != NULL)
    {
      delete micropause_control;
    }

  if (configurator != NULL)
    {
      delete configurator;
    }
  
  TRACE_EXIT();
}


void
GUIControl::init()
{
  TRACE_ENTER("GUIControl:init");

  // FIXME: get_timer is a hack...will be fixed.
  micropause_control = new BreakControl(GUIControl::BREAK_ID_MICRO_PAUSE, core_control, gui_factory,
                                        core_control->get_timer("micro_pause"));
  micropause_control->set_prelude_text("Time for a micro-pause?");

  restbreak_control = new BreakControl(GUIControl::BREAK_ID_REST_BREAK, core_control, gui_factory,
                                       core_control->get_timer("rest_break"));
  restbreak_control->set_prelude_text("You need a rest break...");

  Statistics *stats = Statistics::get_instance();
  stats->load_current_day();
    
  load_config();
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      ConfigCheck *tc = &configCheck[i];
      TimerData *td = &timers[i];
      td->timer = core_control->get_timer(tc->id);
      td->icon = Util::complete_directory(tc->icon, Util::SEARCH_PATH_IMAGES);
      td->name = tc->name;
    }
  TRACE_EXIT();
}


//! Periodic heartbeat.
void
GUIControl::heartbeat()
{
  if (micropause_control && micropause_control->need_heartbeat())
    {
      micropause_control->heartbeat();
    }

  if (restbreak_control && restbreak_control->need_heartbeat())
    {
      restbreak_control->heartbeat();
    }

  update_statistics();
}


void
GUIControl::update_statistics()
{
  static int count = 0;

  if (count % 60 == 0)
    {
      TimerInterface *t = core_control->get_timer("daily_limit");
      assert(t != NULL);

      time_t elasped = t->get_elapsed_time();

      Statistics *stats = Statistics::get_instance();
      stats->set_total_active(elasped);
      stats->save_current_day();
    }

  count++;
}

void
GUIControl::set_quiet(bool quiet)
{
  TRACE_ENTER_MSG("GUIControl::set_quiet", quiet);
  be_quiet = quiet;

  if (quiet)
    {
      micropause_control->stop_break();
      restbreak_control->stop_break();
    }
  else
    {
      TimerInterface *mp_timer = timers[BREAK_ID_MICRO_PAUSE].timer;
      TimerInterface *rb_timer = timers[BREAK_ID_REST_BREAK].timer;
      
      if (rb_timer->get_next_limit_time() > 0 &&
          rb_timer->get_elapsed_time() >= rb_timer->get_limit())
        {
          restbreak_control->start_break();
        }
      else if (mp_timer->get_next_limit_time() > 0 &&
               mp_timer->get_elapsed_time() >= mp_timer->get_limit())
        {
          micropause_control->start_break();
        }
    }
  TRACE_EXIT();
}

 
//! Handles a timer action.
/*!
 *  \param break_id ID of the timer that perfomed an action.
 *  \param action action that was performed.
 */
void
GUIControl::break_action(BreakId id, BreakAction action)
{
  TRACE_ENTER("GUIControl::timer_action");

  if (be_quiet && action != BREAK_ACTION_FORCE_START_BREAK)
    {
      return;
    }
  
  BreakInterface *breaker = NULL;
  TimerInterface *timer = NULL;

  switch (id)
    {
    case BREAK_ID_MICRO_PAUSE:
      breaker = micropause_control;
      timer = timers[id].timer;
      break;
    case BREAK_ID_REST_BREAK:
      breaker = restbreak_control;
      timer = timers[id].timer;
      break;
    case BREAK_ID_DAILY_LIMIT:
      // FIXME: temp hack
      if (action == BREAK_ACTION_NATURAL_STOP_BREAK ||
          action == BREAK_ACTION_STOP_BREAK)
        {
          Statistics *stats = Statistics::get_instance();
          stats->start_new_day();
        }
      break; 
    default:
      break;
   }

  if (breaker != NULL && timer != NULL)
    {
      switch (action)
        {
        case BREAK_ACTION_START_BREAK:
          if (breaker->get_break_state() == BreakInterface::BREAK_INACTIVE)
            {
              handle_start_break(breaker, id, timer);
            }
          break;
        case BREAK_ACTION_STOP_BREAK:
          if (breaker->get_break_state() == BreakInterface::BREAK_ACTIVE)
            {
              handle_stop_break(breaker, id, timer);
            }
          break;
      
        case BREAK_ACTION_NATURAL_STOP_BREAK:
          {
            if (breaker->get_break_state() == BreakInterface::BREAK_ACTIVE)
              {
                handle_stop_break(breaker, id, timer);
              }
            Statistics *stats = Statistics::get_instance();
            stats->increment_counter(id, Statistics::STAT_TYPE_NATURAL_TAKEN);
          }
          break;
      
        case BREAK_ACTION_FORCE_START_BREAK:
          if (breaker->get_break_state() == BreakInterface::BREAK_INACTIVE)
            {
              // quick hack...
              if (id == BREAK_ID_REST_BREAK && micropause_control->get_break_state() == BreakInterface::BREAK_ACTIVE)
                {
                  micropause_control->stop_break();
                }
              breaker->force_start_break();
            }
          break;
        default:
          break;
        }
    }
  TRACE_EXIT();
}


//! Handles requestes to start the specified break.
/*!
 *  \param breaker Interface to the break that must be started.
 *  \param break_id ID of the timer that caused the break.
 *  \param timer Interface to the timer the caused the break.
 */
void
GUIControl::handle_start_break(BreakInterface *breaker, BreakId break_id, TimerInterface *timer)
{
  TRACE_ENTER("GUIControl::handle_start_break");

  // Don't show MP when RB is active.
  if (break_id == BREAK_ID_MICRO_PAUSE && restbreak_control->get_break_state() == BreakInterface::BREAK_ACTIVE)
    {
      // timer->snooze();
      TRACE_RETURN("RB Active, snoozing");
      return;
    }

  // Advance restbreak if it follows within 30s after the end of a micropause break
  if (break_id == BREAK_ID_MICRO_PAUSE)
    {
      TimerInterface *rbTimer = timers[BREAK_ID_REST_BREAK].timer;
      assert(rbTimer != NULL);
      
      // Only advance when
      // 1. we have a next limit reached time.
      // 2. timer is not yet over its limit. otherwise, it will interfere with snoozing.
      if (rbTimer->get_next_limit_time() > 0 &&
          rbTimer->get_elapsed_time() < rbTimer->get_limit())
        {
          int threshold = 30; // TODO: should be configurable
          time_t duration = timer->get_auto_reset();
          time_t now = time(NULL); // TODO: core_control->get_time();
          
          if (now + duration + threshold >= rbTimer->get_next_limit_time())
            {
              TRACE_MSG("Advancing RB " << duration << " + " << threshold <<
                        " = " << rbTimer->get_next_limit_time() - now);
              
              handle_start_break(restbreak_control, BREAK_ID_REST_BREAK, rbTimer);
              // timer->snooze();
              
              TRACE_RETURN("RB started, snoozing MP");
              return;
            }
        }
    }

  // Stop micropause when a restbreak starts. should not happend.
  // restbreak should be advanced.
  if (break_id == BREAK_ID_REST_BREAK && micropause_control->get_break_state() == BreakInterface::BREAK_ACTIVE)
    {
      micropause_control->stop_break();

      TimerInterface *mptimer = timers[BREAK_ID_MICRO_PAUSE].timer;
      assert(mptimer != NULL);
      mptimer->snooze_timer();
    }
  
  breaker->start_break();
  TRACE_EXIT();
}


//! Handles requests to stop the specified break.
/*!
 *  \param breaker Interface to the break that must be stopped.
 *  \param break_id ID of the timer that caused the break.
 *  \param timer Interface to the timer the caused the break.
 */
void
GUIControl::handle_stop_break(BreakInterface *breaker, BreakId break_id, TimerInterface *timer)
{
  (void) breaker;
  (void) break_id;
  (void) timer;
  
  breaker->stop_break();
}


//! Returns the configurator.
Configurator *
GUIControl::get_configurator()
{
  if (configurator == NULL)
    {
#if defined(HAVE_REGISTRY)
      configurator = Configurator::create("w32");
#elif defined(HAVE_GCONF)
      configurator = Configurator::create("gconf");
#elif defined(HAVE_GDOME)
      string configFile = Util::complete_directory("config.xml", Util::SEARCH_PATH_CONFIG);

      configurator = Configurator::create("xml");
      if (configFile != "")
        {
          configurator->load(configFile);
        }
#else
#error No configuator configured        
#endif
      bool changed = verify_config();

      if (changed)
        {
          configurator->save();
        }
    }

  return configurator;
}


bool
GUIControl::verify_config()
{
  TRACE_ENTER("GUIControl::verify_config");
  
  int size = sizeof(configCheck) / sizeof(ConfigCheck);
  bool changed = false;
  
  for (int i = 0; i < size; i++)
    {
      TRACE_MSG(configCheck[i].id);
      string pfx = ControlInterface::CFG_KEY_TIMER + string(configCheck[i].id);

      if (!configurator->exists_dir(pfx))
        {  
          changed = true;
          
          TRACE_MSG("set");
          configurator->set_value(pfx + ControlInterface::CFG_KEY_TIMER_LIMIT, configCheck[i].limit);
          configurator->set_value(pfx + ControlInterface::CFG_KEY_TIMER_AUTO_RESET, configCheck[i].autoreset);
          
          configurator->set_value(pfx + ControlInterface::CFG_KEY_TIMER_SNOOZE, configCheck[i].snooze);
          configurator->set_value(pfx + ControlInterface::CFG_KEY_TIMER_COUNT_ACTIVITY, configCheck[i].countactivity);
          configurator->set_value(pfx + ControlInterface::CFG_KEY_TIMER_RESTORE, true);

          if (configCheck[i].resetpred != "")
            {
              configurator->set_value(pfx + ControlInterface::CFG_KEY_TIMER_RESET_PRED, configCheck[i].resetpred);
            }
        }
    }

  TRACE_EXIT();
  return changed;
}


bool
GUIControl::load_config()
{
  TRACE_ENTER("GUIControl::load_config");
  
  int size = sizeof(configCheck) / sizeof(ConfigCheck);
  bool changed = false;
  
  for (int i = 0; i < size; i++)
    {
      TRACE_MSG(configCheck[i].id);

      int max_preludes = configCheck[i].max_preludes;
      bool force_after_preludes= configCheck[i].force_after_preludes;
      bool insist_break  = configCheck[i].insist_break;
      bool ignorable_break = configCheck[i].ignorable_break;

      string pfx = CFG_KEY_BREAK + string(configCheck[i].id);
      if (!configurator->exists_dir(pfx))
        {  
          changed = true;
          
          TRACE_MSG("set");
          configurator->set_value(pfx + CFG_KEY_BREAK_MAX_PRELUDES, max_preludes);
          configurator->set_value(pfx + CFG_KEY_BREAK_FORCE_AFTER_PRELUDES, force_after_preludes);
          configurator->set_value(pfx + CFG_KEY_BREAK_IGNORABLE, ignorable_break);
          configurator->set_value(pfx + CFG_KEY_BREAK_INSISTING, insist_break);
        }

      load_break_control_config(configCheck[i].id);
    }

  configurator->add_listener(CFG_KEY_BREAKS, this);
  
  TRACE_EXIT();
  return changed;
}


void
GUIControl::load_break_control_config(string name)
{
  TRACE_ENTER_MSG("GUIControl::load_break_control_config", name);
  BreakControl *bc = NULL;
  
  if (name == "micro_pause")
    {
      bc = micropause_control;
    }
  else if (name == "rest_break")
    {
      bc = restbreak_control;
    }

  if (bc != NULL)
    {
      TRACE_MSG(name);
      
      int max_preludes;
      bool force_after_preludes;
      bool insist_break;
      bool ignorable_break;
  
      string pfx = CFG_KEY_BREAK + name;

      configurator->get_value(pfx + CFG_KEY_BREAK_MAX_PRELUDES, &max_preludes);
      configurator->get_value(pfx + CFG_KEY_BREAK_FORCE_AFTER_PRELUDES, &force_after_preludes);
      configurator->get_value(pfx + CFG_KEY_BREAK_IGNORABLE, &ignorable_break);
      configurator->get_value(pfx + CFG_KEY_BREAK_INSISTING, &insist_break);
  
      bc->set_max_preludes(max_preludes);
      bc->set_force_after_preludes(force_after_preludes);
      bc->set_insist_break(insist_break);
      bc->set_ignorable_break(ignorable_break);
    }
  TRACE_EXIT();
}


void
GUIControl::config_changed_notify(string key)
{
  TRACE_ENTER_MSG("GUIControl:config_changed_notify", key);

  // Expected prefix
  string prefix = CFG_KEY_BREAK;

  // Search prefix (just in case some Configurator added a leading /
  std::string::size_type pos = key.rfind(prefix);

  if (pos != std::string::npos)
    {
      key = key.substr(pos + prefix.length());
    }

  TRACE_MSG(key);
  
  pos = key.find('/');

  string break_id;
  if (pos != std::string::npos)
    {
      break_id = key.substr(0, pos);
      key = key.substr(pos + 1);
    }

  load_break_control_config(break_id);

  TRACE_EXIT();
}

