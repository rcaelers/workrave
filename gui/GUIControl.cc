// GUIControl.cc
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
#include "nls.h"

#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>

#include "GUIControl.hh"

#include "BreakControl.hh"
#include "BreakWindowInterface.hh"
#include "Configurator.hh"
#include "Control.hh"
#include "ControlInterface.hh"
#include "GUIFactoryInterface.hh"
#include "SoundPlayer.hh"
#include "SoundPlayerInterface.hh"
#include "Statistics.hh"
#include "TimerInterface.hh"
#include "Util.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#include "PacketBuffer.hh"
#endif


const string GUIControl::CFG_KEY_BREAKS = "gui/breaks";
const string GUIControl::CFG_KEY_BREAK = "gui/breaks/";
const string GUIControl::CFG_KEY_BREAK_MAX_PRELUDES = "/max_preludes";
const string GUIControl::CFG_KEY_BREAK_FORCE_AFTER_PRELUDES = "/force_after_preludes";
const string GUIControl::CFG_KEY_BREAK_IGNORABLE = "/ignorable_break";
const string GUIControl::CFG_KEY_BREAK_INSISTING = "/insist_break";
const string GUIControl::CFG_KEY_BREAK_ENABLED = "/enabled";
#ifdef HAVE_EXERCISES
const string GUIControl::CFG_KEY_BREAK_EXERCISES = "/exercises";
#endif

GUIControl *GUIControl::instance = NULL;

struct ConfigCheck
{
  const char *id;
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
#ifdef HAVE_EXERCISES
  int exercises;
#endif
} default_config[] =
  {
    { "micro_pause",
      3*60, 30, false,	true,	150,
      "timer-micropause.png", "" ,
      3, true, true, true
#ifdef HAVE_EXERCISES
      , 0
#endif
    },
    {
      "rest_break",
      45*60, 10*60, false,	true, 180,
      "timer-restbreak.png", "",
      3, true, true, true
#ifdef HAVE_EXERCISES
      , 3
#endif
    },
    {
      "daily_limit",
      14400, 0,	false,	true, 20 * 60,
      "timer-daily.png", "day/4:00",
      3, true, true, true
#ifdef HAVE_EXERCISES
      , 0
#endif
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
                + default_config[break_id].id
                + CFG_KEY_BREAK_MAX_PRELUDES, &rc);
  if (! b)
    {
      rc = default_config[break_id].max_preludes;
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
                + default_config[break_id].id
                + CFG_KEY_BREAK_FORCE_AFTER_PRELUDES, &rc);
  if (! b)
    {
      rc = default_config[break_id].force_after_preludes;
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
                + default_config[break_id].id
                + CFG_KEY_BREAK_IGNORABLE, &rc);
  if (! b)
    {
      rc = default_config[break_id].ignorable_break;
    }
  return rc;
}

#ifdef HAVE_EXERCISES
int
GUIControl::TimerData::get_break_exercises() const
{
  bool b;
  int rc;
  b = GUIControl::get_instance()->get_configurator()
    ->get_value(CFG_KEY_BREAK
                + default_config[break_id].id
                + CFG_KEY_BREAK_EXERCISES, &rc);
  if (! b)
    {
      rc = default_config[break_id].exercises;
    }
  return rc;
}

void
GUIControl::TimerData::set_break_exercises(int n)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_BREAK
                + default_config[break_id].id
                + CFG_KEY_BREAK_EXERCISES, n);
}

#endif

bool
GUIControl::TimerData::get_break_insisting() const
{
  bool b;
  bool rc;
  b = GUIControl::get_instance()->get_configurator()
    ->get_value(CFG_KEY_BREAK
                + default_config[break_id].id
                + CFG_KEY_BREAK_INSISTING, &rc);
  if (! b)
    {
      rc = default_config[break_id].insist_break;
    }
  return rc;
}

