// BreakControl.hh --- controller for a single break
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
// $Id$
//

#ifndef BREAKCONTROL_HH
#define BREAKCONTROL_HH

#include "CoreInterface.hh"
#include "CoreEventListener.hh"
#include "BreakInterface.hh"
#include "BreakResponseInterface.hh"

class ActivityMonitorListenerInterface;
class Core;
class AppInterface;
class PreludeWindow;
class Timer;

class BreakControl
{
public:
  enum BreakState { BREAK_ACTIVE, BREAK_INACTIVE, BREAK_SUSPENDED };

  enum InsistPolicy
    {
      //! Uninitialized policy
      INSIST_POLICY_INVALID,

      //! Halts the timer on activity.
      INSIST_POLICY_HALT,

      //! Resets the timer on activity.
      INSIST_POLICY_RESET,

      //! Ignores all activity.
      INSIST_POLICY_SUSPEND,

      //! Number of policies.
      INSIST_POLICY_SIZEOF
    };
  
  //! Defines what to do when the user is active during a break.
  struct BreakStateData
  {
    bool forced_break;
    int prelude_count;

    int break_stage;
    bool final_prelude;
    int prelude_time;
  };
  
  BreakControl(BreakId id, Core *core, AppInterface *app, Timer *timer);
  virtual ~BreakControl();

  // BreakInterface
  void start_break();
  void force_start_break();
  void stop_break();
  bool need_heartbeat();
  void heartbeat();
  BreakState get_break_state();
  void set_state_data(bool activate, const BreakStateData &data);
  void get_state_data(BreakStateData &data);

  // Configuration
  void set_force_after_preludes(bool f);
  void set_max_preludes(int m);
  void set_insist_break(bool i);
  void set_insist_policy(InsistPolicy p);
  void set_ignorable_break(bool i);
  
  // BreakResponseInterface
  void postpone_break();
  void skip_break();
  void stop_prelude();
  
private:
  void break_window_start();
  void prelude_window_start();
  void post_event(CoreEvent event);
  
  void freeze();
  void defrost();

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

private:
  //! ID of the break controlled by this BreakControl. 
  BreakId break_id;
  
  //! The Controller.
  Core *core;

  //! GUI Factory used to create the break/prelude windows.
  AppInterface *application;
  
  //! Interface to the timer controlling the break.
  Timer *break_timer;
  
  //! Current stage in the break.
  BreakStage break_stage;

  //! This is a final prelude prompt, forcing break after this prelude
  bool final_prelude;
  
  //! How long is the prelude active.
  int prelude_time;

  //! (User initiated/seld-inflicted) forced break (i.e. RestBreak now)
  bool forced_break;

  //! User initiated skip/start break.
  bool user_initiated;

  //! How many times have we preluded (since the limit was reached)
  int prelude_count;

  //! After how many preludes do we force a break or give up?
  int number_of_preludes;

  //! After 'number_of_preludes' do we force a break ?
  bool force_after_prelude;
  
  //! Can the user continue during break?
  bool insist_break;

  //! Can the use explicitly ignore the break?
  bool ignorable_break;

  //! What to do with activity during insisted break?
  InsistPolicy insist_policy;

  //! Policy currently in effect.
  InsistPolicy active_insist_policy;

  //! Is this a break that is not controlled by the timer.
  bool fake_break;

  //! Fake break counter.
  int fake_break_count;
};

#endif // BREAKCONTROL_HH
