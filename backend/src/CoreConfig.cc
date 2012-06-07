// CoreConfig.cc --- The WorkRave Core Configuration
//
// Copyright (C) 2007, 2008, 2009, 2011, 2012 Rob Caelers & Raymond Penners
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
#include "config.h"
#endif

#include <string>

#include "CoreConfig.hh"
#include "ICore.hh"

using namespace std;
using namespace workrave;
using namespace workrave::config;

const string CoreConfig::CFG_KEY_MICRO_BREAK               = "micro_pause";
const string CoreConfig::CFG_KEY_REST_BREAK                = "rest_break";
const string CoreConfig::CFG_KEY_DAILY_LIMIT               = "daily_limit";

const string CoreConfig::CFG_KEY_TIMERS                    = "timers";
const string CoreConfig::CFG_KEY_TIMER                     = "timers/%b";

const string CoreConfig::CFG_KEY_TIMER_LIMIT               = "timers/%b/limit";
const string CoreConfig::CFG_KEY_TIMER_AUTO_RESET          = "timers/%b/auto_reset";
const string CoreConfig::CFG_KEY_TIMER_RESET_PRED          = "timers/%b/reset_pred";
const string CoreConfig::CFG_KEY_TIMER_SNOOZE              = "timers/%b/snooze";
const string CoreConfig::CFG_KEY_TIMER_MONITOR             = "timers/%b/monitor";
const string CoreConfig::CFG_KEY_TIMER_ACTIVITY_SENSITIVE  = "timers/%b/activity_sensitive";

const string CoreConfig::CFG_KEY_BREAKS                    = "breaks";
const string CoreConfig::CFG_KEY_BREAK                     = "breaks/%b";

const string CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES        = "breaks/%b/max_preludes";
const string CoreConfig::CFG_KEY_BREAK_ENABLED             = "breaks/%b/enabled";

const string CoreConfig::CFG_KEY_MONITOR                   = "monitor";

const string CoreConfig::CFG_KEY_MONITOR_NOISE             = "monitor/noise";
const string CoreConfig::CFG_KEY_MONITOR_ACTIVITY          = "monitor/activity";
const string CoreConfig::CFG_KEY_MONITOR_IDLE              = "monitor/idle";

const string CoreConfig::CFG_KEY_GENERAL_DATADIR           = "general/datadir";
const string CoreConfig::CFG_KEY_OPERATION_MODE            = "general/operation-mode";
const string CoreConfig::CFG_KEY_USAGE_MODE                = "general/usage-mode";


struct Defaults
{
  string name;

  // Timer settings.
  int limit;
  int auto_reset;
  string resetpred;
  int snooze;

  // Break settings
  int max_preludes;

} default_config[] =
  {
    {
      CoreConfig::CFG_KEY_MICRO_BREAK,
      3*60, 30, "", 150,
      3,
    },

    {
      CoreConfig::CFG_KEY_REST_BREAK,
      45*60, 10*60, "", 180,
      3,
    },

    {
      CoreConfig::CFG_KEY_DAILY_LIMIT,
      14400, 0, "day/4:00", 20 * 60,
      3,
    }
  };


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

//! Returns the name of the break (used in configuration)
string
CoreConfig::get_break_name(BreakId id)
{
  Defaults &def = default_config[id];
  return def.name;
}


void
CoreConfig::init(IConfigurator::Ptr config)
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakId break_id = (BreakId) i;
      Defaults &def = default_config[i];

      config->set_delay(CoreConfig::CFG_KEY_TIMER_LIMIT % break_id, 2);
      config->set_delay(CoreConfig::CFG_KEY_TIMER_AUTO_RESET % break_id, 2);

      // Convert old settings.

      config->rename_key(string("gui/breaks/%b/max_preludes") % break_id,
                         CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % break_id);

      config->rename_key(string("gui/breaks/%b/enabled") % break_id,
                         CoreConfig::CFG_KEY_BREAK_ENABLED % break_id);

      config->remove_key(string("gui/breaks/%b/max_postpone") % break_id);

      // Set defaults.

      config->set_value(CoreConfig::CFG_KEY_TIMER_LIMIT % break_id,
                        def.limit,
                        CONFIG_FLAG_DEFAULT);

      config->set_value(CoreConfig::CFG_KEY_TIMER_AUTO_RESET % break_id,
                        def.auto_reset,
                        CONFIG_FLAG_DEFAULT);

      config->set_value(CoreConfig::CFG_KEY_TIMER_RESET_PRED % break_id,
                        def.resetpred,
                        CONFIG_FLAG_DEFAULT);

      config->set_value(CoreConfig::CFG_KEY_TIMER_SNOOZE % break_id,
                        def.snooze,
                        CONFIG_FLAG_DEFAULT);

      config->set_value(CoreConfig::CFG_KEY_TIMER_MONITOR % break_id,
                        "",
                        CONFIG_FLAG_DEFAULT);

      config->set_value(CoreConfig::CFG_KEY_TIMER_ACTIVITY_SENSITIVE % break_id,
                        true,
                        CONFIG_FLAG_DEFAULT);

      config->set_value(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % break_id,
                        def.max_preludes,
                        CONFIG_FLAG_DEFAULT);

      config->set_value(CoreConfig::CFG_KEY_BREAK_ENABLED % break_id,
                        true,
                        CONFIG_FLAG_DEFAULT);
    }
}


bool
CoreConfig::starts_with(const string &key, string prefix, string &name)
{
  bool ret = false;

  // Search prefix (just in case some Configurator added a leading /)
  string::size_type pos = key.rfind(prefix);
  string k;

  if (pos != string::npos)
    {
      k = key.substr(pos + prefix.length());
      pos = k.find('/');

      if (pos != string::npos)
        {
          name = k.substr(0, pos);
        }
      ret = true;
    }
  return ret;
}

namespace workrave
{
  std::string operator%(const string &key, BreakId id)
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
}
