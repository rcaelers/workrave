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
#  include "config.h"
#endif

#include "debug.hh"

#include <cassert>
#include <string>

#include "BreakControl.hh"

#include "core/IBreak.hh"
#include "Core.hh"
#include "Statistics.hh"
#include "core/IApp.hh"
#include "IActivityMonitor.hh"
#include "IActivityMonitorListener.hh"
#include "Timer.hh"
#include "Statistics.hh"

#if defined(HAVE_DBUS)
#  include "dbus/IDBus.hh"
#  include "DBusWorkrave.hh"
#endif

using namespace std;

//! Construct a new Break Controller.
/*!
 *  \param id ID of the break this BreakControl controls.
 *  \param c pointer to control interface.
 *  \param factory pointer to the GUI window factory used to create the break
 *          windows.
 *  \param timer pointer to the interface of the timer that belongs to this break.
 */
BreakControl::BreakControl(BreakId id, const std::string &break_name, IApp *app, Timer *timer)
  : break_id(id)
  , break_name(break_name)
  , application(app)
  , break_timer(timer)
  , break_stage{break_name + ".break.state", BreakStage::None}
  , prelude_count{break_name + ".break.prelude_count", 0}
  , forced_break{break_name + ".break.forced", false}
  , fake_break{break_name + ".break.fake", false}
  , user_abort{break_name + ".break.user_abort", false}
  , delayed_abort{break_name + ".break.delayed_abort", false}
{
  assert(break_timer != nullptr);
  assert(application != nullptr);

  core = Core::get_instance();
}

//! Destructor.
BreakControl::~BreakControl()
{
  // TODO: application->hide_break_window();
}

//! Periodic heartbeat.
void
BreakControl::heartbeat()
{
  TRACE_ENTRY_PAR(break_id);

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

  TRACE_MSG("stage = {}", break_stage);
  switch (break_stage)
    {
    case BreakStage::None:
      break;

    case BreakStage::Snoozed:
      break;

    case BreakStage::Delayed:
      {
        if (delayed_abort)
          {
            // User become active during delayed break.
            goto_stage(BreakStage::Snoozed);
          }
        else if (is_idle)
          {
            // User is idle.
            goto_stage(BreakStage::Taking);
          }
      }
      break;

    case BreakStage::Prelude:
      {
        assert(application != nullptr);

        TRACE_MSG("prelude time = {}", prelude_time);

        update_prelude_window();
        application->refresh_break_window();

        if (is_idle)
          {
            // User is idle.
            if (prelude_time >= 10)
              {
                // User is idle and prelude is visible for at least 10s.
                goto_stage(BreakStage::Taking);
              }
          }
        else if (prelude_time == 30)
          {
            // User is not idle and the prelude is visible for 30s.
            if (reached_max_prelude)
              {
                // Final prelude, force break.
                goto_stage(BreakStage::Taking);
              }
            else
              {
                // Delay break.
                goto_stage(BreakStage::Delayed);
              }
          }
        else if (prelude_time == 20)
          {
            // Still not idle after 20s. Red alert.
            application->set_prelude_stage(IApp::PreludeStage::Alert);
            application->refresh_break_window();
          }
        else if (prelude_time == 10)
          {
            // Still not idle after 10s. Yellow alert.
            application->set_prelude_stage(IApp::PreludeStage::Warn);
            application->refresh_break_window();
          }

        if (prelude_time == 4)
          {
            // Move prelude window to top of screen after 4s.
            application->set_prelude_stage(IApp::PreludeStage::MoveOut);
          }
      }
      break;

    case BreakStage::Taking:
      {
        // refresh the break window.
        update_break_window();
        application->refresh_break_window();
      }
      break;
    }
}

