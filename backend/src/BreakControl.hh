// BreakControl.hh --- controller for a single break
//
// Copyright (C) 2001 - 2011 Rob Caelers & Raymond Penners
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

#ifndef BREAKCONTROL_HH
#define BREAKCONTROL_HH

#include "ICore.hh"
#include "ICoreEventListener.hh"
#include "IBreak.hh"
#include "IBreakResponse.hh"
#include "ActivityMonitorListener.hh"
#include "Diagnostics.hh"

using namespace workrave;

// Forward declarion of external interface.
namespace workrave
{
  class IActivityMonitorListener;
  class IApp;
} // namespace workrave

class Core;
class PreludeWindow;
class Timer;

class BreakControl : public ActivityMonitorListener
{
public:
  enum BreakState
  {
    BREAK_ACTIVE,
    BREAK_INACTIVE,
    BREAK_SUSPENDED
  };

  //! Defines what to do when the user is active during a break.
  struct BreakStateData
  {
    bool forced_break;
    int prelude_count;

    int break_stage;
    bool reached_max_prelude;
    int prelude_time;
  };

  BreakControl(BreakId id, const std::string &break_name, IApp *app, Timer *timer);
  virtual ~BreakControl();

  // BreakInterface
  void start_break();
  void force_start_break(BreakHint break_hint);
  void stop_break(bool reset_count = true);
  bool need_heartbeat();
  void heartbeat();
  BreakState get_break_state();
  void set_state_data(bool activate, const BreakStateData &data);
  void get_state_data(BreakStateData &data);
  bool is_taking();
  bool is_max_preludes_reached() const;

  // ActivityMonitorListener
  bool action_notify();

  // Configuration
  void set_max_preludes(int m);

  // BreakResponseInterface
  void postpone_break();
  void skip_break();
  void stop_prelude();

  std::string get_current_stage();

private:
  void break_window_start();
  void prelude_window_start();
  void post_event(CoreEvent event);

private:
  enum BreakStage
  {
    STAGE_NONE,
    STAGE_SNOOZED,
    STAGE_PRELUDE,
    STAGE_TAKING,
    STAGE_DELAYED
  };

  void update_prelude_window();
  void update_break_window();
  void goto_stage(BreakStage stage);
  void suspend_break();
  std::string get_stage_text(BreakStage stage);
  void send_signal(BreakStage stage);
  void send_skipped();
  void send_postponed();

private:
  //! ID of the break controlled by this BreakControl.
  BreakId break_id;

  //! Name of the break controlled by this BreakControl.
  std::string break_name;

  //! The Controller.
  Core *core;

  //! GUI Factory used to create the break/prelude windows.
  IApp *application;

  //! Interface to the timer controlling the break.
  Timer *break_timer;

  //! Current stage in the break.
  TracedField<BreakStage> break_stage;

  //! This is a final prelude prompt, forcing break after this prelude
  bool reached_max_prelude;

  //! How long is the prelude active.
  int prelude_time;

  //! How many times have we preluded (since the limit was reached)
  TracedField<int> prelude_count;

  //! forced break (i.e. RestBreak now, or screenlock)
  TracedField<bool> forced_break;

  //! After how many preludes do we force a break or give up?
  int max_number_of_preludes;

  //! Is this a break that is not controlled by the timer.
  TracedField<bool> fake_break;

  //! Fake break counter.
  time_t fake_break_count;

  //! Break will be stopped because the user pressed postpone/skip.
  TracedField<bool> user_abort;

  //! User became active during delayed break.
  TracedField<bool> delayed_abort;

  //! Break hint if break has been started.
  BreakHint break_hint;
};

#endif // BREAKCONTROL_HH
