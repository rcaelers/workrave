// Break.hh --- controller for a single break
//
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

#ifndef BREAK_HH
#define BREAK_HH

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "config/Config.hh"

#include "IBreakSupport.hh"
#include "IActivityMonitor.hh"
#include "IActivityMonitorListener.hh"
#include "Statistics.hh"
#include "IBreak.hh"
#include "Timer.hh"

using namespace workrave;
using namespace workrave::config;
using namespace workrave::dbus;

// Forward declarion of external interface.
namespace workrave {
  class IApp;
}

// Forward declarion of internals.
class DayTimePred;

class Break :
  public IBreak,
  public IConfiguratorListener,
  public IActivityMonitorListener,
  public boost::enable_shared_from_this<Break>
{
public:
  typedef boost::shared_ptr<Break> Ptr;

public:
  static Ptr create(BreakId id,
                    IApp *app,
                    IBreakSupport::Ptr break_support,
                    IActivityMonitor::Ptr activity_monitor,
                    Statistics::Ptr statistics,
                    IConfigurator::Ptr configurator,
                    DBus::Ptr dbus);

  Break(BreakId id,
        IApp *app,
        IBreakSupport::Ptr break_support, 
        IActivityMonitor::Ptr activity_monitor,
        Statistics::Ptr statistics,
        IConfigurator::Ptr configurator,
        DBus::Ptr dbus);
  virtual ~Break();

  // Internal

  void init();

  TimerEvent process_timer(ActivityState state);
  void process_break();

  void force_start_break(BreakHint break_hint);
  void start_break();
  void stop_break();
  void freeze_break(bool freeze);
  void force_idle();
  void daily_reset();
  
  Timer::Ptr get_timer() const;
  //  void set_usage_mode(UsageMode mode);
  //  void update_usage_mode();
  void override(BreakId id);

  // IBreak
  virtual boost::signals2::signal<void(BreakEvent)> &signal_break_event();

  virtual std::string get_name() const; 
  virtual bool is_enabled() const; 
  virtual bool is_running() const;
  virtual bool is_taking() const;
  virtual bool is_active() const;
  virtual int64_t get_elapsed_time() const;
  virtual int64_t get_elapsed_idle_time() const;
  virtual int64_t get_auto_reset() const;
  virtual bool is_auto_reset_enabled() const;
  virtual int64_t get_limit() const;
  virtual bool is_limit_enabled() const;
  virtual int64_t get_total_overdue_time() const;
  virtual void postpone_break();
  virtual void skip_break();

private:
  enum BreakStage { STAGE_NONE,
                    STAGE_SNOOZED,
                    STAGE_PRELUDE,
                    STAGE_TAKING,
                    STAGE_DELAYED
  };

  void goto_stage(BreakStage stage);
  
  void break_window_start();
  void break_window_update();
  void prelude_window_start();
  void prelude_window_update();

  // IActivityMonitorListener
  bool action_notify();

  void send_signal(BreakStage stage);
  void update_statistics();
  DayTimePred *create_time_pred(std::string spec);
  void load_timer_config();
  void load_break_control_config();
  void config_changed_notify(const std::string &key);
  
private:
  //! ID of the break controlled by this Break.
  BreakId break_id;

  //! GUI Factory used to create the break/prelude windows.
  IApp *application;

  //! .
  IBreakSupport::Ptr break_support;

  //!
  IActivityMonitor::Ptr activity_monitor;
  
  //!
  IActivityMonitor::Ptr timer_activity_monitor;

  //!
  Statistics::Ptr statistics;
  
  //! The Configurator
  IConfigurator::Ptr configurator;

  //!
  DBus::Ptr dbus;
  
  //! Interface to the timer controlling the break.
  Timer::Ptr break_timer;

  //! Current stage in the break.
  BreakStage break_stage;

  //! This is a final prelude prompt, forcing break after this prelude
  bool reached_max_prelude;

  //! How long is the prelude active.
  int prelude_time;

  //! forced break (i.e. RestBreak now, or screenlock)
  bool forced_break;

  //! How many times have we preluded (since the limit was reached)
  int prelude_count;

  //! After how many preludes do we force a break or give up?
  int max_number_of_preludes;

  //! Is this a break that is not controlled by the timer.
  bool fake_break;

  //! Fake break counter.
  int64_t fake_break_count;

  //! Break will be stopped because the user pressed postpone/skip.
  bool user_abort;

  //! User became active during delayed break.
  bool delayed_abort;

  //! Break hint if break has been started.
  BreakHint break_hint;

  //! Name of the break (used in configuration)
  std::string break_name;

  //! Break enabled?
  bool enabled;

  //!
  boost::signals2::signal<void(BreakEvent)> break_event_signal;
};

#endif // BREAK_HH
