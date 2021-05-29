// Copyright (C) 2007, 2008, 2009, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <string>

#include "core/CoreConfig.hh"

#include "config/SettingCache.hh"

#include "core/ICore.hh"

using namespace std;
using namespace workrave;
using namespace workrave::config;

IConfigurator::Ptr CoreConfig::config;

const string CoreConfig::CFG_KEY_MICRO_BREAK = "micro_pause";
const string CoreConfig::CFG_KEY_REST_BREAK = "rest_break";
const string CoreConfig::CFG_KEY_DAILY_LIMIT = "daily_limit";
const string CoreConfig::CFG_KEY_TIMERS = "timers";
const string CoreConfig::CFG_KEY_TIMER = "timers/%b";

const string CoreConfig::CFG_KEY_TIMER_LIMIT = "timers/%b/limit";
const string CoreConfig::CFG_KEY_TIMER_AUTO_RESET = "timers/%b/auto_reset";
const string CoreConfig::CFG_KEY_TIMER_RESET_PRED = "timers/%b/reset_pred";
const string CoreConfig::CFG_KEY_TIMER_SNOOZE = "timers/%b/snooze";
const string CoreConfig::CFG_KEY_TIMER_MONITOR = "timers/%b/monitor";
const string CoreConfig::CFG_KEY_TIMER_ACTIVITY_SENSITIVE = "timers/%b/activity_sensitive";
const string CoreConfig::CFG_KEY_TIMER_DAILY_LIMIT_USE_MICRO_BREAK_ACTIVITY = "timers/daily_limit/use_microbreak_activity";

const string CoreConfig::CFG_KEY_BREAKS = "breaks";
const string CoreConfig::CFG_KEY_BREAK = "breaks/%b";

const string CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES = "breaks/%b/max_preludes";
const string CoreConfig::CFG_KEY_BREAK_ENABLED = "breaks/%b/enabled";

const string CoreConfig::CFG_KEY_MONITOR = "monitor";

const string CoreConfig::CFG_KEY_MONITOR_NOISE = "monitor/noise";
const string CoreConfig::CFG_KEY_MONITOR_ACTIVITY = "monitor/activity";
const string CoreConfig::CFG_KEY_MONITOR_IDLE = "monitor/idle";
const string CoreConfig::CFG_KEY_MONITOR_SENSITIVITY = "monitor/sensitivity";

const string CoreConfig::CFG_KEY_GENERAL_DATADIR = "general/datadir";
const string CoreConfig::CFG_KEY_OPERATION_MODE = "general/operation-mode";
const string CoreConfig::CFG_KEY_USAGE_MODE = "general/usage-mode";

const string CoreConfig::CFG_KEY_DISTRIBUTION = "distribution";
const string CoreConfig::CFG_KEY_DISTRIBUTION_ENABLED = "distribution/enabled";
const string CoreConfig::CFG_KEY_DISTRIBUTION_LISTENING = "distribution/listening";
const string CoreConfig::CFG_KEY_DISTRIBUTION_PEERS = "distribution/peers";
const string CoreConfig::CFG_KEY_DISTRIBUTION_TCP = "distribution/tcp";
const string CoreConfig::CFG_KEY_DISTRIBUTION_TCP_PORT = "distribution/port";
const string CoreConfig::CFG_KEY_DISTRIBUTION_TCP_USERNAME = "distribution/username";
const string CoreConfig::CFG_KEY_DISTRIBUTION_TCP_PASSWORD = "distribution/password";
const string CoreConfig::CFG_KEY_DISTRIBUTION_TCP_ATTEMPTS = "distribution/reconnect_attempts";
const string CoreConfig::CFG_KEY_DISTRIBUTION_TCP_INTERVAL = "distribution/reconnect_interval";

bool
CoreConfig::match(const std::string &str, const std::string &key, workrave::BreakId &id)
{
  bool ret = false;

  for (int i = 0; !ret && i < BREAK_ID_SIZEOF; i++)
    {
      if (key % BreakId(i) == str)
        {
          id = BreakId(i);
          ret = true;
        }
    }

  return ret;
}

