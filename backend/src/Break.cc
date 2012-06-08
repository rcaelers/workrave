// Break.cc
//
// Copyright (C) 2001 - 2012 Rob Caelers & Raymond Penners
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
#include "ICoreInternal.hh"
#include "Statistics.hh"
#include "IApp.hh"
#include "IActivityMonitor.hh"
#include "ActivityMonitorListener.hh"
#include "TimerActivityMonitor.hh"
#include "Timer.hh"
#include "Statistics.hh"
#include "CoreConfig.hh"
#include "DayTimePred.hh"

#ifdef HAVE_DBUS
#include "dbus/DBus.hh"
#include "DBusWorkrave.hh"
#endif

using namespace std;
using namespace workrave::dbus;

//! Construct a new Break Controller.
/*!
 *  \param id ID of the break this Break controls.
 *  \param c pointer to control interface.
 *  \param factory pointer to the GUI window factory used to create the break
 *          windows.
 *  \param timer pointer to the interface of the timer that belongs to this break.
 */
Break::Break() :
  break_id(BREAK_ID_NONE),
  core(NULL),
  application(NULL),
  break_timer(NULL),
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
  enabled(true),
  usage_mode(USAGE_MODE_NORMAL)
{
}


//! Destructor.
Break::~Break()
{
  application->hide_break_window();
  delete break_timer;
}


//! Initializes the break.
void
Break::init(BreakId id, ICoreInternal *c, IApp *app)
{
  TRACE_ENTER("Break::init");

  core = c;
  application = app;
  config = core->get_configurator();

  break_name = CoreConfig::get_break_name(break_id);
  break_timer = new Timer(core);
  break_timer->set_id(break_name);
  
  init_timer();
  init_break_control();

  
  TRACE_EXIT();
}

//! Periodic heartbeat.
void
Break::heartbeat()
{
  TRACE_ENTER_MSG("Break::heartbeat", break_id);

  prelude_time++;

  bool is_idle = false;

  if (!break_timer->has_activity_monitor())
    {
      // Prefer the running state of the break timer as input for
      // our current activity.
      TimerState tstate = break_timer->get_state();
      is_idle = (tstate == STATE_STOPPED);
    }
  else
    {
      // Unless the timer has its own activity monitor.
      ActivityState activity_state = core->get_current_monitor_state();
      is_idle = (activity_state != ACTIVITY_ACTIVE);
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


        update_prelude_window();
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
        update_break_window();
        application->refresh_break_window();
      }
      break;
    }

  TRACE_EXIT();
}


//! Initiates the specified break stage.
void
Break::goto_stage(BreakStage stage)
{
  TRACE_ENTER_MSG("Break::goto_stage", break_id << " " << stage);

  send_signal(stage);

  switch (stage)
    {
    case STAGE_DELAYED:
      {
        IActivityMonitor *monitor = core->get_activity_monitor();
        monitor->set_listener(this);

        break_timer->set_insensitive_mode(INSENSITIVE_MODE_IDLE_ON_LIMIT_REACHED);
      }
      break;

    case STAGE_NONE:
      {
        // Teminate the break.
        break_timer->set_insensitive_mode(INSENSITIVE_MODE_IDLE_ON_LIMIT_REACHED);
        application->hide_break_window();
        core->defrost();

        if (break_id == BREAK_ID_MICRO_BREAK &&
            core->get_usage_mode() == USAGE_MODE_READING)
          {
            for (int i = BREAK_ID_MICRO_BREAK; i < BREAK_ID_SIZEOF; i++)
              {
                Timer *break_timer = core->get_timer(BreakId(i));
                break_timer->force_active();
              }
          }

        if (break_stage == STAGE_TAKING && !fake_break)
          {
            // Update statistics and play sound if the break end
            // was "natural"
            time_t idle = break_timer->get_elapsed_idle_time();
            time_t reset = break_timer->get_auto_reset();

            if (idle >= reset && !user_abort)
              {
                // natural break end.

                // Update stats.
                Statistics *stats = core->get_statistics();
                stats->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_TAKEN);

                break_event_signal(BREAK_EVENT_BREAK_ENDED);
              }
          }
      }
      break;

    case STAGE_SNOOZED:
      {
        application->hide_break_window();
        if (!forced_break)
          {
            break_event_signal(BREAK_EVENT_BREAK_IGNORED);
          }
        core->defrost();
      }
      break;

    case STAGE_PRELUDE:
      {
        break_timer->set_insensitive_mode(INSENSITIVE_MODE_FOLLOW_IDLE);
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
        // Break timer should always idle.
        // Previous revisions set MODE_IDLE_ON_LIMIT_REACHED
        break_timer->set_insensitive_mode(INSENSITIVE_MODE_IDLE_ALWAYS);

        // Remove the prelude window, if necessary.
        application->hide_break_window();

        // "Innocent until proven guilty".
        TRACE_MSG("Force idle");
        core->force_break_idle(break_id);
        break_timer->stop_timer();

        // Start the break.
        break_window_start();

        // Play sound
        if (!forced_break)
          {
            break_event_signal(BREAK_EVENT_BREAK_STARTED);
          }

        core->freeze();
      }
      break;
    }

  break_stage = stage;
  TRACE_EXIT();
}


