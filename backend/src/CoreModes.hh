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
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

#include "utils/ScopedConnections.hh"

#include "ActivityMonitor.hh"

#include "CoreTypes.hh"

using namespace workrave;

class CoreModes
{
public:
  typedef boost::shared_ptr<CoreModes> Ptr;

  static CoreModes::Ptr create(ActivityMonitor::Ptr monitor);
  
  CoreModes(ActivityMonitor::Ptr monitor);
  virtual ~CoreModes();
  
  boost::signals2::signal<void(workrave::OperationMode)> &signal_operation_mode_changed();
  boost::signals2::signal<void(workrave::UsageMode)> &signal_usage_mode_changed();

  OperationMode get_operation_mode();
  OperationMode get_operation_mode_regular();
  bool is_operation_mode_an_override();
  void set_operation_mode(OperationMode mode);
  void set_operation_mode_override(OperationMode mode, const std::string &id);
  void remove_operation_mode_override(const std::string &id);
  UsageMode get_usage_mode();
  void set_usage_mode(UsageMode mode);
  
private:
  void set_operation_mode_internal(OperationMode mode, bool persistent, const std::string &override_id = "");
  void set_usage_mode_internal(UsageMode mode, bool persistent);
  void load_config();
  
private:
  //! Current operation mode.
  OperationMode operation_mode;

  //! The same as operation_mode unless operation_mode is an override mode.
  OperationMode operation_mode_regular;

  //! Active operation mode overrides.
  std::map<std::string, OperationMode> operation_mode_overrides;

  //! Current usage mode.
  UsageMode usage_mode;

  //!
  ActivityMonitor::Ptr monitor;
  
  //! Operation mode changed notification.
  boost::signals2::signal<void(OperationMode)> operation_mode_changed_signal;

  //! Usage mode changed notification.
  boost::signals2::signal<void(UsageMode)> usage_mode_changed_signal;

  scoped_connections connections;
};

#endif // COREMODES_HH
