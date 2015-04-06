// Core.hh --- The main controller
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

#ifndef CORE_HH
#define CORE_HH

#include <string>
#include <map>

#include "dbus/IDBus.hh"
#include "config/IConfigurator.hh"

#include "ICore.hh"
#include "LocalActivityMonitor.hh"
#include "BreaksControl.hh"
#include "Statistics.hh"
#include "CoreHooks.hh"
#include "CoreModes.hh"
#include "CoreDBus.hh"

// Forward declarion of external interface.
namespace workrave {
  class IApp;
}

class Core : public workrave::ICore
{
public:
  Core();
  virtual ~Core();

  // ICore
  virtual boost::signals2::signal<void(workrave::OperationMode)> &signal_operation_mode_changed();
  virtual boost::signals2::signal<void(workrave::UsageMode)> &signal_usage_mode_changed();
  virtual void init(workrave::IApp *application, const std::string &display_name);
  virtual void heartbeat();
  virtual void force_break(workrave::BreakId id, workrave::BreakHint break_hint);
  virtual workrave::IBreak::Ptr get_break(workrave::BreakId id);
  virtual workrave::IStatistics::Ptr get_statistics() const;
  virtual workrave::config::IConfigurator::Ptr get_configurator() const;
  virtual ICoreHooks::Ptr get_hooks() const;
  virtual workrave::dbus::IDBus::Ptr get_dbus() const;
  virtual bool is_user_active() const;
  virtual workrave::OperationMode get_operation_mode();
  virtual workrave::OperationMode get_operation_mode_regular();
  virtual bool is_operation_mode_an_override();
  virtual void set_operation_mode(workrave::OperationMode mode);
  virtual void set_operation_mode_override(workrave::OperationMode mode, const std::string &id);
  virtual void remove_operation_mode_override(const std::string &id);
  virtual workrave::UsageMode get_usage_mode();
  virtual void set_usage_mode(workrave::UsageMode mode);
  virtual void set_powersave(bool down);
  virtual void set_insist_policy(workrave::InsistPolicy p);
  virtual void force_idle();

  // DBus functions.
  void report_external_activity(std::string who, bool act);

private:
  void init_configurator();
  void init_bus();

private:
  //! List of breaks.
  BreaksControl::Ptr breaks_control;

  //! The Configurator.
  workrave::config::IConfigurator::Ptr configurator;

  //! The activity monitor
  LocalActivityMonitor::Ptr monitor;

  //! Hooks to alter the backend behaviour.
  CoreHooks::Ptr hooks;

  //!
  CoreModes::Ptr core_modes;

  //!
  CoreDBus::Ptr core_dbus;

  //! GUI Widget factory.
  workrave::IApp *application;

  //! The statistics collector.
  Statistics::Ptr statistics;

  //! Did the OS announce a powersave?
  bool powersave;

  //! DBUS bridge
  workrave::dbus::IDBus::Ptr dbus;
};

#endif // CORE_HH