bool
GUIControl::TimerData::get_break_enabled() const
{
  bool b;
  bool rc;
  b = GUIControl::get_instance()->get_configurator()
    ->get_value(CFG_KEY_BREAK
                + default_config[break_id].id
                + CFG_KEY_BREAK_ENABLED, &rc);
  if (! b)
    {
      rc = true;
    }
  return rc;
}


void
GUIControl::TimerData::set_break_max_preludes(int n)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_BREAK
                + default_config[break_id].id
                + CFG_KEY_BREAK_MAX_PRELUDES, n);
}


void
GUIControl::TimerData::set_break_force_after_preludes(bool b)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_BREAK
                + default_config[break_id].id
                + CFG_KEY_BREAK_FORCE_AFTER_PRELUDES, b);
}


void
GUIControl::TimerData::set_break_ignorable(bool b)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_BREAK
                + default_config[break_id].id
                + CFG_KEY_BREAK_IGNORABLE, b);
}


void
GUIControl::TimerData::set_break_insisting(bool b)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_BREAK
                + default_config[break_id].id
                + CFG_KEY_BREAK_INSISTING, b);
}


void
GUIControl::TimerData::set_break_enabled(bool b)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_BREAK
                + default_config[break_id].id
                + CFG_KEY_BREAK_ENABLED, b);
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
  Statistics *stats = Statistics::get_instance();
  stats->init(core_control);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      ConfigCheck *tc = &default_config[i];
      TimerData *td = &timers[i];
      td->timer = core_control->create_timer(i, tc->id);
      td->icon = Util::complete_directory(tc->icon, Util::SEARCH_PATH_IMAGES);
      td->break_id = i;
      td->break_name = tc->id;
      
      switch (i)
        {
        case BREAK_ID_MICRO_PAUSE:
          td->break_control = new BreakControl
            (GUIControl::BREAK_ID_MICRO_PAUSE, core_control, gui_factory, td->timer);
          td->break_control->set_prelude_text(_("Time for a micro-pause?"));
          td->label = _("Micro-pause");
          break;
          
        case BREAK_ID_REST_BREAK:
          td->break_control = new BreakControl
            (GUIControl::BREAK_ID_REST_BREAK, core_control, gui_factory, td->timer);
          td->break_control->set_prelude_text(_("You need a rest break..."));
          td->label = _("Rest break");
          break;
          
        case BREAK_ID_DAILY_LIMIT:
          td->break_control = new BreakControl
            (GUIControl::BREAK_ID_DAILY_LIMIT, core_control, gui_factory, td->timer);
          td->break_control->set_prelude_text(_("You should stop for today..."));
          td->label = _("Daily limit");
          break;
          
        default:
          td->break_control = NULL;
        }
    }
 
  core_control->init_timers();
  load_config();

  sound_player = new SoundPlayer(gui_factory->create_sound_player());

#ifdef HAVE_DISTRIBUTION
  init_distribution_manager();
#endif
}



//! Notication of a timer action.
/*!
 *  \param timerId ID of the timer that caused the action.
 *  \param action action that is performed by the timer.
*/
void
GUIControl::timer_action(BreakId id, TimerInfo info)
{
  // Parse action.
  if (info.event == TIMER_EVENT_LIMIT_REACHED)
    {
      break_action(id, GUIControl::BREAK_ACTION_START_BREAK);
    }
  else if (info.event == TIMER_EVENT_RESET)
    {
      break_action(id, GUIControl::BREAK_ACTION_STOP_BREAK);
    }
  else if (info.event == TIMER_EVENT_NATURAL_RESET)
    {
      break_action(id, GUIControl::BREAK_ACTION_NATURAL_STOP_BREAK);
    }
}