//! Initiates the specified break stage.
void
BreakControl::goto_stage(BreakStage stage)
{
  TRACE_ENTRY_PAR(break_id, stage);

  send_signal(stage);

  switch (stage)
    {
    case BreakStage::Delayed:
      {
        IActivityMonitor::Ptr monitor = core->get_activity_monitor();
        monitor->set_listener(this);

        core->set_insensitive_mode_all_breaks(INSENSITIVE_MODE_IDLE_ON_LIMIT_REACHED);
      }
      break;

    case BreakStage::None:
      {
        // Terminate the break.
        core->set_insensitive_mode_all_breaks(INSENSITIVE_MODE_IDLE_ON_LIMIT_REACHED);
        application->hide_break_window();
        core->defrost();

        if (break_id == BREAK_ID_MICRO_BREAK && core->get_usage_mode() == UsageMode::Reading)
          {
            for (int i = BREAK_ID_MICRO_BREAK; i < BREAK_ID_SIZEOF; i++)
              {
                Timer *timer = core->get_timer(BreakId(i));
                timer->force_active();
              }
          }

        if (break_stage == BreakStage::Prelude)
          {
            if (!forced_break)
              {
                break_event_signal(BreakEvent::BreakIgnored);
              }
          }

        if (break_stage == BreakStage::Taking && !fake_break)
          {
            // Update statistics and play sound if the break end
            // was "natural"
            int64_t idle = break_timer->get_elapsed_idle_time();
            int64_t reset = break_timer->get_auto_reset();

            if (idle >= reset && !user_abort)
              {
                // natural break end.

                // Update stats.
                Statistics *stats = core->get_statistics();
                stats->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_TAKEN);

                // Play sound
                switch (break_id)
                  {
                  case BREAK_ID_REST_BREAK:
                    post_event(CORE_EVENT_SOUND_REST_BREAK_ENDED);
                    break;
                  case BREAK_ID_MICRO_BREAK:
                    post_event(CORE_EVENT_SOUND_MICRO_BREAK_ENDED);
                    break;
                  default:
                    break;
                  }

                // Breaks end without user skip/postponing it.
                break_event_signal(BreakEvent::BreakTaken);
              }
          }
        break_event_signal(BreakEvent::BreakIdle);
      }
      break;

    case BreakStage::Snoozed:
      {
        application->hide_break_window();
        if (!forced_break)
          {
            post_event(CORE_EVENT_SOUND_BREAK_IGNORED);
            break_event_signal(BreakEvent::BreakIgnored);
          }
        break_event_signal(BreakEvent::BreakIdle);
        core->defrost();
      }
      break;

    case BreakStage::Prelude:
      {
        core->set_insensitive_mode_all_breaks(INSENSITIVE_MODE_FOLLOW_IDLE);
        prelude_count++;
        prelude_time = 0;
        application->hide_break_window();

        prelude_window_start();
        application->refresh_break_window();
        post_event(CORE_EVENT_SOUND_BREAK_PRELUDE);
      }
      break;

    case BreakStage::Taking:
      {
        // Break timer should always idle.
        // Previous revisions set MODE_IDLE_ON_LIMIT_REACHED
        // core->set_insensitive_mode_all_breaks(INSENSITIVE_MODE_IDLE_ALWAYS);
        core->set_insensitive_mode_all_breaks(INSENSITIVE_MODE_FOLLOW_IDLE);

        // Remove the prelude window, if necessary.
        application->hide_break_window();

        // "Innocent until proven guilty".
        TRACE_MSG("Force idle");
        core->force_idle(break_id);
        break_timer->stop_timer();

        // Start the break.
        break_window_start();

        // Play sound
        if (!forced_break)
          {
            CoreEvent event = CORE_EVENT_NONE;
            switch (break_id)
              {
              case BREAK_ID_REST_BREAK:
                event = CORE_EVENT_SOUND_REST_BREAK_STARTED;
                break;
              case BREAK_ID_MICRO_BREAK:
                event = CORE_EVENT_SOUND_MICRO_BREAK_STARTED;
                break;
              case BREAK_ID_DAILY_LIMIT:
                event = CORE_EVENT_SOUND_DAILY_LIMIT;
                break;
              default:
                break;
              }
            post_event(event);
          }

        core->freeze();
      }
      break;
    }

  break_stage = stage;
  break_stage_changed_signal(stage);
}

//! Updates the contents of the prelude window.
void
BreakControl::update_prelude_window()
{
  application->set_break_progress(prelude_time, 29);
}

