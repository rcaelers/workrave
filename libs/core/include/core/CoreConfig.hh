// Copyright (C) 2001 - 2009, 2012, 2013 Rob Caelers & Raymond Penners
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

#ifndef WORKRAVE_BACKEND_CORECONFIG_HH
#define WORKRAVE_BACKEND_CORECONFIG_HH

#include <chrono>

#include "config/IConfigurator.hh"
#include "config/Setting.hh"
#include "core/ICore.hh"

class CoreConfig
{
public:
  static workrave::config::SettingGroup &key_timers();
  static workrave::config::SettingGroup &key_breaks();
  static workrave::config::SettingGroup &key_timer(workrave::BreakId break_id);
  static workrave::config::SettingGroup &key_break(workrave::BreakId break_id);
  static workrave::config::SettingGroup &key_monitor();

  static workrave::config::Setting<int> &timer_limit(workrave::BreakId break_id);
  static workrave::config::Setting<int> &timer_auto_reset(workrave::BreakId break_id);
  static workrave::config::Setting<std::string> &timer_reset_pred(workrave::BreakId break_id);
  static workrave::config::Setting<int> &timer_snooze(workrave::BreakId break_id);

  static workrave::config::Setting<bool> &timer_daily_limit_use_micro_break_activity();

  static workrave::config::Setting<int> &break_max_preludes(workrave::BreakId break_id);
  static workrave::config::Setting<bool> &break_enabled(workrave::BreakId break_id);

  static workrave::config::Setting<int> &monitor_noise();
  static workrave::config::Setting<int> &monitor_activity();
  static workrave::config::Setting<int> &monitor_idle();
  static workrave::config::Setting<int> &monitor_sensitivity();
  static workrave::config::Setting<std::string> &general_datadir();
  static workrave::config::Setting<int, workrave::OperationMode> &operation_mode();
  static workrave::config::Setting<int, workrave::UsageMode> &usage_mode();
  static workrave::config::Setting<int, std::chrono::minutes> &operation_mode_auto_reset_duration();
  static workrave::config::Setting<std::vector<int>, std::vector<std::chrono::minutes>> &operation_mode_auto_reset_options();
  static workrave::config::Setting<int64_t, std::chrono::system_clock::time_point> &operation_mode_auto_reset_time();

  // private:
  static const std::string CFG_KEY_TIMER_MONITOR;

  static const std::string CFG_KEY_MICRO_BREAK;
  static const std::string CFG_KEY_REST_BREAK;
  static const std::string CFG_KEY_DAILY_LIMIT;

  static const std::string CFG_KEY_TIMERS;
  static const std::string CFG_KEY_TIMER;

  static const std::string CFG_KEY_TIMER_LIMIT;
  static const std::string CFG_KEY_TIMER_AUTO_RESET;
  static const std::string CFG_KEY_TIMER_RESET_PRED;
  static const std::string CFG_KEY_TIMER_SNOOZE;
  static const std::string CFG_KEY_TIMER_ACTIVITY_SENSITIVE;
  static const std::string CFG_KEY_TIMER_DAILY_LIMIT_USE_MICRO_BREAK_ACTIVITY;

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
  static const std::string CFG_KEY_OPERATION_MODE_RESET_DURATION;
  static const std::string CFG_KEY_OPERATION_MODE_RESET_OPTIONS;
  static const std::string CFG_KEY_OPERATION_MODE_RESET_TIME;
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

private:
  static workrave::config::IConfigurator::Ptr config;
  static std::string expand(const std::string &key, workrave::BreakId id);

public:
  static bool match(const std::string &str, const std::string &key, workrave::BreakId &id);
  static void init(workrave::config::IConfigurator::Ptr config);
  static std::string get_break_name(workrave::BreakId id);
};

#endif
