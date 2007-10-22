// BreakControl.hh --- controller for a single break
//
// Copyright (C) 2001 - 2007 Rob Caelers & Raymond Penners
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
// $Id$
//

#ifndef BREAKCONTROL_HH
#define BREAKCONTROL_HH

#include "ICore.hh"
#include "CoreEventListener.hh"
#include "IBreak.hh"
#include "IBreakResponse.hh"
#include "ActivityMonitorListener.hh"

using namespace workrave;

// Forward declarion of external interface.
namespace workrave {
  class IActivityMonitorListener;
  class IApp;
}

class Core;
class PreludeWindow;
class Timer;

class BreakControl :
  public ActivityMonitorListener
{
public:
  enum BreakState { BREAK_ACTIVE, BREAK_INACTIVE, BREAK_SUSPENDED };

  //! Defines what to do when the user is active during a break.
  struct BreakStateData
  {
    bool user_initiated;
    int prelude_count;
    int postponable_count;

    int break_stage;
    bool reached_max_prelude;
    int prelude_time;
  };

  BreakControl(BreakId id, IApp *app, Timer *timer);
  virtual ~BreakControl();

  // BreakInterface
  void start_break();
  void force_start_break(bool initiated_by_user = true);
  void stop_break(bool forced_stop = false);
  bool need_heartbeat();
  void heartbeat();
  BreakState get_break_state();
  void set_state_data(bool activate, const BreakStateData &data);
  void get_state_data(BreakStateData &data);

  // ActivityMonitorListener
  bool action_notify();

  // Configuration
  void set_max_preludes(int m);
  void set_ignorable_break(bool i);
  
  // BreakResponseInterface
  void postpone_break();
  void skip_break();
  void stop_prelude();

private:
  void break_window_start();
  void prelude_window_start();
  void post_event(CoreEvent event);

private:
  enum BreakStage { STAGE_NONE,
                    STAGE_SNOOZED,
                    STAGE_PRELUDE,
                    STAGE_TAKING,
                    STAGE_DELAYED
  };

  void update_prelude_window();
  void update_break_window();
  void goto_stage(BreakStage stage);
  void suspend_break();
  void send_signal(BreakStage stage);

private:
  //! ID of the break controlled by this BreakControl.
  BreakId break_id;

  //! The Controller.
  Core *core;

  //! GUI Factory used to create the break/prelude windows.
  IApp *application;

  //! Interface to the timer controlling the break.
  Timer *break_timer;

  //! Current stage in the break.
  BreakStage break_stage;

  //! This is a final prelude prompt, forcing break after this prelude
  bool reached_max_prelude;

  //! How long is the prelude active.
  int prelude_time;

  //! (User initiated/seld-inflicted) forced break (i.e. RestBreak now)
  bool user_initiated;

  //! How many times have we preluded (since the limit was reached)
  int prelude_count;

  //! Total number of time the user could have postponed the break.
  /*! This includes the implicit (ignoring the prelude windows) as well as the
   *  explicit (clicking postpone) number of times the break was postponed.
   */
  int postponable_count;

  //! After how many preludes do we force a break or give up?
  int max_number_of_preludes;

  //! Can the use explicitly ignore the break?
  bool ignorable_break;

  //! Is this a break that is not controlled by the timer.
  bool fake_break;

  //! Fake break counter.
  int fake_break_count;

  //! Break will be stopped because the user pressed postpone/skip.
  bool user_abort;

  //! User became active during delayed break.
  bool delayed_abort;
};

#endif // BREAKCONTROL_HH
