// Networking.hh --- Networking network server
//
// Copyright (C) 2007, 2008, 2009, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef NETWORKING_HH
#define NETWORKING_HH

#include <list>
#include <map>
#include <string>

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "config/Config.hh"
#include "cloud/Cloud.hh"

#include "ICore.hh"

#include "NetworkConfigurationManager.hh"
#include "NetworkActivityMonitor.hh"

#include "workrave.pb.h"

using namespace workrave;
using namespace workrave::config;

class Networking
{
public:
  typedef boost::shared_ptr<Networking> Ptr;
  
public:
  static Ptr create(ICore::Ptr core);

public:  
  Networking(ICore::Ptr core);
  virtual ~Networking();

  void init();
  void terminate();
  void heartbeat();
  void connect(const std::string host, int port);

#ifdef HAVE_TESTS
  void start_announce();
  ICloud::Ptr get_cloud() const { return cloud; }
#endif
  
private:
  void on_break_message(Message::Ptr message, MessageContext::Ptr context);
  void on_operation_mode_message(Message::Ptr message, MessageContext::Ptr context);
  void on_usage_mode_message(Message::Ptr message, MessageContext::Ptr context);
  void on_timer_message(Message::Ptr message, MessageContext::Ptr context);

  void send_timer_state();

  void send_break_event(BreakId id, workrave::networking::Break::BreakEvent event);
  
  void on_break_postponed(BreakId id);
  void on_break_skipped(BreakId id);
  void on_break_forced(BreakId id, BreakHint hint);
  void on_operation_mode_changed(OperationMode mode);
  void on_usage_mode_changed(UsageMode mode);

  boost::signals2::connection operation_mode_connection;
  boost::signals2::connection usage_mode_connection;
  boost::signals2::connection break_postponed_connection[BREAK_ID_SIZEOF];
  boost::signals2::connection break_skipped_connection[BREAK_ID_SIZEOF];
  boost::signals2::connection break_forced_connection[BREAK_ID_SIZEOF];
  
private:
  //! 
  ICloud::Ptr cloud;

  //!
  ICore::Ptr core;

  //!
  IConfigurator::Ptr configurator;
  
  //! 
  NetworkConfigurationManager::Ptr configuration_manager;

  //!
  NetworkActivityMonitor::Ptr activity_monitor;
};


#endif // NETWORKING_HH
