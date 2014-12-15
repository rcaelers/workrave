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

#include "BreakStateModel.hh"

#include "IBreak.hh"
#include "IApp.hh"
#include "IActivityMonitor.hh"
#include "Timer.hh"
#include "CoreConfig.hh"

using namespace std;
using namespace workrave::config;

BreakStateModel::Ptr
BreakStateModel::create(BreakId id,
                        IApp *app,
                        Timer::Ptr timer,
                        IActivityMonitor::Ptr activity_monitor,
                        CoreHooks::Ptr hooks)
{
  return Ptr(new BreakStateModel(id, app, timer, activity_monitor, hooks));
}


//! Construct a new BreakStateModel Controller.
BreakStateModel::BreakStateModel(BreakId id,
                                 IApp *app,
                                 Timer::Ptr timer,
                                 IActivityMonitor::Ptr activity_monitor,
                                 CoreHooks::Ptr hooks) :
  break_id(id),
  application(app),
  timer(timer),
  activity_monitor(activity_monitor),
  hooks(hooks),
  break_stage(BreakStage::None),
  prelude_time(0),
  prelude_count(0),
  max_number_of_preludes(2),
  fake_break(false),
  fake_break_count(0),
  user_abort(false),
  delayed_abort(false),
  break_hint(BREAK_HINT_NONE)
{
}


//! Destructor.
BreakStateModel::~BreakStateModel()
{
}


void
BreakStateModel::process()
{
  TRACE_ENTER_MSG("BreakStateModel::process", break_id);

  prelude_time++;

  bool user_is_active = activity_monitor->is_active();

  TRACE_MSG("stage = " << static_cast<std::underlying_type<BreakStage>::type>(break_stage));
  TRACE_MSG("active = " << user_is_active);
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

    case BreakStage::Taking:
      {
        // refresh the break window.
        break_window_update();
        application->refresh_break_window();
      }
      break;
    }

  TRACE_EXIT();
}


//! Starts the break.
void
BreakStateModel::start_break()
{
  TRACE_ENTER_MSG("BreakStateModel::start_break", break_id);

  break_hint = BREAK_HINT_NONE;
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

  TRACE_EXIT();
}


//! Starts the break without preludes.
void
BreakStateModel::force_start_break(BreakHint hint)
{
  TRACE_ENTER_MSG("BreakStateModel::force_start_break", break_id);

  break_hint = hint;
  forced_break = (break_hint & (BREAK_HINT_USER_INITIATED | BREAK_HINT_NATURAL_BREAK) ) != 0;
  fake_break = false;
  prelude_time = 0;
  user_abort = false;
  delayed_abort = false;

  if (timer->is_auto_reset_enabled())
    {
      TRACE_MSG("auto reset enabled");
      int64_t idle = timer->get_elapsed_idle_time();
      TRACE_MSG(idle << " " << timer->get_auto_reset() << " " << timer->is_enabled());
      if (idle >= timer->get_auto_reset() || !timer->is_enabled())
        {
          TRACE_MSG("Faking break");
          fake_break = true;
          fake_break_count = timer->get_auto_reset();
        }
    }

  goto_stage(BreakStage::Taking);

  TRACE_EXIT();
}

void
BreakStateModel::postpone_break()
{
  TRACE_ENTER_MSG("BreakStateModel::postpone_break", break_id);
  if (break_stage == BreakStage::Taking)
    {
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

      // This is to avoid a skip sound...
      user_abort = true;

      // and stop the break.
      stop_break();
    }
}

void
BreakStateModel::skip_break()
{
  // This is to avoid a skip sound...
  if (break_stage == BreakStage::Taking)
    {
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
  TRACE_ENTER("BreakStateModel::stop_break");

  break_hint = BREAK_HINT_NONE;
  goto_stage(BreakStage::None);
  prelude_count = 0;
  fake_break = false;

  break_event_signal(BreakEvent::BreakStop);
  
  TRACE_EXIT();
}

void
BreakStateModel::override(BreakId id)
{
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


void BreakStateModel::set_max_number_of_preludes(int max_number_of_preludes)
{
  this->max_number_of_preludes = max_number_of_preludes;
}

void
BreakStateModel::goto_stage(BreakStage stage)
{
  TRACE_ENTER_MSG("BreakStateModel::goto_stage", break_id << " " << static_cast<std::underlying_type<BreakStage>::type>(stage));
  
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
  TRACE_EXIT();
}

void
BreakStateModel::break_window_start()
{
  TRACE_ENTER_MSG("BreakStateModel::break_window_start", break_id);

  application->hide_break_window();
  application->create_break_window(break_id, break_hint);
  break_window_update();
  application->show_break_window();
  application->refresh_break_window();

  // Report state change.
  break_event_signal(forced_break
                     ? BreakEvent::ShowBreakForced
                     : BreakEvent::ShowBreak);
  
  TRACE_EXIT();
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

  application->set_break_progress((int)idle, (int)duration);
}

void
BreakStateModel::break_window_stop()
{
  TRACE_ENTER_MSG("BreakStateModel::break_window_stop", break_id);

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

  TRACE_EXIT();
}

void
BreakStateModel::prelude_window_start()
{
  TRACE_ENTER_MSG("BreakStateModel::prelude_window_start", break_id);

  prelude_count++;
  prelude_time = 0;
  
  application->hide_break_window();
  application->create_prelude_window(break_id);
  application->set_prelude_stage(IApp::STAGE_INITIAL);

  if (!has_reached_max_preludes())
    {
      application->set_prelude_progress_text(IApp::PROGRESS_TEXT_DISAPPEARS_IN);
    }
  else
    {
      application->set_prelude_progress_text(IApp::PROGRESS_TEXT_BREAK_IN);
    }

  prelude_window_update();

  application->show_break_window();
  application->refresh_break_window();

  if (prelude_count == 1)
    {
      break_event_signal(BreakEvent::BreakStart);
    }

  break_event_signal(BreakEvent::ShowPrelude);
  TRACE_EXIT();
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
  TRACE_ENTER("BreakStateModel::action_notify");
  delayed_abort = true;
  TRACE_EXIT();
  return false;   // false: kill listener.
}

bool
BreakStateModel::has_reached_max_preludes()
{
  return max_number_of_preludes >= 0 && prelude_count >= max_number_of_preludes;
}
