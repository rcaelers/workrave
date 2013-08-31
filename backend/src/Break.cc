// Break.cc
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

#include <assert.h>
#include <string>

#include "Break.hh"

#include "IBreak.hh"
#include "IBreakSupport.hh"
#include "Statistics.hh"
#include "IApp.hh"
#include "IActivityMonitor.hh"
#include "IActivityMonitorListener.hh"
#include "TimerActivityMonitor.hh"
#include "Timer.hh"
#include "DayTimePred.hh"
#include "Statistics.hh"
#include "CoreConfig.hh"

#include "dbus/DBus.hh"
#include "dbus/DBusException.hh"

#ifdef HAVE_DBUS
#include "DBusWorkrave.hh"
#endif

using namespace workrave::dbus;
using namespace std;


Break::Ptr
Break::create(BreakId id,
              IApp *app,
              IBreakSupport::Ptr break_support, 
              IActivityMonitor::Ptr activity_monitor,
              Statistics::Ptr statistics,
              IConfigurator::Ptr configurator,
              DBus::Ptr dbus)
{
  return Ptr(new Break(id, app, break_support, activity_monitor, statistics, configurator, dbus));
}


//! Construct a new Break Controller.
Break::Break(BreakId id,
             IApp *app,
             IBreakSupport::Ptr break_support, 
             IActivityMonitor::Ptr activity_monitor,
             Statistics::Ptr statistics,
             IConfigurator::Ptr configurator,
             DBus::Ptr dbus) :
  break_id(id),
  application(app),
  break_support(break_support),
  activity_monitor(activity_monitor),
  statistics(statistics),
  configurator(configurator),
  dbus(dbus),
  break_stage(STAGE_NONE),
  reached_max_prelude(false),
  prelude_time(0),
  prelude_count(0),
  max_number_of_preludes(2),
  fake_break(false),
  fake_break_count(0),
  user_abort(false),
  delayed_abort(false),
  break_hint(BREAK_HINT_NONE),
  enabled(true)
{
}


//! Destructor.
Break::~Break()
{
  application->hide_break_window();
}


/********************************************************************************/
/**** Internal Interface                                                   ******/
/********************************************************************************/

//! Initializes the break.
void
Break::init()
{
  TRACE_ENTER("Break::init");

  break_name = CoreConfig::get_break_name(break_id);

  break_timer = Timer::create();
  break_timer->set_id(break_name);
  
  load_timer_config();

  break_timer->enable();
  break_timer->stop_timer();

  load_break_control_config();
  
  configurator->add_listener(CoreConfig::CFG_KEY_TIMER % break_id, this);
  configurator->add_listener(CoreConfig::CFG_KEY_BREAK % break_id, this);

#ifdef HAVE_DBUS
  if (dbus)
    {
      try
        {
          string path = string("/org/workrave/Workrave/Break/" + get_name());
          dbus->connect(path, "org.workrave.BreakInterface", this);
          dbus->register_object_path(path);
        }
      catch (dbus::DBusException &)
        {
        }
    }
#endif

  TRACE_EXIT();
}


TimerEvent
Break::process_timer(ActivityState state)
{
  TRACE_ENTER_MSG("Break::process_timer", break_id);

  if (timer_activity_monitor)
    {
      // Prefer the running state of the break timer as input for
      // our current activity.
      state = timer_activity_monitor->get_state();
    }
  
  TimerEvent event = break_timer->process(state);

  if (break_id == BREAK_ID_DAILY_LIMIT &&
      (event == TIMER_EVENT_NATURAL_RESET || event == TIMER_EVENT_RESET))
    {
      statistics->set_counter(Statistics::STATS_VALUE_TOTAL_ACTIVE_TIME, (int) break_timer->get_elapsed_time());
      statistics->start_new_day();
    }

  if (event == TIMER_EVENT_NATURAL_RESET)
    {
      statistics->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_NATURAL_TAKEN);
    }

  TRACE_RETURN(event);
  return event;
}


