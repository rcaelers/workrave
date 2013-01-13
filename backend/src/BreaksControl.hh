// BreaksControl.hh --- The main controller
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

#ifndef BREAKSCONTROL_HH
#define BREAKSCONTROL_HH

#include <boost/enable_shared_from_this.hpp>

#include "config/Config.hh"

#include "ActivityMonitor.hh"
#include "Statistics.hh"
#include "Break.hh"
#include "IBreakSupport.hh"

using namespace workrave;
using namespace workrave::config;

class BreaksControl :
  public IBreakSupport,
  public boost::enable_shared_from_this<BreaksControl>
{
public:
  typedef boost::shared_ptr<BreaksControl> Ptr;

  static Ptr create(IApp *app,
                    IActivityMonitor::Ptr activity_monitor,
                    Statistics::Ptr statistics,
                    IConfigurator::Ptr configurator);
  
  BreaksControl(IApp *app,
                IActivityMonitor::Ptr activity_monitor,
                Statistics::Ptr statistics,
                IConfigurator::Ptr configurator);
  virtual ~BreaksControl();

  void init();
  void heartbeat();

  void force_break(BreakId id, BreakHint break_hint);
  void stop_all_breaks();

  IBreak::Ptr get_break(BreakId id);
  Timer::Ptr get_timer(std::string name) const;
  Timer::Ptr get_timer(int id) const;

  void set_operation_mode(OperationMode mode);
  void set_usage_mode(UsageMode mode);
  void set_insist_policy(ICore::InsistPolicy p);

  virtual void resume_reading_mode_timers();
  virtual IActivityMonitor::Ptr create_timer_activity_monitor(const string &break_name);
  virtual void defrost();
  virtual void freeze();
  virtual void set_insensitive_mode(InsensitiveMode mode);
  
private:
  void set_freeze_all_breaks(bool freeze);
  void process_timers();
  void daily_reset();
  void start_break(BreakId break_id, BreakId resume_this_break = BREAK_ID_NONE);
  void save_state() const;
  void load_state();
  
private:
  //! GUI Factory used to create the break/prelude windows.
  IApp *application;

  //!
  IActivityMonitor::Ptr activity_monitor;
  
  //!
  Statistics::Ptr statistics;
  
  //! The Configurator
  IConfigurator::Ptr configurator;

  //! List of breaks.
  Break::Ptr breaks[BREAK_ID_SIZEOF];

  //!
  OperationMode operation_mode;

  //! What to do with activity during insisted break?
  ICore::InsistPolicy insist_policy;

  //! Policy currently in effect.
  ICore::InsistPolicy active_insist_policy;
};

#endif // BREAKSCONTROL_HH
