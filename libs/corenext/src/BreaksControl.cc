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

#include <iostream>
#include <fstream>
#include <sstream>

#include "utils/Paths.hh"
#include "utils/TimeSource.hh"
#include "dbus/IDBus.hh"

#include "BreaksControl.hh"
#include "core/CoreConfig.hh"
#include "TimerActivityMonitor.hh"

static const char *WORKRAVESTATE = "WorkRaveState";
static const int SAVESTATETIME = 60;

using namespace std;
using namespace workrave;
using namespace workrave::utils;
using namespace workrave::dbus;

BreaksControl::BreaksControl(IApp *app,
                             IActivityMonitor::Ptr activity_monitor,
                             CoreModes::Ptr modes,
                             Statistics::Ptr statistics,
                             std::shared_ptr<IDBus> dbus,
                             CoreHooks::Ptr hooks)
  : application(app)
  , activity_monitor(activity_monitor)
  , modes(modes)
  , statistics(statistics)
  , dbus(dbus)
  , hooks(hooks)
  , insist_policy(InsistPolicy::Halt)
  , active_insist_policy(InsistPolicy::Invalid)
{
}

BreaksControl::~BreaksControl()
{
  save_state();
}

void
BreaksControl::init()
{
  connect(modes->signal_operation_mode_changed(), this, [this](auto &&mode) {
    on_operation_mode_changed(std::forward<decltype(mode)>(mode));
  });

  for (BreakId break_id = BREAK_ID_MICRO_BREAK; break_id < BREAK_ID_SIZEOF; break_id++)
    {
      string break_name = CoreConfig::get_break_name(break_id);

      timers[break_id] = std::make_shared<Timer>(break_name);
      timers[break_id]->enable();

      breaks[break_id] = std::make_shared<Break>(break_id,
                                                 application,
                                                 timers[break_id],
                                                 activity_monitor,
                                                 statistics,
                                                 dbus,
                                                 hooks);
      connect(breaks[break_id]->signal_break_event(), this, [this, break_id](auto &&event) {
        on_break_event(break_id, std::forward<decltype(event)>(event));
      });
    }

  reading_activity_monitor = std::make_shared<ReadingActivityMonitor>(activity_monitor, modes);
  reading_activity_monitor->init();

  microbreak_activity_monitor = std::make_shared<TimerActivityMonitor>(activity_monitor, timers[BREAK_ID_MICRO_BREAK]);

  load_state();
}

IBreak::Ptr
BreaksControl::get_break(BreakId break_id)
{
  return breaks[break_id];
}

void
BreaksControl::force_idle()
{
  activity_monitor->force_idle();
  microbreak_activity_monitor->force_idle();
  reading_activity_monitor->force_idle();

  for (auto &timer: timers)
    {
      timer->stop_timer();
    }
}

void
BreaksControl::stop_all_breaks()
{
  for (auto &b: breaks)
    {
      if (b->is_active())
        {
          b->stop_break();
        }
    }
}

//! Forces the start of the specified break.
void
BreaksControl::force_break(BreakId break_id, workrave::utils::Flags<BreakHint> break_hint)
{
  TRACE_ENTRY_PAR(break_id, break_hint);

  if (break_id == BREAK_ID_REST_BREAK && breaks[BREAK_ID_MICRO_BREAK]->is_active())
    {
      breaks[BREAK_ID_MICRO_BREAK]->stop_break();
      TRACE_MSG("Resuming Micro break");
    }

  breaks[break_id]->force_start_break(break_hint);
}

//! Periodic heartbeat.
void
BreaksControl::heartbeat()
{
  TRACE_ENTRY();
  bool user_is_active = false;
  if (modes->get_usage_mode() == UsageMode::Reading)
    {
      user_is_active = reading_activity_monitor->is_active();
    }
  else
    {
      user_is_active = activity_monitor->is_active();
    }

  // Perform timer processing.
  process_timers(user_is_active);

  // Send heartbeats to other components.
  for (auto &b: breaks)
    {
      b->process();
    }

  // Make state persistent.
  if (TimeSource::get_monotonic_time_sec() % SAVESTATETIME == 0)
    {
      statistics->update();
      save_state();
    }
}

