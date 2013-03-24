// BreaksControl.cc --- The main controller
//
// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#include "debug.hh"

#include <iostream>
#include <fstream>
#include <sstream>

#include "BreaksControl.hh"
#include "TimerActivityMonitor.hh"

#include "Util.hh"
#include "utils/TimeSource.hh"
#include "dbus/DBus.hh"

static const char *WORKRAVESTATE="WorkRaveState";
static const int SAVESTATETIME = 60;

using namespace workrave::utils;
using namespace workrave::dbus;

BreaksControl::Ptr
BreaksControl::create(IApp *app,
                      IActivityMonitor::Ptr activity_monitor,
                      Statistics::Ptr statistics,
                      IConfigurator::Ptr configurator,
                      DBus::Ptr dbus)
{
  return Ptr(new BreaksControl(app,
                               activity_monitor,
                               statistics,
                               configurator,
                               dbus));
}


//! Construct a new Break Controller.
BreaksControl::BreaksControl(IApp *app,
                             IActivityMonitor::Ptr activity_monitor,
                             Statistics::Ptr statistics,
                             IConfigurator::Ptr configurator,
                             DBus::Ptr dbus) :
  application(app),
  activity_monitor(activity_monitor),
  statistics(statistics),
  configurator(configurator),
  dbus(dbus),
  insist_policy(ICore::INSIST_POLICY_HALT),
  active_insist_policy(ICore::INSIST_POLICY_INVALID)
{
  TRACE_ENTER("BreaksControl::BreaksControl");
  TRACE_EXIT();
}


//! Destructor.
BreaksControl::~BreaksControl()
{
  TRACE_ENTER("BreaksControl::~BreaksControl");
  save_state();
  TRACE_EXIT();
}


/********************************************************************************/
/**** Initialization                                                       ******/
/********************************************************************************/


//! Initializes the breaks controller.
void
BreaksControl::init()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      breaks[i] = Break::create(BreakId(i),
                                application,
                                shared_from_this(),
                                activity_monitor,
                                statistics,
                                configurator,
                                dbus);
      breaks[i]->init();
    }

  load_state();
}


//! Returns the specified timer.
Timer::Ptr
BreaksControl::get_timer(string name) const
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (breaks[i]->get_name() == name)
        {
          return breaks[i]->get_timer();
        }
    }
  return Timer::Ptr();
}


//! Returns the specified timer.
Timer::Ptr
BreaksControl::get_timer(int id) const
{
  return breaks[id]->get_timer();
}


//! Returns the specified break controller.
IBreak::Ptr
BreaksControl::get_break(BreakId id)
{
  assert(id >= 0 && id < BREAK_ID_SIZEOF);
  return breaks[id];
}


//! Sets the usage mode.
void
BreaksControl::set_usage_mode(UsageMode mode)
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      breaks[i]->set_usage_mode(mode);
    }
}


//! Sets the insensitive mode.
void
BreaksControl::set_insensitive_mode(InsensitiveMode mode)
{
  for (int i = 0; i < BREAK_ID_SIZEOF; ++i)
    {
      get_timer(i)->set_insensitive_mode(mode);
    }
}


//! Sets the freeze state of all breaks.
void
BreaksControl::set_freeze_all_breaks(bool freeze)
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      breaks[i]->freeze_break(freeze);
    }
}


//! Stops all breaks.
void
BreaksControl::stop_all_breaks()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      breaks[i]->stop_break();
    }
}


//! Forces the start of the specified break.
void
BreaksControl::force_break(BreakId id, BreakHint break_hint)
{
  TRACE_ENTER_MSG("BreaksControl::force_break", id << " " << break_hint);
  
  if (id == BREAK_ID_REST_BREAK && breaks[BREAK_ID_MICRO_BREAK]->is_active())
    {
      breaks[BREAK_ID_MICRO_BREAK]->stop_break();
      TRACE_MSG("Resuming Micro break");
    }

  breaks[id]->force_start_break(break_hint);
  TRACE_EXIT();
}


void
BreaksControl::resume_reading_mode_timers()
{
  for (int i = BREAK_ID_MICRO_BREAK; i < BREAK_ID_SIZEOF; i++)
    {
      get_timer(i)->restart_insensitive_timer();
    }
}