//! Periodic heartbeat.
void
Break::process_break()
{
  TRACE_ENTER_MSG("Break::process_break", break_id);

  prelude_time++;

  bool is_idle = false;

  if (!timer_activity_monitor)
    {
      // Prefer the running state of the break timer as input for
      // our current activity.
      TimerState tstate = break_timer->get_state();
      is_idle = (tstate == STATE_STOPPED);
    }
  else
    {
      // Unless the timer has its own activity monitor.
      is_idle = activity_monitor->get_state() != ACTIVITY_ACTIVE;
    }

  TRACE_MSG("stage = " << break_stage);
  switch (break_stage)
    {
    case STAGE_NONE:
      break;

    case STAGE_SNOOZED:
      break;

    case STAGE_DELAYED:
      {
        if (delayed_abort)
          {
            // User become active during delayed break.
            goto_stage(STAGE_SNOOZED);
          }
        else if (is_idle)
          {
            // User is idle.
            goto_stage(STAGE_TAKING);
          }
      }
      break;

    case STAGE_PRELUDE:
      {
        assert(application != NULL);

        TRACE_MSG("prelude time = " << prelude_time);

        prelude_window_update();
        application->refresh_break_window();

        if (is_idle)
          {
            // User is idle.
            if (prelude_time >= 10)
              {
                // User is idle and prelude is visible for at least 10s.
                goto_stage(STAGE_TAKING);
              }
          }
        else if (prelude_time == 30)
          {
            // User is not idle and the prelude is visible for 30s.
            if (reached_max_prelude)
              {
                // Final prelude, force break.
                goto_stage(STAGE_TAKING);
              }
            else
              {
                // Delay break.
                goto_stage(STAGE_DELAYED);
              }
          }
        else if (prelude_time == 20)
          {
            // Still not idle after 20s. Red alert.
            application->set_prelude_stage(IApp::STAGE_ALERT);
            application->refresh_break_window();
          }
        else if (prelude_time == 10)
          {
            // Still not idle after 10s. Yellow alert.
            application->set_prelude_stage(IApp::STAGE_WARN);
            application->refresh_break_window();
          }

        if (prelude_time == 4)
          {
            // Move prelude window to top of screen after 4s.
            application->set_prelude_stage(IApp::STAGE_MOVE_OUT);
          }
      }
      break;

    case STAGE_TAKING:
      {
        // refresh the break window.
        break_window_update();
        application->refresh_break_window();
      }
      break;
    }

  update_statistics();
  
  TRACE_EXIT();
}


//! Starts the break.
void
Break::start_break()
{
  TRACE_ENTER_MSG("Break::start_break", break_id);

  break_hint = BREAK_HINT_NONE;
  forced_break = false;
  fake_break = false;
  prelude_time = 0;
  user_abort = false;
  delayed_abort = false;

  reached_max_prelude = max_number_of_preludes >= 0 && prelude_count + 1 >= max_number_of_preludes;

  if (max_number_of_preludes >= 0 && prelude_count >= max_number_of_preludes)
    {
      // Forcing break without prelude.
      goto_stage(STAGE_TAKING);
    }
  else
    {
      // Starting break with prelude.

      // Idle until proven guilty.
      force_idle();

      // Update statistics.
      statistics->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_PROMPTED);

      if (prelude_count == 0)
        {
          statistics->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_UNIQUE_BREAKS);
        }

      // Start prelude.
      goto_stage(STAGE_PRELUDE);
    }

  TRACE_EXIT();
}


//! Starts the break without preludes.
void
Break::force_start_break(BreakHint hint)
{
  TRACE_ENTER_MSG("Break::force_start_break", break_id);

  break_hint = hint;
  forced_break = (break_hint & (BREAK_HINT_USER_INITIATED | BREAK_HINT_NATURAL_BREAK) ) != 0;
  fake_break = false;
  prelude_time = 0;
  user_abort = false;
  delayed_abort = false;

  if (break_timer->is_auto_reset_enabled())
    {
      TRACE_MSG("auto reset enabled");
      int64_t idle = break_timer->get_elapsed_idle_time();
      TRACE_MSG(idle << " " << break_timer->get_auto_reset() << " " << break_timer->is_enabled());
      if (idle >= break_timer->get_auto_reset() || !break_timer->is_enabled())
        {
          TRACE_MSG("Faking break");
          fake_break = true;
          fake_break_count = break_timer->get_auto_reset();
        }
    }

  goto_stage(STAGE_TAKING);

  TRACE_EXIT();
}


