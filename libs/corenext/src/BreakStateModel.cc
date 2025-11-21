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
#  include "config.h"
#endif

#include "debug.hh"

#include <cassert>
#include <string>

#include "BreakStateModel.hh"

#include "core/IBreak.hh"
#include "core/IApp.hh"
#include "IActivityMonitor.hh"
#include "Timer.hh"
#include "core/CoreConfig.hh"

using namespace std;
using namespace workrave;
using namespace workrave::config;

BreakStateModel::BreakStateModel(BreakId id,
                                 IApp *app,
                                 Timer::Ptr timer,
                                 IActivityMonitor::Ptr activity_monitor,
                                 CoreHooks::Ptr hooks)
  : break_id(id)
  , application(app)
  , timer(timer)
  , activity_monitor(activity_monitor)
  , hooks(hooks)
  , break_stage(BreakStage::None)
  , prelude_time(0)
  , prelude_count(0)
  , max_number_of_preludes(2)
  , fake_break(false)
  , fake_break_count(0)
  , user_abort(false)
  , delayed_abort(false)
  , break_hint(BreakHint::Normal)
{
}

void
BreakStateModel::process()
{
  TRACE_ENTRY_PAR(break_id);

  TRACE_MSG("stage = {}", break_stage);

  prelude_time++;
  bool user_is_active = activity_monitor->is_active();

  TRACE_MSG("active = {}", user_is_active);
  TRACE_MSG("prelude time = {}", prelude_time);

  switch (break_stage)
    {
    case BreakStage::None:
      break;

    case BreakStage::Snoozed:
      break;

    case BreakStage::Delayed:
      {
        if (prelude_time > 35 || delayed_abort)
          {
            // User become active during delayed break.
            goto_stage(BreakStage::Snoozed);
          }
        else if (!user_is_active)
          {
            // User is idle.
            goto_stage(BreakStage::Taking);
          }
      }
      break;

    case BreakStage::Prelude:
      {
        prelude_window_update();
        application->refresh_break_window();

        if (!user_is_active)
          {
            // User is idle.
            if (prelude_time >= 10)
              {
                // User is idle and prelude is visible for at least 10s.
                goto_stage(BreakStage::Taking);
              }
          }
        else if (prelude_time >= 30)
          {
            // User is not idle and the prelude is visible for 30s.
            if (has_reached_max_preludes())
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
        break_window_update();
        application->refresh_break_window();
      }
      break;
    }
}

//! Starts the break.
void
BreakStateModel::start_break()
{
  TRACE_ENTRY_PAR(break_id, break_stage);

  break_hint = BreakHint::Normal;
  forced_break = false;
  fake_break = false;
  prelude_time = 0;
  user_abort = false;
  delayed_abort = false;

  if (max_number_of_preludes >= 0 && prelude_count >= max_number_of_preludes)
    {
      goto_stage(BreakStage::Taking);
    }
  else
    {
      goto_stage(BreakStage::Prelude);
    }
}

//! Starts the break without preludes.
void
BreakStateModel::force_start_break(workrave::utils::Flags<BreakHint> hint)
{
  TRACE_ENTRY_PAR(break_id, break_stage);

  break_hint = hint;
  forced_break = (break_hint & (BreakHint::UserInitiated | BreakHint::NaturalBreak)).get() != 0;
  fake_break = false;
  prelude_time = 0;
  user_abort = false;
  delayed_abort = false;

  if (timer->is_auto_reset_enabled())
    {
      TRACE_MSG("auto reset enabled");
      int64_t idle = timer->get_elapsed_idle_time();
      TRACE_VAR(idle, timer->get_auto_reset(), timer->is_enabled());
      if (idle >= timer->get_auto_reset() || !timer->is_enabled())
        {
          TRACE_MSG("Faking break");
          fake_break = true;
          fake_break_count = timer->get_auto_reset();
        }
    }

  goto_stage(BreakStage::Taking);
}

void
BreakStateModel::postpone_break()
{
  TRACE_ENTRY_PAR(break_id, break_stage);
  if (break_stage == BreakStage::Taking)
    {
      // This is to avoid a skip sound...
      user_abort = true;

      if (!forced_break)
        {
          if (!fake_break)
            {
              // Snooze the timer.
              TRACE_MSG("Snoozing timer");
              timer->snooze_timer();
            }

          break_event_signal(BreakEvent::BreakPostponed);
        }

      // and stop the break.
      stop_break();
    }
}

void
BreakStateModel::skip_break()
{
  TRACE_ENTRY_PAR(break_id, break_stage);

  if (break_stage == BreakStage::Taking)
    {
      // This is to avoid a skip sound...
      user_abort = true;

      if (break_id == BREAK_ID_DAILY_LIMIT)
        {
          // Make sure the daily limit remains silent after skipping.
          timer->inhibit_snooze();
        }
      else
        {
          // Reset the restbreak timer.
          timer->reset_timer();
        }

      break_event_signal(BreakEvent::BreakSkipped);

      // and stop the break.
      stop_break();
    }
}

//! Stops the break.
void
BreakStateModel::stop_break()
{
  TRACE_ENTRY_PAR(break_id, break_stage);

  break_hint = BreakHint::Normal;
  goto_stage(BreakStage::None);
  prelude_count = 0;
  fake_break = false;

  break_event_signal(BreakEvent::BreakStop);
}

void
BreakStateModel::override(BreakId id)
{
  TRACE_ENTRY_PAR(break_id, break_stage);
  int max_preludes = CoreConfig::break_max_preludes(break_id)();

  if (break_id != id)
    {
      int max_preludes_other = CoreConfig::break_max_preludes(id)();
      if (max_preludes_other != -1 && max_preludes_other < max_preludes)
        {
          max_preludes = max_preludes_other;
        }
    }

  max_number_of_preludes = max_preludes;
}

boost::signals2::signal<void(BreakEvent)> &
BreakStateModel::signal_break_event()
{
  return break_event_signal;
}

boost::signals2::signal<void(BreakStage)> &
BreakStateModel::signal_break_stage_changed()
{
  return break_stage_changed_signal;
}

bool
BreakStateModel::is_taking() const
{
  return break_stage == BreakStage::Taking;
}

bool
BreakStateModel::is_active() const
{
  return break_stage != BreakStage::None && break_stage != BreakStage::Snoozed;
}

BreakStage
BreakStateModel::get_break_stage() const
{
  return break_stage;
}

void
BreakStateModel::set_max_number_of_preludes(int max_number_of_preludes)
{
  this->max_number_of_preludes = max_number_of_preludes;
}

void
BreakStateModel::goto_stage(BreakStage stage)
{
  TRACE_ENTRY_PAR(break_id, break_stage, stage);

  switch (stage)
    {
    case BreakStage::Delayed:
      {
        activity_monitor->set_listener(shared_from_this());
      }
      break;

    case BreakStage::None:
      {
        if (break_stage == BreakStage::Prelude)
          {
            prelude_window_stop();
          }
        else if (break_stage == BreakStage::Taking)
          {
            break_window_stop();
          }
      }
      break;

    case BreakStage::Snoozed:
      {
        prelude_window_stop();
      }
      break;

    case BreakStage::Prelude:
      {
        prelude_window_start();
      }
      break;

    case BreakStage::Taking:
      {
        // Start the break.
        break_window_start();
      }
      break;
    }

  break_stage = stage;
  break_stage_changed_signal(stage);
}

void
BreakStateModel::break_window_start()
{
  TRACE_ENTRY_PAR(break_id);

  application->hide_break_window();
  application->create_break_window(break_id, break_hint);
  break_window_update();
  application->show_break_window();
  application->refresh_break_window();

  // Report state change.
  break_event_signal(forced_break ? BreakEvent::ShowBreakForced : BreakEvent::ShowBreak);
}

void
BreakStateModel::break_window_update()
{
  int64_t duration = timer->get_auto_reset();
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
      idle = timer->get_elapsed_idle_time();
    }

  if (idle > duration)
    {
      idle = duration;
    }

  application->set_break_progress(static_cast<int>(idle), static_cast<int>(duration));
}