IActivityMonitor::Ptr
BreaksControl::create_timer_activity_monitor(const string &break_name)
{
  Timer::Ptr master = get_timer(break_name);
  if (master)
    {
      return TimerActivityMonitor::create(activity_monitor, master);
    }
  return IActivityMonitor::Ptr();
}


/********************************************************************************/
/**** Break handling                                                       ******/
/********************************************************************************/

//! Periodic heartbeat.
void
BreaksControl::heartbeat()
{
  TRACE_ENTER("BreaksControl::heartbeat");

  // Perform timer processing.
  process_timers();

  // Send heartbeats to other components.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      breaks[i]->process_break();
    }

  // Make state persistent.
  if (TimeSource::get_monotonic_time_sec() % SAVESTATETIME == 0)
    {
      statistics->update();
      save_state();
    }

  TRACE_EXIT();
}


//! Processes all timers.
void
BreaksControl::process_timers()
{
  TRACE_ENTER("BreaksControl::process_timers");

  TimerEvent events[BREAK_ID_SIZEOF];

  // Process Timers.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      events[i] = breaks[i]->process_timer();
    }

  // Process all timer events.
  for (int i = BREAK_ID_SIZEOF - 1; i >= 0;  i--)
    {
      if (breaks[i]->is_enabled())
        {
          switch (events[i])
            {
            case TIMER_EVENT_LIMIT_REACHED:
              TRACE_MSG("limit reached" << i);
              if (!breaks[i]->is_active() && operation_mode == OPERATION_MODE_NORMAL)
                {
                  start_break((BreakId)i);
                }
              break;

            case TIMER_EVENT_NATURAL_RESET:
            case TIMER_EVENT_RESET:
              TRACE_MSG("limi reset" << i);
              if (breaks[i]->is_active())
                {
                  breaks[i]->stop_break();
                }
              break;

            case TIMER_EVENT_NONE:
              break;
            }
        }
    }

  if (events[BREAK_ID_DAILY_LIMIT] == TIMER_EVENT_NATURAL_RESET ||
      events[BREAK_ID_DAILY_LIMIT] == TIMER_EVENT_RESET)
    {
      daily_reset();
    }

  TRACE_EXIT();
}


//! starts the specified break.
/*!
 *  \param break_id ID of the timer that caused the break.
 */
void
BreaksControl::start_break(BreakId break_id, BreakId resume_this_break)
{
  // Don't show MB when RB is active, RB when DL is active.
  for (int bi = break_id; bi <= BREAK_ID_DAILY_LIMIT; bi++)
    {
      if (breaks[bi]->is_active())
        {
          return;
        }
    }

  if (break_id == BREAK_ID_REST_BREAK && resume_this_break == BREAK_ID_NONE)
    {
      // Reset override.
      breaks[BREAK_ID_REST_BREAK]->override(BREAK_ID_REST_BREAK);
    }

  // Advance restbreak if it follows within 30s after the end of a microbreak
  if (break_id == BREAK_ID_MICRO_BREAK && breaks[BREAK_ID_REST_BREAK]->is_enabled())
    {
      Timer::Ptr rb_timer = get_timer(BREAK_ID_REST_BREAK);

      bool activity_sensitive = get_timer(BREAK_ID_REST_BREAK)->is_activity_sensitive();

      // Only advance when
      // 0. It is activity sensitive
      // 1. we have a next limit reached time.
      if (activity_sensitive &&
          rb_timer->get_next_limit_time() > 0)
        {
          Timer::Ptr timer = get_timer(break_id);

          gint64 duration = timer->get_auto_reset();
          gint64 now = TimeSource::get_monotonic_time_sec_sync();

          if (now + duration + 30 >= rb_timer->get_next_limit_time())
            {
              breaks[BREAK_ID_REST_BREAK]->override(BREAK_ID_MICRO_BREAK);

              start_break(BREAK_ID_REST_BREAK, BREAK_ID_MICRO_BREAK);

              // Snooze timer before the limit was reached. Just to make sure
              // that it doesn't reach its limit again when elapsed == limit
              rb_timer->snooze_timer();
              return;
            }
        }
    }

  // Stop microbreak when a restbreak starts. should not happend.
  // restbreak should be advanced.
  for (int bi = BREAK_ID_MICRO_BREAK; bi < break_id; bi++)
    {
      if (breaks[bi]->is_active())
        {
          breaks[bi]->stop_break();
        }
    }

  breaks[break_id]->start_break();
}