//! Periodic heartbeat.
void
GUIControl::heartbeat()
{
  TimerInfo infos[BREAK_ID_SIZEOF];

  for (int i = 0; i <= BREAK_ID_SIZEOF; i++)
    {
      infos[i].enabled = timers[i].enabled;
    }

  core_control->process_timers(infos);

  for (int i = BREAK_ID_SIZEOF - 1; i >= 0;  i--)
    {
      TimerInfo info = infos[i];
      if (timers[i].enabled)
        {
          timer_action((BreakId)i, info);
          
          if (i == BREAK_ID_DAILY_LIMIT)
            {
              if (info.event == TIMER_EVENT_NATURAL_RESET ||
                  info.event == TIMER_EVENT_RESET)
                {
                  Statistics *stats = Statistics::get_instance();
                  stats->set_counter(Statistics::STATS_VALUE_TOTAL_ACTIVE_TIME, info.elapsed_time);
                  stats->start_new_day();
                  
                  daily_reset();
                }
            }
        }

    }

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
GUIControl::daily_reset()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      TimerInterface *t = timers[i].timer;
      assert(t != NULL);
      
      int overdue = t->get_total_overdue_time();

      Statistics *stats = Statistics::get_instance();
      stats->set_break_counter(((GUIControl::BreakId)i),
                               Statistics::STATS_BREAKVALUE_TOTAL_OVERDUE, overdue);

      t->daily_reset_timer();
    }

  ActivityMonitorInterface *monitor = core_control->get_activity_monitor();
  assert(monitor != NULL);
  monitor->reset_statistics();
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
      if (timers[i].enabled && bc != NULL && t != NULL)
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
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      TimerInterface *t = timers[i].timer;
      assert(t != NULL);
      if (!t->has_activity_monitor())
        {
          t->freeze_timer(freeze);
        }
    }
}


GUIControl::OperationMode
GUIControl::get_operation_mode()
{
  return operation_mode;
}