//! Updates the contents of the break window.
void
BreakControl::update_break_window()
{
  assert(break_timer != nullptr);
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

//! Starts the break.
void
BreakControl::start_break()
{
  TRACE_ENTRY_PAR(break_id);

  break_hint = BreakHint::Normal;
  forced_break = false;
  fake_break = false;
  prelude_time = 0;
  user_abort = false;
  delayed_abort = false;

  reached_max_prelude = max_number_of_preludes >= 0 && prelude_count + 1 >= max_number_of_preludes;

  if (max_number_of_preludes >= 0 && prelude_count >= max_number_of_preludes)
    {
      // Forcing break without prelude.
      goto_stage(BreakStage::Taking);
    }
  else
    {
      // Starting break with prelude.

      // Idle until proven guilty.
      TRACE_MSG("Force idle");
      core->force_idle(break_id);
      break_timer->stop_timer();

      // Update statistics.
      Statistics *stats = core->get_statistics();
      stats->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_PROMPTED);

      if (prelude_count == 0)
        {
          stats->increment_break_counter(break_id, Statistics::STATS_BREAKVALUE_UNIQUE_BREAKS);
          // break_event_signal(BreakEvent::BreakStart);
        }

      // Start prelude.
      goto_stage(BreakStage::Prelude);
      // break_event_signal(BreakEvent::ShowPrelude);
    }
}

//! Starts the break without preludes.
void
BreakControl::force_start_break(workrave::utils::Flags<BreakHint> hint)
{
  TRACE_ENTRY_PAR(break_id);

  break_hint = hint;
  forced_break = (break_hint & (BreakHint::UserInitiated | BreakHint::NaturalBreak)).get() != 0;
  fake_break = false;
  prelude_time = 0;
  user_abort = false;
  delayed_abort = false;

  if (break_timer->is_auto_reset_enabled())
    {
      TRACE_MSG("auto reset enabled");
      int64_t idle = break_timer->get_elapsed_idle_time();
      TRACE_VAR(idle, break_timer->get_auto_reset(), break_timer->is_enabled());
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
      core->force_idle(break_id);
    }

  goto_stage(BreakStage::Taking);
}

//! Stops the break.
/*!
 *  Stopping a break will reset the "number of presented preludes" counter. So,
 *  wrt, "max-preludes", the break will start over when it comes back.
 */
void
BreakControl::stop_break(bool reset_count)
{
  TRACE_ENTRY_PAR(break_id, reset_count);

  suspend_break();

  if (reset_count)
    {
      prelude_count = 0;
    }

  break_event_signal(BreakEvent::BreakStop);
}

//! Suspend the break.
/*!
 *  A suspended break will come back after snooze time. The number of times the
 *  break will come back can be defined with set_max_preludes.
 */
void
BreakControl::suspend_break()
{
  TRACE_ENTRY_PAR(break_id);

  break_hint = BreakHint::Normal;

  goto_stage(BreakStage::None);
}

//! Does the controller need a heartbeat?
bool
BreakControl::need_heartbeat()
{
  return (break_stage != BreakStage::None && break_stage != BreakStage::Snoozed);
}

//! Is the break active ?
BreakControl::BreakState
BreakControl::get_break_state()
{
  BreakState ret = BREAK_INACTIVE;

  if (break_stage == BreakStage::None)
    {
      ret = BREAK_INACTIVE;
    }
  else if (break_stage == BreakStage::Snoozed)
    {
      ret = BREAK_INACTIVE;
    }
  else
    {
      ret = BREAK_ACTIVE;
    }
  return ret;
}

bool
BreakControl::is_taking()
{
  return break_stage == BreakStage::Taking;
}

bool
BreakControl::is_active() const
{
  return break_stage != BreakStage::None && break_stage != BreakStage::Snoozed;
}

bool
BreakControl::is_max_preludes_reached() const
{
  TRACE_ENTRY_PAR(reached_max_prelude, prelude_count, max_number_of_preludes);
  return reached_max_prelude;
}

//! Postpones the active break.
/*!
 *  Postponing a break does not reset the break timer. The break prelude window
 *  will re-appear after snooze time.
 */
void
BreakControl::postpone_break()
{
  if (break_stage == BreakStage::Taking)
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

          break_event_signal(BreakEvent::BreakPostponed);
        }

      // This is to avoid a skip sound...
      user_abort = true;

      // and stop the break.
      stop_break();

      send_postponed();
    }
}

//! Skips the active break.
/*!
 *  Skipping a break resets the break timer.
 */
void
BreakControl::skip_break()
{
  // This is to avoid a skip sound...
  if (break_stage == BreakStage::Taking)
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

      break_event_signal(BreakEvent::BreakSkipped);

      // and stop the break.
      stop_break();

      send_skipped();
    }
}

void
BreakControl::stop_prelude()
{
  delayed_abort = true;
}