//! Sets the operation mode.
void
BreaksControl::set_operation_mode(OperationMode mode)
{
  operation_mode = mode;
}


//! Sets the insist policy.
/*!
 *  The insist policy determines what to do when the user is active while
 *  taking a break.
 */
void
BreaksControl::set_insist_policy(ICore::InsistPolicy p)
{
  TRACE_ENTER_MSG("Core::set_insist_policy", p);

  if (active_insist_policy != ICore::INSIST_POLICY_INVALID &&
      insist_policy != p)
    {
      TRACE_MSG("refreeze " << active_insist_policy);
      defrost();
      insist_policy = p;
      freeze();
    }
  else
    {
      insist_policy = p;
    }
  TRACE_EXIT();
}




//! Excecute the insist policy.
void
BreaksControl::freeze()
{
  TRACE_ENTER_MSG("Core::freeze", insist_policy);
  ICore::InsistPolicy policy = insist_policy;

  switch (policy)
    {
    case ICore::INSIST_POLICY_IGNORE:
      {
        // Ignore all activity during break by suspending the activity monitor.
        activity_monitor->suspend();
      }
      break;
    case ICore::INSIST_POLICY_HALT:
      {
        // Halt timer when the user is active.
        set_freeze_all_breaks(true);
      }
      break;
    case ICore::INSIST_POLICY_RESET:
      // reset the timer when the user becomes active.
      // default.
      break;

    default:
      break;
    }

  active_insist_policy = policy;
  TRACE_EXIT();
}


//! Undo the insist policy.
void
BreaksControl::defrost()
{
  TRACE_ENTER_MSG("Core::defrost", active_insist_policy);

  switch (active_insist_policy)
    {
    case ICore::INSIST_POLICY_IGNORE:
      {
        // Resumes the activity monitor, if not suspended.
        if (operation_mode != OPERATION_MODE_SUSPENDED)
          {
            activity_monitor->resume();
          }
      }
      break;
    case ICore::INSIST_POLICY_HALT:
      {
        // Desfrost timers.
        set_freeze_all_breaks(false);
      }
      break;

    default:
      break;
    }

  active_insist_policy = ICore::INSIST_POLICY_INVALID;
  TRACE_EXIT();
}


//! Performs a reset when the daily limit is reached.
void
BreaksControl::daily_reset()
{
  TRACE_ENTER("BreaksControl::daily_reset");
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      breaks[i]->daily_reset();
    }

  save_state();
  TRACE_EXIT();
}


//! Saves the current state.
void
BreaksControl::save_state() const
{
  stringstream ss;
  ss << Util::get_home_directory();
  ss << "state" << ends;

  ofstream stateFile(ss.str().c_str());

  stateFile << "WorkRaveState 3"  << endl
            << TimeSource::get_real_time_sec() << endl;

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      string stateStr = get_timer(i)->serialize_state();

      stateFile << stateStr << endl;
    }

  stateFile.close();
}


//! Loads the current state.
void
BreaksControl::load_state()
{
  stringstream ss;
  ss << Util::get_home_directory();
  ss << "state" << ends;

  ifstream stateFile(ss.str().c_str());

  int version = 0;
  bool ok = stateFile.good();

  if (ok)
    {
      string tag;
      stateFile >> tag;

      ok = (tag == WORKRAVESTATE);
    }

  if (ok)
    {
      stateFile >> version;

      ok = (version >= 1 && version <= 3);
    }

  if (ok)
    {
      gint64 saveTime;
      stateFile >> saveTime;
    }

  while (ok && !stateFile.eof())
    {
      string id;
      stateFile >> id;

      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          if (get_timer(i)->get_id() == id)
            {
              string state;
              getline(stateFile, state);

              get_timer(i)->deserialize_state(state, version);
              break;
            }
        }
    }
}