//! Stops the break.
void
Break::stop_break()
{
  TRACE_ENTER("Break::stop_break");

  break_hint = BREAK_HINT_NONE;
  goto_stage(STAGE_NONE);
  prelude_count = 0;

  TRACE_EXIT();
}


//! Freezes the break.
void
Break::freeze_break(bool freeze)
{
  TRACE_ENTER("Break::freeze_break");

  if (! timer_activity_monitor)
    {
      break_timer->freeze_timer(freeze);
    }

  TRACE_EXIT();
}


//! Force the timer to become idle.
void
Break::force_idle()
{
  TRACE_ENTER("Break::Force_idle");
  activity_monitor->force_idle();
  if (timer_activity_monitor)
    {
      timer_activity_monitor->force_idle();
    }
  break_timer->stop_timer();
  TRACE_EXIT();
}

void
Break::daily_reset()
{
  update_statistics();
  break_timer->daily_reset_timer();
}


//! Returns the timer.
Timer::Ptr
Break::get_timer() const
{
  return break_timer;
}


void
Break::override(BreakId id)
{
  int max_preludes;
  configurator->get_value(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % break_id, max_preludes);

  if (break_id != id)
    {
      int max_preludes_other;
      configurator->get_value(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % id, max_preludes_other);
      if (max_preludes_other != -1 && max_preludes_other < max_preludes)
        {
          max_preludes = max_preludes_other;
        }
    }

  max_number_of_preludes = max_preludes;
}


/********************************************************************************/
/**** IBreak                                                               ******/
/********************************************************************************/

boost::signals2::signal<void(IBreak::BreakEvent)> &
Break::signal_break_event()
{
  return break_event_signal;
}


//! Returns the name of the break.
string
Break::get_name() const
{
  return break_name;
}


//! Is this break currently enabled?
bool
Break::is_enabled() const
{
  return enabled;
}


bool
Break::is_running() const
{
  TimerState state = break_timer->get_state();
  return state == STATE_RUNNING;
}


bool
Break::is_taking() const
{
  return break_stage == STAGE_TAKING;
}


//! Is the break active ?
bool
Break::is_active() const
{
  return break_stage != STAGE_NONE && break_stage != STAGE_SNOOZED;
}


int64_t
Break::get_elapsed_time() const
{
  return break_timer->get_elapsed_time();
}


int64_t
Break::get_elapsed_idle_time() const
{
  return break_timer->get_elapsed_idle_time();
}


int64_t
Break::get_auto_reset() const
{
  return break_timer->get_auto_reset();
}


bool
Break::is_auto_reset_enabled() const
{
  return break_timer->is_auto_reset_enabled();
}


int64_t
Break::get_limit() const
{
  return break_timer->get_limit();
}


bool
Break::is_limit_enabled() const
{
  return break_timer->is_limit_enabled();
}


int64_t
Break::get_total_overdue_time() const
{
  return break_timer->get_total_overdue_time();
}


//! Postpones the active break.
/*!
 *  Postponing a break does not reset the break timer. The break prelude window
 *  will re-appear after snooze time.
 */
void
Break::postpone_break()
{
  TRACE_ENTER_MSG("Break::postpone_break", break_id);
  if (break_stage == STAGE_TAKING)
    {
      if (!forced_break)
        {
          if (!fake_break)
            {
              // Snooze the timer.
              TRACE_MSG("Snoozing timer");
              break_timer->snooze_timer();
            }

          // Update stats.
          statistics->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_POSTPONED);
        }

      // This is to avoid a skip sound...
      user_abort = true;

      // and stop the break.
      stop_break();
    }
}


//! Skips the active break.
void
Break::skip_break()
{
  // This is to avoid a skip sound...
  if (break_stage == STAGE_TAKING)
    {
      user_abort = true;

      if (break_id == BREAK_ID_DAILY_LIMIT)
        {
          // Make sure the daily limit remains silent after skipping.
          break_timer->inhibit_snooze();
        }
      else
        {
          // Reset the restbreak timer.
          break_timer->reset_timer();
        }

      // Update stats.
      statistics->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_SKIPPED);

      // and stop the break.
      stop_break();
    }
}


/********************************************************************************/
/**** Internal                                                             ******/
/********************************************************************************/

