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

#include <vector>
#include <boost/enable_shared_from_this.hpp>

#include "config/Config.hh"
#include "dbus/IDBus.hh"

#include "ActivityMonitor.hh"
#include "Statistics.hh"
#include "Break.hh"
#include "IBreakSupport.hh"
#include "ReadingActivityMonitor.hh"

using namespace workrave;
using namespace workrave::config;
using namespace workrave::dbus;

class BreaksControl :
  public IBreakSupport,
  public boost::enable_shared_from_this<BreaksControl>
{
public:
  typedef boost::shared_ptr<BreaksControl> Ptr;

  static Ptr create(IApp *app,
                    IActivityMonitor::Ptr activity_monitor,
                    Statistics::Ptr statistics,
                    IConfigurator::Ptr configurator,
                    IDBus::Ptr dbus);
  
  BreaksControl(IApp *app,
                IActivityMonitor::Ptr activity_monitor,
                Statistics::Ptr statistics,
                IConfigurator::Ptr configurator,
                IDBus::Ptr dbus);
  virtual ~BreaksControl();

  void init();
  void heartbeat();
  void save_state() const;

  void force_break(BreakId id, BreakHint break_hint);
  void stop_all_breaks();

  IBreak::Ptr get_break(BreakId id);

  void set_operation_mode(OperationMode mode);
  void set_usage_mode(UsageMode mode);
  void set_insist_policy(ICore::InsistPolicy p);

  virtual IActivityMonitor::Ptr create_timer_activity_monitor(const string &break_name);
  
private:
  void set_freeze_all_breaks(bool freeze);
  void process_timers();
  void daily_reset();
  void start_break(BreakId break_id, BreakId resume_this_break = BREAK_ID_NONE);
  void load_state();
  void defrost();
  void freeze();

  void on_break_event(BreakId break_id, IBreak::BreakEvent event);
  
  Timer::Ptr get_timer(std::string name) const;
  Timer::Ptr get_timer(int id) const;
  
private:
  //! GUI Factory used to create the break/prelude windows.
  IApp *application;

  //!
  IActivityMonitor::Ptr activity_monitor;
  
  //!
  Statistics::Ptr statistics;
  
  //! The Configurator
  IConfigurator::Ptr configurator;

  //! DBUs
  IDBus::Ptr dbus;
  
  //! List of breaks.
  Break::Ptr breaks[BREAK_ID_SIZEOF];

  //!
  ReadingActivityMonitor::Ptr reading_activity_monitor;
  
  //!
  OperationMode operation_mode;

  UsageMode usage_mode;
  
  //! What to do with activity during insisted break?
  ICore::InsistPolicy insist_policy;

  //! Policy currently in effect.
  ICore::InsistPolicy active_insist_policy;
};

#endif // BREAKSCONTROL_HH
