// GUIControl.cc
//
// Copyright (C) 2001, 2002 Rob Caelers & Raymond Penners
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
#include "SoundPlayerInterface.hh"
#include "GUIFactoryInterface.hh"
#include "SoundPlayer.hh"

#include "Util.hh"
#include "nls.h"

#include "Configurator.hh"
#include "ControlInterface.hh"
#include "TimerInterface.hh"
#include "Control.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#include "PacketBuffer.hh"
#endif

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
  const char *label;
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
      3*60, 30, false,	true,	150,
      "timer-micropause.png", "" ,
      3, true, true, true
    },
    {
      "rest_break",
      "Rest break",
      45*60, 10*60, false,	true, 180,
      "timer-restbreak.png", "",
      3, true, true, true
    },
    {
      "daily_limit",
      "Daily limit",
      14400, 0,	false,	true, 150,
      "timer-daily.png", "day/4:00",
      3, true, true, true
    },
  };



GUIControl::TimerData::TimerData()
{
  timer = NULL;
  break_control = NULL;
}
    

int
GUIControl::TimerData::get_break_max_preludes() const
{
  bool b;
  int rc;
  b = GUIControl::get_instance()->get_configurator()
    ->get_value(CFG_KEY_BREAK
                + configCheck[break_id].id
                + CFG_KEY_BREAK_MAX_PRELUDES, &rc);
  if (! b)
    {
      rc = configCheck[break_id].max_preludes;
    }
  return rc;
}

bool
GUIControl::TimerData::get_break_force_after_preludes() const
{
  bool b;
  bool rc;
  b = GUIControl::get_instance()->get_configurator()
    ->get_value(CFG_KEY_BREAK
                + configCheck[break_id].id
                + CFG_KEY_BREAK_FORCE_AFTER_PRELUDES, &rc);
  if (! b)
    {
      rc = configCheck[break_id].force_after_preludes;
    }
  return rc;
}

bool
GUIControl::TimerData::get_break_ignorable() const
{
  bool b;
  bool rc;
  b = GUIControl::get_instance()->get_configurator()
    ->get_value(CFG_KEY_BREAK
                + configCheck[break_id].id
                + CFG_KEY_BREAK_IGNORABLE, &rc);
  if (! b)
    {
      rc = configCheck[break_id].ignorable_break;
    }
  return rc;
}

bool
GUIControl::TimerData::get_break_insisting() const
{
  bool b;
  bool rc;
  b = GUIControl::get_instance()->get_configurator()
    ->get_value(CFG_KEY_BREAK
                + configCheck[break_id].id
                + CFG_KEY_BREAK_INSISTING, &rc);
  if (! b)
    {
      rc = configCheck[break_id].insist_break;
    }
  return rc;
}

void
GUIControl::TimerData::set_break_max_preludes(int n)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_BREAK
                + configCheck[break_id].id
                + CFG_KEY_BREAK_MAX_PRELUDES, n);
}

void
GUIControl::TimerData::set_break_force_after_preludes(bool b)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_BREAK
                + configCheck[break_id].id
                + CFG_KEY_BREAK_FORCE_AFTER_PRELUDES, b);
}

void
GUIControl::TimerData::set_break_ignorable(bool b)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_BREAK
                + configCheck[break_id].id
                + CFG_KEY_BREAK_IGNORABLE, b);
}

void
GUIControl::TimerData::set_break_insisting(bool b)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_BREAK
                + configCheck[break_id].id
                + CFG_KEY_BREAK_INSISTING, b);
}


//! GUIControl Constructor.
/*!
 *  \param controller interface to the controller.
 */
GUIControl::GUIControl(GUIFactoryInterface *factory, ControlInterface *controller)
{
  TRACE_ENTER("GUIControl:GUIControl");

  assert(! instance);
  instance = this;
  
  statistics = NULL;
  configurator = NULL;

  master_node = true;
  operation_mode = OPERATION_MODE_NORMAL;
  gui_factory = factory;
  core_control = controller;
  sound_player = NULL;

  TRACE_EXIT();
}


//! Destructor.
GUIControl::~GUIControl()
{
  TRACE_ENTER("GUIControl:~GUIControl");
  assert(instance);
  instance = NULL;

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (timers[i].break_control != NULL)
        {
          delete timers[i].break_control;
        }
    }
  if (configurator != NULL)
    {
      delete configurator;
    }
  if (sound_player != NULL)
    {
      delete sound_player;
    }
  if (statistics != NULL)
    {
      delete statistics;
    }
  TRACE_EXIT();
}


