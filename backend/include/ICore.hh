// ICore.hh --- The main controller interface
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Rob Caelers <robc@krandor.org>
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

#ifndef ICORE_HH
#define ICORE_HH

#include <string>

class IBreak;
class ITimer;
class IActivityMonitor;
class ActivityMonitorListener;
class Configurator;
class IApp;
class IStatistics;
class IDistributionManager;
class CoreEventListener;

//! Mode
enum OperationMode
  {
    //! Breaks are reported to the user when due.
    OPERATION_MODE_NORMAL=0,
    
    //! Monitoring is suspended.
    OPERATION_MODE_SUSPENDED,

    //! Breaks are not reported to the user when due.
    OPERATION_MODE_QUIET,

    //! Number of modes.
    OPERATION_MODE_SIZEOF
  };

  
//! ID of a break.
enum BreakId
  {
    BREAK_ID_NONE = -1,
    BREAK_ID_MICRO_BREAK = 0,
    BREAK_ID_REST_BREAK,
    BREAK_ID_DAILY_LIMIT,
    BREAK_ID_SIZEOF
  };

  
class ICore
{
public:
  virtual ~ICore() {}
  
  enum InsistPolicy
    {
      //! Uninitialized policy
      INSIST_POLICY_INVALID,

      //! Halts the timer on activity.
      INSIST_POLICY_HALT,

      //! Resets the timer on activity.
      INSIST_POLICY_RESET,

      //! Ignores all activity.
      INSIST_POLICY_IGNORE,

      //! Number of policies.
      INSIST_POLICY_SIZEOF
    };

  //! Initialize the Core Control. Must be called first.
  virtual void init(int argc, char **argv, IApp *app, char *display) = 0;

  //! Periodic heartbeat. The GUI *MUST* call this method every second.
  virtual void heartbeat() = 0;

  //! Forces a break of the specified type.
  virtual void force_break(BreakId id, bool initiated_by_user) = 0;
 
  //! Returns the break interface of the specified type.
  virtual IBreak *get_break(BreakId id) = 0;

  //! Returns the timer interface of the specified break.
  virtual ITimer *get_timer(BreakId id) const = 0;

  //! Returns the statistics interface.
  virtual IStatistics *get_statistics() const = 0;

  //! Returns the activity monitor interface
  virtual IActivityMonitor *get_activity_monitor() const = 0;

#ifdef HAVE_DISTRIBUTION
  //! Returns the distribution manager (if available).
  virtual IDistributionManager *get_distribution_manager() const = 0;
#endif
  
  //! Returns the current operational mode.
  virtual OperationMode get_operation_mode() = 0;

  //! Sets the operational mode.
  virtual OperationMode set_operation_mode(OperationMode mode) = 0;

  //! Sets the callback for activity monitor events.
  virtual void set_core_events_listener(CoreEventListener *l) = 0;

  //! Notifies the core that the computer will enter or leave powersave (suspend/hibernate)
  virtual void set_powersave(bool down) = 0;

  virtual void set_insist_policy(InsistPolicy p) = 0;
  
#ifndef NDEBUG
  virtual void test_me() = 0;
#endif

};

#endif // ICORE_HH
