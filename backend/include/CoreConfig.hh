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

#include "ICore.hh"

#include "config/IConfigurator.hh"
#include "config/Setting.hh"

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

  static workrave::config::Setting<int> &timer_daily_limit_use_micro_break_activity();

  static workrave::config::Setting<int> &break_max_preludes(workrave::BreakId break_id);
  static workrave::config::Setting<bool> &break_enabled(workrave::BreakId break_id);

  static workrave::config::Setting<int> &monitor_noise();
  static workrave::config::Setting<int> &monitor_activity();
  static workrave::config::Setting<int> &monitor_idle();
  static workrave::config::Setting<std::string> &general_datadir();
  static workrave::config::Setting<int, workrave::OperationMode> &operation_mode();
  static workrave::config::Setting<int, workrave::UsageMode> &usage_mode();

private:
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
  static const std::string CFG_KEY_TIMER_DAILY_LIMIT_USE_MICRO_BREAK_ACTIVITY;

  static const std::string CFG_KEY_BREAKS;
  static const std::string CFG_KEY_BREAK;
  static const std::string CFG_KEY_BREAK_MAX_PRELUDES;
  static const std::string CFG_KEY_BREAK_ENABLED;

  static const std::string CFG_KEY_MONITOR;
  static const std::string CFG_KEY_MONITOR_NOISE;
  static const std::string CFG_KEY_MONITOR_ACTIVITY;
  static const std::string CFG_KEY_MONITOR_IDLE;
  static const std::string CFG_KEY_GENERAL_DATADIR;
  static const std::string CFG_KEY_OPERATION_MODE;
  static const std::string CFG_KEY_USAGE_MODE;

  struct Defaults
  {
    std::string name;
    
    // Timer settings.
    int limit;
    int auto_reset;
    std::string resetpred;
    int snooze;
    
    // Break settings
    int max_preludes;
  };


  static Defaults default_config[];
  static workrave::config::IConfigurator::Ptr config;

  static std::string expand(const std::string &key, workrave::BreakId id);
public:
  static void init(workrave::config::IConfigurator::Ptr config);
  static std::string get_break_name(workrave::BreakId id);
};

#endif