void
GUIControl::init()
{
  TRACE_ENTER("GUIControl:init");

  // FIXME: get_timer is a hack...will be fixed.
  BreakControl *micropause_control = new BreakControl(GUIControl::BREAK_ID_MICRO_PAUSE, core_control, gui_factory,
                                        core_control->get_timer("micro_pause"));
  micropause_control->set_prelude_text(_("Time for a micro-pause?"));

  BreakControl *restbreak_control = new BreakControl(GUIControl::BREAK_ID_REST_BREAK, core_control, gui_factory,
                                       core_control->get_timer("rest_break"));
  restbreak_control->set_prelude_text(_("You need a rest break..."));

  Statistics *stats = Statistics::get_instance();
  stats->init(core_control);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      ConfigCheck *tc = &configCheck[i];
      TimerData *td = &timers[i];
      td->timer = core_control->get_timer(tc->id);
      td->icon = Util::complete_directory(tc->icon, Util::SEARCH_PATH_IMAGES);
      td->break_id = i;
      td->break_name = tc->id;
      td->label = tc->label;
    }

  timers[BREAK_ID_MICRO_PAUSE].break_control = micropause_control;
  timers[BREAK_ID_REST_BREAK].break_control = restbreak_control;

  // FIXME: Raymond??
  load_config();

  sound_player = new SoundPlayer(gui_factory->create_sound_player());

#ifdef HAVE_DISTRIBUTION
  init_distribution_manager();
#endif
  
  TRACE_EXIT();
}


//! Periodic heartbeat.
void
GUIControl::heartbeat()
{
  // Distributed  stuff

#ifdef HAVE_DISTRIBUTION
  bool new_master_node = true;
  DistributionManager *dist_manager = DistributionManager::get_instance();
  if (dist_manager != NULL)
    {
      new_master_node = dist_manager->is_master();
    }

  if (master_node != new_master_node)
    {
      master_node = new_master_node;
      if (!master_node)
        {
          stop_all_breaks();
        }
    }
#endif

  if (master_node)
    {
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          BreakControl *bc = timers[i].break_control;
          if (bc != NULL
              && bc->need_heartbeat())
            {
              bc->heartbeat();
            }
        }

      update_statistics();
    }
}


void
GUIControl::update_statistics()
{
  static int count = 0;

  if (count % 60 == 0)
    {
      Statistics *stats = Statistics::get_instance();
      stats->heartbeat();
    }

  count++;
}

void
GUIControl::stop_all_breaks()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakControl *bc = timers[i].break_control;
      if (bc != NULL)
        {
          bc->stop_break();
        }
    }
}


void
GUIControl::restart_break()
{
  for (int i = BREAK_ID_SIZEOF - 1; i >= 0; i--)
    {
      TimerInterface *t = timers[i].timer;
      BreakControl *bc = timers[i].break_control;
      if (bc != NULL && t != NULL)
        {
          if (t->get_next_limit_time() > 0
              && t->get_elapsed_time() >= t->get_limit())
            {
              bc->start_break();
              break;
            }
        }
    }
}

void
GUIControl::set_freeze_all_breaks(bool freeze)
{
  TRACE_ENTER_MSG("GUIControl::set_freeze_all_breaks", freeze);
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      TimerInterface *t = timers[i].timer;
      assert(t != NULL);
      if (!t->has_activity_monitor())
        {
          t->freeze_timer(freeze);
        }
    }
  TRACE_EXIT();
}


GUIControl::OperationMode
GUIControl::set_operation_mode(OperationMode mode)
{
  TRACE_ENTER_MSG("GUIControl::set_operation_mode", mode);

  OperationMode previous_mode = operation_mode;  

  if (operation_mode != mode)
    {
      operation_mode = mode;

      if (operation_mode == OPERATION_MODE_SUSPENDED ||
          operation_mode == OPERATION_MODE_QUIET)
        {
          stop_all_breaks();
        }

      ActivityMonitorInterface *monitor = core_control->get_activity_monitor();
      assert(monitor != NULL);

      if (operation_mode == OPERATION_MODE_SUSPENDED)
        {
          monitor->force_idle();
          monitor->suspend();
        }
      else if (previous_mode == OPERATION_MODE_SUSPENDED)
        {
          monitor->resume();
        }
    }

  TRACE_EXIT();
  return previous_mode;
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

  if (operation_mode == OPERATION_MODE_QUIET && action != BREAK_ACTION_FORCE_START_BREAK)
    {
      return;
    }
  
  BreakInterface *breaker = timers[id].break_control;
  TimerInterface *timer = timers[id].timer;
  if (id == BREAK_ID_DAILY_LIMIT)
    {
      // FIXME: temp hack
      if (action == BREAK_ACTION_NATURAL_STOP_BREAK ||
          action == BREAK_ACTION_STOP_BREAK)
        {
          Statistics *stats = Statistics::get_instance();
          stats->start_new_day();
        }
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
            stats->increment_break_counter(id, Statistics::STATS_BREAKVALUE_NATURAL_TAKEN);
          }
          break;
      
        case BREAK_ACTION_FORCE_START_BREAK:
          {
            // quick hack...
            BreakControl *micropause_control
              = timers[BREAK_ID_MICRO_PAUSE].break_control;
            if (id == BREAK_ID_REST_BREAK
                && (micropause_control->get_break_state()
                    == BreakInterface::BREAK_ACTIVE))
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
  BreakControl *restbreak_control, *micropause_control;
  restbreak_control = timers[BREAK_ID_REST_BREAK].break_control;
  micropause_control = timers[BREAK_ID_MICRO_PAUSE].break_control;
  
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
      load_break_control_config(i);
    }

  configurator->add_listener(CFG_KEY_BREAKS, this);
  
  TRACE_EXIT();
  return changed;
}


