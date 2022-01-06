// Copyright (C) 2001 - 2013 Ray Satiro & Rob Caelers & Raymond Penners
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

#ifndef COREMODES_HH
#define COREMODES_HH

#include <string>
#include <map>
#include <memory>
#include <chrono>
#include <optional>
#include <boost/signals2.hpp>

#include "IActivityMonitor.hh"

#include "core/CoreTypes.hh"
#include "utils/Signals.hh"

class CoreModes : public workrave::utils::Trackable
{
public:
  using Ptr = std::shared_ptr<CoreModes>;

  explicit CoreModes(IActivityMonitor::Ptr monitor);
  virtual ~CoreModes();

  boost::signals2::signal<void(workrave::OperationMode)> &signal_operation_mode_changed();
  boost::signals2::signal<void(workrave::UsageMode)> &signal_usage_mode_changed();

  workrave::OperationMode get_active_operation_mode();
  workrave::OperationMode get_regular_operation_mode();
  bool is_operation_mode_an_override();
  void set_operation_mode(workrave::OperationMode mode);
  void set_operation_mode_for(workrave::OperationMode mode, std::chrono::minutes duration);
  void set_operation_mode_override(workrave::OperationMode mode, const std::string &id);
  void remove_operation_mode_override(const std::string &id);
  workrave::UsageMode get_usage_mode();
  void set_usage_mode(workrave::UsageMode mode);
  void heartbeat();
  void daily_reset();

private:
  void set_operation_mode_internal(workrave::OperationMode mode);
  void update_active_operation_mode();
  void set_usage_mode_internal(workrave::UsageMode mode, bool persistent);
  void load_config();
  void check_auto_reset();

private:
  //! Current operation mode.
  workrave::OperationMode operation_mode_active;

  //! The same as operation_mode unless operation_mode is an override mode.
  workrave::OperationMode operation_mode_regular;

  //! Active operation mode overrides.
  std::map<std::string, workrave::OperationMode> operation_mode_overrides;

  //! Current usage mode.
  workrave::UsageMode usage_mode;

  //!
  IActivityMonitor::Ptr monitor;

  //! Operation mode changed notification.
  boost::signals2::signal<void(workrave::OperationMode)> operation_mode_changed_signal;

  //! Usage mode changed notification.
  boost::signals2::signal<void(workrave::UsageMode)> usage_mode_changed_signal;
};

#endif // COREMODES_HH