//! Initiates the specified break stage.
void
Break::goto_stage(BreakStage stage)
{
  TRACE_ENTER_MSG("Break::goto_stage", break_id << " " << stage);

  switch (stage)
    {
    case STAGE_DELAYED:
      {
        activity_monitor->set_listener(shared_from_this());
      }
      break;

    case STAGE_NONE:
      {
        // Teminate the break.
        application->hide_break_window();

        if (break_stage == STAGE_TAKING && !fake_break)
          {
            // Update statistics and play sound if the break end
            // was "natural"
            int64_t idle = break_timer->get_elapsed_idle_time();
            int64_t reset = break_timer->get_auto_reset();

            if (idle >= reset && !user_abort)
              {
                // Breaks end without user skip/postponing it.
                statistics->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_TAKEN);
                break_event_signal(BREAK_EVENT_BREAK_ENDED);
              }
          }
        break_event_signal(BREAK_EVENT_BREAK_IDLE);
      }
      break;

    case STAGE_SNOOZED:
      {
        application->hide_break_window();
        if (!forced_break)
          {
            break_event_signal(BREAK_EVENT_BREAK_IGNORED);
          }
        break_event_signal(BREAK_EVENT_BREAK_IDLE);
      }
      break;

    case STAGE_PRELUDE:
      {
        prelude_count++;
        prelude_time = 0;
        application->hide_break_window();

        prelude_window_start();
        application->refresh_break_window();
        break_event_signal(BREAK_EVENT_PRELUDE_STARTED);
      }
      break;

    case STAGE_TAKING:
      {
        // Remove the prelude window, if necessary.
        application->hide_break_window();

        // "Innocent until proven guilty".
        force_idle();

        // Start the break.
        break_window_start();

        // Report state change.
        break_event_signal(forced_break
                           ? BREAK_EVENT_BREAK_STARTED
                           : BREAK_EVENT_BREAK_STARTED_FORCED);
      }
      break;
    }

  break_stage = stage;
  TRACE_EXIT();
}


//! Creates and shows the break window.
void
Break::break_window_start()
{
  TRACE_ENTER_MSG("Break::break_window_start", break_id);

  application->create_break_window(break_id, break_hint);
  break_window_update();
  application->show_break_window();

  TRACE_EXIT();
}

//! Creates and shows the prelude window.
void
Break::prelude_window_start()
{
  TRACE_ENTER_MSG("Break::prelude_window_start", break_id);

  application->create_prelude_window(break_id);

  application->set_prelude_stage(IApp::STAGE_INITIAL);

  if (!reached_max_prelude)
    {
      application->set_prelude_progress_text(IApp::PROGRESS_TEXT_DISAPPEARS_IN);
    }
  else
    {
      application->set_prelude_progress_text(IApp::PROGRESS_TEXT_BREAK_IN);
    }

  prelude_window_update();

  application->show_break_window();

  TRACE_EXIT();
}


//! Updates the contents of the break window.
void
Break::break_window_update()
{
  int64_t duration = break_timer->get_auto_reset();
  int64_t idle = 0;

  if (fake_break)
    {
      idle = duration - fake_break_count;
      if (fake_break_count <= 0)
        {
          stop_break();
        }

      fake_break_count--;
    }
  else
    {
      idle = break_timer->get_elapsed_idle_time();
    }

  if (idle > duration)
    {
      idle = duration;
    }

  application->set_break_progress((int)idle, (int)duration);
}


//! Updates the contents of the prelude window.
void
Break::prelude_window_update()
{
  application->set_break_progress(prelude_time, 29);
}


//!
bool
Break::action_notify()
{
  TRACE_ENTER("GUI::action_notify");
  delayed_abort = true;
  TRACE_EXIT();
  return false;   // false: kill listener.
}


//! Send DBus signal when break stage changes.
void
Break::send_signal(BreakStage stage)
{
  (void) stage;
#ifdef HAVE_DBUS
  const char *progress = NULL;
  (void) progress;
  
  switch (stage)
    {
    case STAGE_NONE:
      progress = "none";
      break;

    case STAGE_SNOOZED:
      progress = "none";
      break;

    case STAGE_DELAYED:
      // Do not send this stage.
      break;

    case STAGE_PRELUDE:
      progress = "prelude";
      break;

    case STAGE_TAKING:
      progress = "break";
      break;
    }

  if (progress != NULL && dbus)
    {
      org_workrave_BreakInterface *iface = org_workrave_BreakInterface::instance(dbus.get());

      if (iface != NULL)
        {
          iface->Changed("/org/workrave/Workrave/Break/" + get_name(), progress);
        }
    }
#endif
}


