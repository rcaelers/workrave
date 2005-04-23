// BreakControl.hh --- controller for a single break
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Rob Caelers & Raymond Penners
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
#include "ActivityMonitorListener.hh"

class ActivityMonitorListenerInterface;
class Core;
class AppInterface;
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
    bool reached_max_postpone;
  };
  
  BreakControl(BreakId id, Core *core, AppInterface *app, Timer *timer);
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
  void set_max_postpone(int m);
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

  //! Have we seen too many preludes and need to make the break non-ignorable
  bool reached_max_postpone;

  //! After how many preludes do we force a break or give up?
  int max_number_of_preludes;

  //! After how many preludes do we make the break not ignorable?
  int max_number_of_postpones;

  //! Can the use explicitly ignore the break? (window setting)
  bool ignorable_break;

  //! Can the use explicitly ignore the break? (configuration setting)
  bool config_ignorable_break;

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