//! Updates the contents of the prelude window.
void
Break::update_prelude_window()
{
  application->set_break_progress(prelude_time, 29);
}


//! Updates the contents of the break window.
void
Break::update_break_window()
{
  assert(break_timer != NULL);
  time_t duration = break_timer->get_auto_reset();
  time_t idle = 0;

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
      TRACE_MSG("Force idle");
      core->force_break_idle(break_id);
      break_timer->stop_timer();

      // Update statistics.
      Statistics *stats = core->get_statistics();
      stats->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_PROMPTED);

      if (prelude_count == 0)
        {
          stats->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_UNIQUE_BREAKS);
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
      time_t idle = break_timer->get_elapsed_idle_time();
      TRACE_MSG(idle << " " << break_timer->get_auto_reset() << " " << break_timer->is_enabled());
      if (idle >= break_timer->get_auto_reset() || !break_timer->is_enabled())
        {
          TRACE_MSG("Faking break");
          fake_break = true;
          fake_break_count = break_timer->get_auto_reset();
        }
    }

  if (!break_timer->get_activity_sensitive())
    {
      TRACE_MSG("Forcing idle");
      core->force_break_idle(break_id);
    }

  goto_stage(STAGE_TAKING);

  TRACE_EXIT();
}


//! Stops the break.
/*!
 *  Stopping a break will reset the "number of presented preludes" counter. So,
 *  wrt, "max-preludes", the break will start over when it comes back.
 */
void
Break::stop_break()
{
  TRACE_ENTER("Break::stop_break");

  suspend_break();
  prelude_count = 0;

  TRACE_EXIT();
}


//! Suspend the break.
/*!
 *  A suspended break will come back after snooze time. The number of times the
 *  break will come back can be defined with set_max_preludes.
 */
void
Break::suspend_break()
{
  TRACE_ENTER_MSG("Break::suspend_break", break_id);

  break_hint = BREAK_HINT_NONE;

  goto_stage(STAGE_NONE);

  TRACE_EXIT();
}


boost::signals2::signal<void(IBreak::BreakEvent)> &
Break::signal_break_event()
{
  return break_event_signal;
}

//! Returns the timer.
Timer *
Break::get_timer() const
{
  return break_timer;
}

//! Returns the name of the break (used in configuration)
string
Break::get_name() const
{
  return break_name;
}


//! Is the break active ?
bool
Break::is_active() const
{
  return break_stage != STAGE_NONE && break_stage != STAGE_SNOOZED;
}

bool
Break::is_taking() const
{
  return break_stage == STAGE_TAKING;
}


//! Postpones the active break.
/*!
 *  Postponing a break does not reset the break timer. The break prelude window
 *  will re-appear after snooze time.
 */
void
Break::postpone_break()
{
  if (break_stage == STAGE_TAKING)
    {
      if (!forced_break)
        {
          if (!fake_break)
            {
              // Snooze the timer.
              break_timer->snooze_timer();
            }

          // Update stats.
          Statistics *stats = core->get_statistics();
          stats->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_POSTPONED);
        }

      // This is to avoid a skip sound...
      user_abort = true;

      // and stop the break.
      stop_break();
    }
}


//! Skips the active break.
/*!
 *  Skipping a break resets the break timer.
 */
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
      Statistics *stats = core->get_statistics();
      stats->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_SKIPPED);

      // and stop the break.
      stop_break();
    }
}

void
Break::stop_prelude()
{
  delayed_abort = true;
}


//! Sets the maximum number of preludes.
/*!
 *  After the maximum number of preludes, the break either stops bothering the
 *  user, or forces a break. This can be set with set_force_after_preludes.
 */
void
Break::set_max_preludes(int m)
{
  max_number_of_preludes = m;
}

//! Creates and shows the break window.
void
Break::break_window_start()
{
  TRACE_ENTER_MSG("Break::break_window_start", break_id);

  application->create_break_window(break_id, break_hint);
  update_break_window();
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

  update_prelude_window();

  application->show_break_window();

  TRACE_EXIT();
}


bool
Break::action_notify()
{
  TRACE_ENTER("GUI::action_notify");
  stop_prelude();
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

  if (progress != NULL)
    {
      DBus *dbus = core->get_dbus();
      org_workrave_CoreInterface *iface = org_workrave_CoreInterface::instance(dbus);

      if (iface != NULL)
        {
          switch (break_id)
            {
            case BREAK_ID_MICRO_BREAK:
              iface->MicrobreakChanged("/org/workrave/Workrave/Core", progress);
              break;

            case BREAK_ID_REST_BREAK:
              iface->RestbreakChanged("/org/workrave/Workrave/Core", progress);
              break;

            case BREAK_ID_DAILY_LIMIT:
              iface->DailylimitChanged("/org/workrave/Workrave/Core", progress);
              break;

            default:
              break;
            }
        }
    }

#endif
}