void
Break::update_statistics()
{
  if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      statistics->set_counter(Statistics::STATS_VALUE_TOTAL_ACTIVE_TIME, (int)get_elapsed_time());
    }
  
  statistics->set_break_counter(break_id, Statistics::STATS_BREAKVALUE_TOTAL_OVERDUE, (int)get_total_overdue_time());
}


DayTimePred *
Break::create_time_pred(string spec)
{
  DayTimePred *pred = 0;
  bool ok = false;

  std::string type;
  std::string::size_type pos = spec.find('/');

  if (pos != std::string::npos)
    {
      type = spec.substr(0, pos);
      spec = spec.substr(pos + 1);

      if (type == "day")
        {
          DayTimePred *dayPred = new DayTimePred();
          ok = dayPred->init(spec);
          pred = dayPred;
        }
    }

  if (pred && !ok)
    {
      delete pred;
      pred = NULL;
    }

  return pred;
}


//! Load the configuration of the timer.
void
Break::load_timer_config()
{
  TRACE_ENTER("Break::load_timer_config");
  // Read break limit.
  int limit;
  configurator->get_value(CoreConfig::CFG_KEY_TIMER_LIMIT % break_id, limit);
  break_timer->set_limit(limit);
  break_timer->set_limit_enabled(limit > 0);

  // Read autoreset interval
  int autoreset;
  configurator->get_value(CoreConfig::CFG_KEY_TIMER_AUTO_RESET % break_id, autoreset);
  break_timer->set_auto_reset(autoreset);
  break_timer->set_auto_reset_enabled(autoreset > 0);

  // Read reset predicate
  string reset_pred;
  configurator->get_value(CoreConfig::CFG_KEY_TIMER_RESET_PRED % break_id, reset_pred);
  if (reset_pred != "")
    {
      DayTimePred *pred = create_time_pred(reset_pred);
      break_timer->set_daily_reset(pred);
    }

  // Read the snooze time.
  int snooze;
  configurator->get_value(CoreConfig::CFG_KEY_TIMER_SNOOZE % break_id, snooze);
  break_timer->set_snooze(snooze);

  // Read the monitor setting for the timer.
  timer_activity_monitor.reset();
  if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      string monitor_name;
      bool ret = configurator->get_value(CoreConfig::CFG_KEY_TIMER_MONITOR % break_id, monitor_name);

      if (ret && monitor_name != "" && monitor_name != get_name())
        {
          timer_activity_monitor = break_support->create_timer_activity_monitor(monitor_name);
        }
    }
  
  TRACE_EXIT();
}


//! Load the configuration of the timer.
void
Break::load_break_control_config()
{
  // Maximum number of prelude windows.
  int max_preludes;
  configurator->get_value(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % break_id, max_preludes);
  max_number_of_preludes = max_preludes;

  // Break enabled?
  enabled = true;
  configurator->get_value(CoreConfig::CFG_KEY_BREAK_ENABLED % break_id, enabled);

  if (enabled)
    {
      break_timer->enable();
      if (break_id == BREAK_ID_DAILY_LIMIT)
        {
          break_timer->set_limit_enabled(break_timer->get_limit() > 0);
        }
    }
  else
    {
      if (break_id != BREAK_ID_DAILY_LIMIT)
        {
          break_timer->disable();
        }
      else
        {
          break_timer->set_limit_enabled(false);
        }
    }
}


//! Notification that the configuration changed.
void
Break::config_changed_notify(const string &key)
{
  TRACE_ENTER_MSG("Break::config_changed_notify", key);
  string name;

  if (CoreConfig::starts_with(key, CoreConfig::CFG_KEY_BREAKS, name))
    {
      TRACE_MSG("break: " << name);
      load_break_control_config();
    }
  else if (CoreConfig::starts_with(key, CoreConfig::CFG_KEY_TIMERS, name))
    {
      TRACE_MSG("timer: " << name);
      load_timer_config();
    }
  TRACE_EXIT();
}