string
CoreConfig::get_break_name(BreakId id)
{
  const char *names[] = {"micro_pause", "rest_break", "daily_limit"};
  return names[(int)id];
}
string
CoreConfig::expand(const string &key, workrave::BreakId id)
{
  string str = key;
  string::size_type pos = 0;
  string name = CoreConfig::get_break_name(id);

  while ((pos = str.find("%b", pos)) != string::npos)
    {
      str.replace(pos, 2, name);
      pos++;
    }

  return str;
}

SettingGroup &
CoreConfig::key_timer(workrave::BreakId break_id)
{
  return SettingCache::group(config, expand(CFG_KEY_TIMER, break_id));
}

SettingGroup &
CoreConfig::key_break(workrave::BreakId break_id)
{
  return SettingCache::group(config, expand(CFG_KEY_BREAK, break_id));
}

SettingGroup &
CoreConfig::key_timers()
{
  return SettingCache::group(config, CFG_KEY_TIMERS);
}

SettingGroup &
CoreConfig::key_breaks()
{
  return SettingCache::group(config, CFG_KEY_BREAKS);
}

SettingGroup &
CoreConfig::key_monitor()
{
  return SettingCache::group(config, CFG_KEY_MONITOR);
}

Setting<int> &
CoreConfig::timer_limit(workrave::BreakId break_id)
{
  return SettingCache::get<int>(config, expand(CFG_KEY_TIMER_LIMIT, break_id));
}

Setting<int> &
CoreConfig::timer_auto_reset(workrave::BreakId break_id)
{
  return SettingCache::get<int>(config, expand(CFG_KEY_TIMER_AUTO_RESET, break_id));
}

Setting<std::string> &
CoreConfig::timer_reset_pred(workrave::BreakId break_id)
{
  return SettingCache::get<std::string>(config, expand(CFG_KEY_TIMER_RESET_PRED, break_id));
}

Setting<int> &
CoreConfig::timer_snooze(workrave::BreakId break_id)
{
  return SettingCache::get<int>(config, expand(CFG_KEY_TIMER_SNOOZE, break_id));
}

Setting<bool> &
CoreConfig::timer_daily_limit_use_micro_break_activity()
{
  return SettingCache::get<bool>(config, CFG_KEY_TIMER_DAILY_LIMIT_USE_MICRO_BREAK_ACTIVITY);
}

Setting<int> &
CoreConfig::break_max_preludes(workrave::BreakId break_id)
{
  return SettingCache::get<int>(config, expand(CFG_KEY_BREAK_MAX_PRELUDES, break_id));
}

Setting<bool> &
CoreConfig::break_enabled(workrave::BreakId break_id)
{
  return SettingCache::get<bool>(config, expand(CFG_KEY_BREAK_ENABLED, break_id));
}

Setting<int> &
CoreConfig::monitor_noise()
{
  return SettingCache::get<int>(config, CFG_KEY_MONITOR_NOISE, 9000);
}

Setting<int> &
CoreConfig::monitor_activity()
{
  return SettingCache::get<int>(config, CFG_KEY_MONITOR_ACTIVITY, 1000);
}

Setting<int> &
CoreConfig::monitor_idle()
{
  return SettingCache::get<int>(config, CFG_KEY_MONITOR_IDLE, 5000);
}

Setting<int> &
CoreConfig::monitor_sensitivity()
{
  return SettingCache::get<int>(config, CFG_KEY_MONITOR_SENSITIVITY, 3);
}

Setting<std::string> &
CoreConfig::general_datadir()
{
  return SettingCache::get<std::string>(config, CFG_KEY_GENERAL_DATADIR);
}

Setting<int, workrave::OperationMode> &
CoreConfig::operation_mode()
{
  return SettingCache::get<int, workrave::OperationMode>(config, CFG_KEY_OPERATION_MODE);
}

Setting<int, workrave::UsageMode> &
CoreConfig::usage_mode()
{
  return SettingCache::get<int, workrave::UsageMode>(config, CFG_KEY_USAGE_MODE);
}