GUIControl::OperationMode
GUIControl::set_operation_mode(OperationMode mode)
{
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
  if (operation_mode == OPERATION_MODE_QUIET && action != BREAK_ACTION_FORCE_START_BREAK)
    {
      return;
    }
  
  BreakInterface *breaker = timers[id].break_control;
  TimerInterface *timer = timers[id].timer;

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
  // Don't show MP when RB is active, RB when DL is active.
  for (int bi = break_id; bi <= BREAK_ID_DAILY_LIMIT; bi++)
    {
      if (timers[bi].break_control->get_break_state()
          == BreakInterface::BREAK_ACTIVE)
        {
          return;
        }
    }
  

  // FIXME: how does this relate to daily limit?

  // Advance restbreak if it follows within 30s after the end of a micropause break
  BreakControl *restbreak_control;
  restbreak_control = timers[BREAK_ID_REST_BREAK].break_control;

  if (break_id == BREAK_ID_MICRO_PAUSE && timers[BREAK_ID_REST_BREAK].enabled)
    {
      TimerInterface *rbTimer = timers[BREAK_ID_REST_BREAK].timer;
      assert(rbTimer != NULL);
      
      // Only advance when
      // 1. we have a next limit reached time.
      // 2. timer is not yet over its limit. otherwise, it will interfere with snoozing.
      if (rbTimer->get_next_limit_time() > 0 /* &&
                                                rbTimer->get_elapsed_time() < rbTimer->get_limit() */)
        {
          int threshold = 30; // TODO: should be configurable
          time_t duration = timer->get_auto_reset();
          time_t now = time(NULL); // TODO: core_control->get_time();
          
          if (now + duration + threshold >= rbTimer->get_next_limit_time())
            {
              handle_start_break(restbreak_control, BREAK_ID_REST_BREAK, rbTimer);

              // Snooze timer before the limit was reached. Just to make sure
              // that it doesn't reach its limit again when elapsed == limit
              rbTimer->snooze_timer();
              return;
            }
        }
    }

  // Stop micropause when a restbreak starts. should not happend.
  // restbreak should be advanced.
  for (int bi = BREAK_ID_MICRO_PAUSE; bi < break_id; bi++)
    {
      TimerData *t = &timers[bi];
      if (t->break_control->get_break_state() == BreakInterface::BREAK_ACTIVE)
        {
          t->break_control->stop_break();
        }
    }
  
  breaker->start_break();
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


Configurator *
GUIControl::get_configurator()
{
  if (configurator == NULL)
    {
      configurator = gui_factory->create_configurator();
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
  int size = sizeof(default_config) / sizeof(ConfigCheck);
  bool changed = false;
  
  for (int i = 0; i < size; i++)
    {
      string pfx = ControlInterface::CFG_KEY_TIMER + string(default_config[i].id);

      if (!configurator->exists_dir(pfx))
        {  
          changed = true;
          
          configurator->set_value(pfx + ControlInterface::CFG_KEY_TIMER_LIMIT, default_config[i].limit);
          configurator->set_value(pfx + ControlInterface::CFG_KEY_TIMER_AUTO_RESET, default_config[i].autoreset);
          
          configurator->set_value(pfx + ControlInterface::CFG_KEY_TIMER_SNOOZE, default_config[i].snooze);
          configurator->set_value(pfx + ControlInterface::CFG_KEY_TIMER_COUNT_ACTIVITY, default_config[i].countactivity);
          configurator->set_value(pfx + ControlInterface::CFG_KEY_TIMER_RESTORE, true);

          if (default_config[i].resetpred != "")
            {
              configurator->set_value(pfx + ControlInterface::CFG_KEY_TIMER_RESET_PRED, default_config[i].resetpred);
            }
        }
    }

  return changed;
}


bool
GUIControl::load_config()
{
  int size = sizeof(default_config) / sizeof(ConfigCheck);
  bool changed = false;
  
  for (int i = 0; i < size; i++)
    {
      load_break_control_config(i);
    }

  configurator->add_listener(CFG_KEY_BREAKS, this);
  
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
  TimerData *timer = &timers[break_id];
  BreakControl *bc = timers[break_id].break_control;
  
  if (bc != NULL)
    {
      bc->set_max_preludes(timer->get_break_max_preludes());
      bc->set_force_after_preludes(timer->get_break_force_after_preludes());
      bc->set_insist_break(timer->get_break_insisting());
      bc->set_ignorable_break(timer->get_break_ignorable());
    }

  timer->enabled = timer->get_break_enabled();
}


void
GUIControl::config_changed_notify(string key)
{
  // Expected prefix
  string prefix = CFG_KEY_BREAK;

  // Search prefix (just in case some Configurator added a leading /)
  std::string::size_type pos = key.rfind(prefix);

  if (pos != std::string::npos)
    {
      key = key.substr(pos + prefix.length());
    }

  pos = key.find('/');

  string break_id;
  if (pos != std::string::npos)
    {
      break_id = key.substr(0, pos);
      key = key.substr(pos + 1);
    }

  load_break_control_config(break_id);
}


GUIControl::TimerData *
GUIControl::get_timer_data(GUIControl::BreakId id)
{
  return &timers[id];
}



#ifdef HAVE_DISTRIBUTION
// Create the monitor based on the specified configuration.
void
GUIControl::init_distribution_manager()
{
  DistributionManager *dist_manager = DistributionManager::get_instance();

  if (dist_manager != NULL)
    {
      dist_manager->register_client_message(DCM_BREAKS, DCMT_MASTER, this);
    }
}


bool
GUIControl::request_client_message(DistributionClientMessageID id, unsigned char **buffer, int *size)
{
  (void) id;
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

  return true;
}


bool
GUIControl::client_message(DistributionClientMessageID id, bool master, char *client_id, unsigned char *buffer, int size)
{
  (void) id;
  (void) client_id;
  PacketBuffer state_packet;
  state_packet.create();

  state_packet.pack_raw(buffer, size);
  
  int num_breaks = state_packet.unpack_ushort();

  if (num_breaks > BREAK_ID_SIZEOF)
    {
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
  
  return true;
}
#endif
