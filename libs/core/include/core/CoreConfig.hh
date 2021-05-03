// CoreConfig.hh --- Configuration keys of the core.
//
// Copyright (C) 2001 - 2009 Rob Caelers & Raymond Penners
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

#ifndef CORECONFIG_HH
#define CORECONFIG_HH

#include "ICore.hh"

class CoreConfig
{
public:
  static const std::string CFG_KEY_TIMERS;
  static const std::string CFG_KEY_TIMER;

  static const std::string CFG_KEY_TIMER_LIMIT;
  static const std::string CFG_KEY_TIMER_AUTO_RESET;
  static const std::string CFG_KEY_TIMER_RESET_PRED;
  static const std::string CFG_KEY_TIMER_SNOOZE;
  static const std::string CFG_KEY_TIMER_MONITOR;
  static const std::string CFG_KEY_TIMER_ACTIVITY_SENSITIVE;

  static const std::string CFG_KEY_BREAKS;
  static const std::string CFG_KEY_BREAK;
  static const std::string CFG_KEY_BREAK_MAX_PRELUDES;
  static const std::string CFG_KEY_BREAK_ENABLED;

  static const std::string CFG_KEY_MONITOR;
  static const std::string CFG_KEY_MONITOR_NOISE;
  static const std::string CFG_KEY_MONITOR_ACTIVITY;
  static const std::string CFG_KEY_MONITOR_IDLE;
  static const std::string CFG_KEY_MONITOR_SENSITIVITY;
  static const std::string CFG_KEY_GENERAL_DATADIR;
  static const std::string CFG_KEY_OPERATION_MODE;
  static const std::string CFG_KEY_USAGE_MODE;

  static const std::string CFG_KEY_DISTRIBUTION;
  static const std::string CFG_KEY_DISTRIBUTION_ENABLED;
  static const std::string CFG_KEY_DISTRIBUTION_LISTENING;
  static const std::string CFG_KEY_DISTRIBUTION_PEERS;
  static const std::string CFG_KEY_DISTRIBUTION_TCP;
  static const std::string CFG_KEY_DISTRIBUTION_TCP_PORT;
  static const std::string CFG_KEY_DISTRIBUTION_TCP_USERNAME;
  static const std::string CFG_KEY_DISTRIBUTION_TCP_PASSWORD;
  static const std::string CFG_KEY_DISTRIBUTION_TCP_ATTEMPTS;
  static const std::string CFG_KEY_DISTRIBUTION_TCP_INTERVAL;

  static bool match(const std::string &str, const std::string &key, workrave::BreakId &id);
  static std::string get_break_name(workrave::BreakId id);
};

#endif