void
BreakStateModel::break_window_stop()
{
  TRACE_ENTRY_PAR(break_id);

  application->hide_break_window();

  if (!fake_break)
    {
      // Update statistics and play sound if the break end
      // was "natural"
      int64_t idle = timer->get_elapsed_idle_time();
      int64_t reset = timer->get_auto_reset();

      if (idle >= reset && !user_abort)
        {
          // Breaks end without user skip/postponing it.
          break_event_signal(BreakEvent::BreakTaken);
        }
    }
  break_event_signal(BreakEvent::BreakIdle);
}

void
BreakStateModel::prelude_window_start()
{
  TRACE_ENTRY_PAR(break_id);

  prelude_count++;
  prelude_time = 0;

  application->hide_break_window();
  application->create_prelude_window(break_id);
  application->set_prelude_stage(IApp::PreludeStage::Initial);

  if (!has_reached_max_preludes())
    {
      application->set_prelude_progress_text(IApp::PreludeProgressText::DisappearsIn);
    }
  else
    {
      application->set_prelude_progress_text(IApp::PreludeProgressText::BreakIn);
    }

  prelude_window_update();

  application->show_break_window();
  application->refresh_break_window();

  if (prelude_count == 1)
    {
      break_event_signal(BreakEvent::BreakStart);
    }

  break_event_signal(BreakEvent::ShowPrelude);
}
void
BreakStateModel::prelude_window_update()
{
  application->set_break_progress(prelude_time, 29);
}

void
BreakStateModel::prelude_window_stop()
{
  application->hide_break_window();
  if (!forced_break)
    {
      break_event_signal(BreakEvent::BreakIgnored);
    }
  break_event_signal(BreakEvent::BreakIdle);
}

bool
BreakStateModel::action_notify()
{
  TRACE_ENTRY();
  delayed_abort = true;
  return false; // false: kill listener.
}

bool
BreakStateModel::has_reached_max_preludes()
{
  return max_number_of_preludes >= 0 && prelude_count >= max_number_of_preludes;
}