//! Processes all timers.
void
BreaksControl::process_timers(bool user_is_active)
{
  TRACE_ENTRY();
  for (int i = BREAK_ID_DAILY_LIMIT; i > BREAK_ID_NONE; i--)
    {
      auto break_id = static_cast<BreakId>(i);

      bool user_is_active_for_break = user_is_active;
      if (breaks[break_id]->is_microbreak_used_for_activity())
        {
          user_is_active_for_break = microbreak_activity_monitor->is_active();
        }

      TimerEvent event = timers[break_id]->process(user_is_active_for_break);

      if (breaks[break_id]->is_enabled())
        {
          switch (event)
            {
            case TIMER_EVENT_LIMIT_REACHED:
              TRACE_MSG("limit reached {}", break_id);
              if (!breaks[break_id]->is_active() && modes->get_active_operation_mode() == OperationMode::Normal)
                {
                  start_break(break_id);
                }
              break;

            case TIMER_EVENT_NATURAL_RESET:
            case TIMER_EVENT_RESET:
              TRACE_MSG("limi reset {}", break_id);
              if (breaks[break_id]->is_active())
                {
                  breaks[break_id]->stop_break();
                }

              if (break_id == BREAK_ID_DAILY_LIMIT)
                {
                  for (BreakId i = BREAK_ID_MICRO_BREAK; i < BREAK_ID_SIZEOF; i++)
                    {
                      breaks[i]->daily_reset();
                    }
                  modes->daily_reset();
                }
              break;

            case TIMER_EVENT_NONE:
              break;
            }
        }
    }
}

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
      Timer::Ptr rb_timer = timers[BREAK_ID_REST_BREAK];

      bool activity_sensitive = true; // get_timer(BREAK_ID_REST_BREAK)->is_activity_sensitive();

      // Only advance when
      // 0. It is activity sensitive
      // 1. we have a next limit reached time.
      if (activity_sensitive && rb_timer->get_next_limit_time() > 0)
        {
          Timer::Ptr timer = timers[break_id];

          int64_t duration = timer->get_auto_reset();
          int64_t now = TimeSource::get_monotonic_time_sec_sync();

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

  // Stop microbreak when a restbreak starts. should not happened.
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

void
BreaksControl::set_insist_policy(InsistPolicy p)
{
  TRACE_ENTRY_PAR(p);
  TRACE_MSG("current {}", active_insist_policy);

  if (active_insist_policy != InsistPolicy::Invalid && insist_policy != p)
    {
      TRACE_MSG("refreeze {}", active_insist_policy);
      defrost();
      insist_policy = p;
      freeze();
    }
  else
    {
      insist_policy = p;
      freeze();
    }
}

//! Exceute the insist policy.
void
BreaksControl::freeze()
{
  InsistPolicy policy = insist_policy;

  switch (policy)
    {
    case InsistPolicy::Ignore:
      {
        // Ignore all activity during break by suspending the activity monitor.
        activity_monitor->suspend();
      }
      break;
    case InsistPolicy::Halt:
      {
        // Halt timer when the user is active.
        set_freeze_all_breaks(true);
      }
      break;
    case InsistPolicy::Reset:
      // reset the timer when the user becomes active.
      // default.
      break;

    case InsistPolicy::Invalid:
      break;
    }

  active_insist_policy = policy;
}

//! Undo the insist policy.
void
BreaksControl::defrost()
{
  switch (active_insist_policy)
    {
    case InsistPolicy::Ignore:
      {
        // Resumes the activity monitor, if not suspended.
        if (modes->get_active_operation_mode() != OperationMode::Suspended)
          {
            activity_monitor->resume();
          }
      }
      break;
    case InsistPolicy::Halt:
      {
        // Desfrost timers.
        set_freeze_all_breaks(false);
      }
      break;

    case InsistPolicy::Invalid:
    case InsistPolicy::Reset:
      break;
    }

  active_insist_policy = InsistPolicy::Invalid;
}

void
BreaksControl::set_freeze_all_breaks(bool freeze)
{
  for (BreakId break_id = BREAK_ID_MICRO_BREAK; break_id <= BREAK_ID_DAILY_LIMIT; break_id++)
    {
      if (!breaks[break_id]->is_microbreak_used_for_activity())
        {
          timers[break_id]->freeze_timer(freeze);
        }
    }
}

void
BreaksControl::on_operation_mode_changed(OperationMode operation_mode)
{
  if (operation_mode == OperationMode::Suspended || operation_mode == OperationMode::Quiet)
    {
      stop_all_breaks();
    }
  if (operation_mode == OperationMode::Suspended)
    {
      reading_activity_monitor->suspend();
    }
  else
    {
      reading_activity_monitor->resume();
    }
}

void
BreaksControl::on_break_event(BreakId break_id, BreakEvent event)
{
  TRACE_ENTRY_PAR(break_id, static_cast<std::underlying_type<BreakEvent>::type>(event));
  switch (event)
    {
    case BreakEvent::BreakIdle:
      defrost();
      break;

    case BreakEvent::ShowPrelude:
      force_idle();
      break;

    case BreakEvent::ShowBreak:
    case BreakEvent::ShowBreakForced:
      force_idle();
      freeze();
      break;

    default:
      break;
    }

  if (modes->get_usage_mode() == UsageMode::Reading)
    {
      reading_activity_monitor->handle_break_event(break_id, event);
    }
}

//! Saves the current state.
void
BreaksControl::save_state() const
{
  std::filesystem::path path = Paths::get_state_directory() / "state";
  ofstream stateFile(path.string());

  stateFile << "WorkRaveState 3" << endl << TimeSource::get_real_time_sec() << endl;

  for (BreakId break_id = BREAK_ID_MICRO_BREAK; break_id < BREAK_ID_SIZEOF; break_id++)
    {
      const string stateStr = timers[break_id]->serialize_state();

      stateFile << stateStr << endl;
    }

  stateFile.close();
}

//! Loads the current state.
void
BreaksControl::load_state()
{
  std::filesystem::path path = Paths::get_state_directory() / "state";

#if defined(HAVE_TESTS)
  if (hooks->hook_load_timer_state())
    {
      if (hooks->hook_load_timer_state()(timers))
        {
          return;
        }
    }
#endif
  ifstream state_file(path.string());

  int version = 0;
  bool ok = state_file.good();

  if (ok)
    {
      string tag;
      state_file >> tag;

      ok = (tag == WORKRAVESTATE);
    }

  if (ok)
    {
      state_file >> version;

      ok = (version >= 1 && version <= 3);
    }

  if (ok)
    {
      int64_t saveTime = 0;
      state_file >> saveTime;
    }

  while (ok && !state_file.eof())
    {
      string id;
      state_file >> id;

      for (BreakId break_id = BREAK_ID_MICRO_BREAK; break_id < BREAK_ID_SIZEOF; break_id++)
        {
          if (timers[break_id]->get_id() == id)
            {
              string state;
              getline(state_file, state);

              timers[break_id]->deserialize_state(state, version);
              break;
            }
        }
    }
}