//! Sets the maximum number of preludes.
/*!
 *  After the maximum number of preludes, the break either stops bothering the
 *  user, or forces a break. This can be set with set_force_after_preludes.
 */
void
BreakControl::set_max_preludes(int m)
{
  max_number_of_preludes = m;
}

//! Creates and shows the break window.
void
BreakControl::break_window_start()
{
  TRACE_ENTRY_PAR(break_id);

  application->create_break_window(break_id, break_hint);
  update_break_window();
  application->show_break_window();

  // Report state change.
  break_event_signal(forced_break ? BreakEvent::ShowBreakForced : BreakEvent::ShowBreak);
}

//! Creates and shows the prelude window.
void
BreakControl::prelude_window_start()
{
  TRACE_ENTRY_PAR(break_id);

  application->create_prelude_window(break_id);

  application->set_prelude_stage(IApp::PreludeStage::Initial);

  if (!reached_max_prelude)
    {
      application->set_prelude_progress_text(IApp::PreludeProgressText::DisappearsIn);
    }
  else
    {
      application->set_prelude_progress_text(IApp::PreludeProgressText::BreakIn);
    }

  update_prelude_window();

  application->show_break_window();

  if (prelude_count == 1)
    {
      break_event_signal(BreakEvent::BreakStart);
    }

  break_event_signal(BreakEvent::ShowPrelude);
}

bool
BreakControl::action_notify()
{
  TRACE_ENTRY();
  core->stop_prelude(break_id);
  return false; // false: kill listener.
}

//! Initializes this control to the specified state.
void
BreakControl::set_state_data(bool active, const BreakStateData &data)
{
  (void)active;
  forced_break = data.forced_break;
  prelude_count = data.prelude_count;
  prelude_time = data.prelude_time;
}

//! Returns the state of this control.
void
BreakControl::get_state_data(BreakStateData &data)
{
  data.forced_break = forced_break;
  data.prelude_count = prelude_count;
  data.break_stage = (int)break_stage.get();
  data.reached_max_prelude = reached_max_prelude;
  data.prelude_time = prelude_time;
}

//! Plays the specified sound unless action is user initiated.
void
BreakControl::post_event(CoreEvent event)
{
  TRACE_ENTRY_PAR(break_id);
  if (event >= 0)
    {
      core->post_event(event);
    }
}

void
BreakControl::send_postponed()
{
#if defined(HAVE_DBUS)
  workrave::dbus::IDBus::Ptr dbus = core->get_dbus();
  org_workrave_CoreInterface *iface = org_workrave_CoreInterface::instance(dbus);

  if (iface != nullptr)
    {
      iface->BreakPostponed("/org/workrave/Workrave/Core", break_id);
    }
#endif
}

void
BreakControl::send_skipped()
{
#if defined(HAVE_DBUS)
  workrave::dbus::IDBus::Ptr dbus = core->get_dbus();
  org_workrave_CoreInterface *iface = org_workrave_CoreInterface::instance(dbus);

  if (iface != nullptr)
    {
      iface->BreakSkipped("/org/workrave/Workrave/Core", break_id);
    }
#endif
}

std::string
BreakControl::get_current_stage()
{
  return get_stage_text(break_stage);
}

std::string
BreakControl::get_stage_text(BreakStage stage)
{
  std::string progress;
  switch (stage)
    {
    case BreakStage::None:
      progress = "none";
      break;

    case BreakStage::Snoozed:
      progress = "none";
      break;

    case BreakStage::Delayed:
      // Do not send this stage.
      break;

    case BreakStage::Prelude:
      progress = "prelude";
      break;

    case BreakStage::Taking:
      progress = "break";
      break;
    }
  return progress;
}

//! Send DBus signal when break stage changes.
void
BreakControl::send_signal(BreakStage stage)
{
  (void)stage;

#if defined(HAVE_DBUS)
  std::string progress = get_stage_text(stage);

  if (progress != "")
    {
      workrave::dbus::IDBus::Ptr dbus = core->get_dbus();
      org_workrave_CoreInterface *iface = org_workrave_CoreInterface::instance(dbus);

      if (iface != nullptr)
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

boost::signals2::signal<void(BreakEvent)> &
BreakControl::signal_break_event()
{
  return break_event_signal;
}

boost::signals2::signal<void(BreakStage)> &
BreakControl::signal_break_stage_changed()
{
  return break_stage_changed_signal;
}
