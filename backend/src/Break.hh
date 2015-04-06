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

#include "config/IConfigurator.hh"
#include "dbus/IDBus.hh"

#include "IBreak.hh"
#include "Timer.hh"
#include "Statistics.hh"
#include "IActivityMonitor.hh"

#include "BreakStateModel.hh"
#include "BreakStatistics.hh"
#include "BreakConfig.hh"
#include "BreakDBus.hh"

class Break : public workrave::IBreak
{
public:
  typedef boost::shared_ptr<Break> Ptr;

public:
  static Ptr create(workrave::BreakId id,
                    workrave::IApp *app,
                    Timer::Ptr timer,
                    IActivityMonitor::Ptr activity_monitor,
                    Statistics::Ptr statistics,
                    workrave::dbus::IDBus::Ptr dbus,
                    CoreHooks::Ptr hooks);

  Break(workrave::BreakId id,
        workrave::IApp *app,
        Timer::Ptr timer,
        IActivityMonitor::Ptr activity_monitor,
        Statistics::Ptr statistics,
        workrave::dbus::IDBus::Ptr dbus,
        CoreHooks::Ptr hooks);

  // IBreak
  virtual boost::signals2::signal<void(workrave::BreakEvent)> &signal_break_event();
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

  void process();
  void start_break();
  void stop_break();
  void force_start_break(workrave::BreakHint break_hint);
  void override(workrave::BreakId id);
  void daily_reset();
  bool is_microbreak_used_for_activity() const;

  // TODO: add custom string<->enum converters to dbus binding.
  std::string get_break_stage() const;
  static std::string get_stage_text(BreakStage stage);

private:
  workrave::BreakId break_id;
  Timer::Ptr timer;
  BreakStateModel::Ptr break_state_model;
  BreakStatistics::Ptr break_statistics;
  BreakConfig::Ptr break_configuration;
  BreakDBus::Ptr break_dbus;
};

#endif // BREAK_HH