void
Break::override(BreakId id)
{
  int max_preludes;
  config->get_value(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % break_id, max_preludes);

  if (break_id != id)
    {
      int max_preludes_other;
      config->get_value(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % id, max_preludes_other);
      if (max_preludes_other != -1 && max_preludes_other < max_preludes)
        {
          max_preludes = max_preludes_other;
        }
    }

  set_max_preludes(max_preludes);
}


bool
Break::get_timer_activity_sensitive() const
{
  return break_timer->get_activity_sensitive();
}


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


time_t
Break::get_elapsed_time() const
{
  return break_timer->get_elapsed_time();
}


time_t
Break::get_elapsed_idle_time() const
{
  return break_timer->get_elapsed_idle_time();
}


time_t
Break::get_auto_reset() const
{
  return break_timer->get_auto_reset();
}


bool
Break::is_auto_reset_enabled() const
{
  return break_timer->is_auto_reset_enabled();
}


time_t
Break::get_limit() const
{
  return break_timer->get_limit();
}


bool
Break::is_limit_enabled() const
{
  return break_timer->is_limit_enabled();
}


time_t
Break::get_total_overdue_time() const
{
  return break_timer->get_total_overdue_time();
}


void
Break::set_usage_mode(UsageMode mode)
{
  TRACE_ENTER_MSG("Break::set_usage_mode", mode);
  if (usage_mode != mode)
    {
      usage_mode = mode;

      TRACE_MSG("changing");
      if (mode == USAGE_MODE_NORMAL)
        {
          break_timer->set_activity_sensitive(true);
        }
      else if (mode == USAGE_MODE_READING)
        {
          break_timer->set_activity_sensitive(false);
        }
    }
  TRACE_EXIT();
}


// Initialize the timer based.
void
Break::init_timer()
{
  load_timer_config();

  break_timer->enable();
  break_timer->stop_timer();

  config->add_listener(CoreConfig::CFG_KEY_TIMER % break_id, this);
}


//! Load the configuration of the timer.
void
Break::load_timer_config()
{
  TRACE_ENTER("Break::load_timer_config");
  // Read break limit.
  int limit;
  config->get_value(CoreConfig::CFG_KEY_TIMER_LIMIT % break_id, limit);
  break_timer->set_limit(limit);
  break_timer->set_limit_enabled(limit > 0);

  // Read autoreset interval
  int autoreset;
  config->get_value(CoreConfig::CFG_KEY_TIMER_AUTO_RESET % break_id, autoreset);
  break_timer->set_auto_reset(autoreset);
  break_timer->set_auto_reset_enabled(autoreset > 0);

  // Read reset predicate
  string reset_pred;
  config->get_value(CoreConfig::CFG_KEY_TIMER_RESET_PRED % break_id, reset_pred);
  if (reset_pred != "")
    {
      TimePred *pred = create_time_pred(reset_pred);
      if (pred != NULL)
        {
          break_timer->set_auto_reset(pred);
        }
    }

  // Read the snooze time.
  int snooze;
  config->get_value(CoreConfig::CFG_KEY_TIMER_SNOOZE % break_id, snooze);
  break_timer->set_snooze_interval(snooze);

  // Load the monitor setting for the timer.
  string monitor_name;

  bool ret = config->get_value(CoreConfig::CFG_KEY_TIMER_MONITOR % break_id, monitor_name);

  TRACE_MSG(ret << " " << monitor_name);
  if (ret && monitor_name != "")
    {
      Timer *master = core->get_timer(monitor_name);
      if (master != NULL)
        {
          TRACE_MSG("found master timer");
          IActivityMonitor *monitor = core->get_activity_monitor();
          TimerActivityMonitor *am = new TimerActivityMonitor(monitor, master);
          break_timer->set_activity_monitor(am);
        }
    }
  else
    {
      break_timer->set_activity_monitor(NULL);
    }
  TRACE_EXIT();
}


TimePred *
Break::create_time_pred(string spec)
{
  TimePred *pred = 0;
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

// Initialize the break control.
void
Break::init_break_control()
{
  load_break_control_config();
  config->add_listener(CoreConfig::CFG_KEY_BREAK % break_id, this);
}


//! Load the configuration of the timer.
void
Break::load_break_control_config()
{
  // Maximum number of prelude windows.
  int max_preludes;
  config->get_value(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % break_id, max_preludes);
  set_max_preludes(max_preludes);

  // Break enabled?
  enabled = true;
  config->get_value(CoreConfig::CFG_KEY_BREAK_ENABLED % break_id, enabled);
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