void
GUIControl::load_break_control_config(string break_name)
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (timers[i].break_name == break_name)
        {
          load_break_control_config(i);
          return;
        }
    }
}

void
GUIControl::load_break_control_config(int break_id)
{
  TRACE_ENTER_MSG("GUIControl::load_break_control_config", break_id);
  TimerData *timer = &timers[break_id];
  BreakControl *bc = timers[break_id].break_control;
  
  if (bc != NULL)
    {
      bc->set_max_preludes(timer->get_break_max_preludes());
      bc->set_force_after_preludes(timer->get_break_force_after_preludes());
      bc->set_insist_break(timer->get_break_insisting());
      bc->set_ignorable_break(timer->get_break_ignorable());
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




#ifdef HAVE_DISTRIBUTION
// Create the monitor based on the specified configuration.
void
GUIControl::init_distribution_manager()
{
  DistributionManager *dist_manager = DistributionManager::get_instance();

  if (dist_manager != NULL)
    {
      dist_manager->register_state(DISTR_STATE_BREAKS,  this);
    }
}

bool
GUIControl::get_state(DistributedStateID id, unsigned char **buffer, int *size)
{
  TRACE_ENTER("GUIControl::get_state");

  PacketBuffer state_packet;
  state_packet.create();
  state_packet.pack_ushort(BREAK_ID_SIZEOF);
  
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakControl *bi = timers[i].break_control;

      BreakInterface::BreakStateData state_data;

      if (bi != NULL)
        {
          bi->get_state_data(state_data);
      
          int pos = state_packet.bytes_written();

          state_packet.pack_ushort(0);
          state_packet.pack_byte((guint8)state_data.forced_break);
          state_packet.pack_byte((guint8)state_data.final_prelude);
          state_packet.pack_ulong((guint32)state_data.prelude_count);
          state_packet.pack_ulong((guint32)state_data.break_stage);
          state_packet.pack_ulong((guint32)state_data.prelude_time);
      
          state_packet.poke_ushort(pos, state_packet.bytes_written() - pos);
        }
      else
        {
          state_packet.pack_ushort(0);
        }
    }

  *size = state_packet.bytes_written();
  *buffer = new unsigned char[*size + 1];
  memcpy(*buffer, state_packet.get_buffer(), *size);

  TRACE_EXIT();
  return true;
}

bool
GUIControl::set_state(DistributedStateID id, bool master, unsigned char *buffer, int size)
{
  TRACE_ENTER("GUIControl::set_state");

  PacketBuffer state_packet;
  state_packet.create();

  state_packet.pack_raw(buffer, size);
  
  int num_breaks = state_packet.unpack_ushort();

  if (num_breaks > BREAK_ID_SIZEOF)
    {
      TRACE_MSG("More breaks received");
      num_breaks = BREAK_ID_SIZEOF;
    }
      
  for (int i = 0; i < num_breaks; i++)
    {
      BreakControl *bi = timers[i].break_control;
      
      BreakInterface::BreakStateData state_data;

      int data_size = state_packet.unpack_ushort();

      if (data_size > 0)
        {
          state_data.forced_break = state_packet.unpack_byte();
          state_data.final_prelude = state_packet.unpack_byte();
          state_data.prelude_count = state_packet.unpack_ulong();
          state_data.break_stage = state_packet.unpack_ulong();
          state_data.prelude_time = state_packet.unpack_ulong();

          bi->set_state_data(master, state_data);
        }
    }
  
  TRACE_EXIT();
  return true;
}
#endif
