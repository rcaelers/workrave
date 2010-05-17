// CoreConfig.cc --- The WorkRave Core Configuration
//
// Copyright (C) 2007, 2008, 2009 Rob Caelers & Raymond Penners
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
const string CoreConfig::CFG_KEY_OPERATION_MODE            = "gui/operation-mode";
const string CoreConfig::CFG_KEY_USAGE_MODE                = "general/usage-mode";

const string CoreConfig::CFG_KEY_DISTRIBUTION              = "distribution";
const string CoreConfig::CFG_KEY_DISTRIBUTION_ENABLED      = "distribution/enabled";
const string CoreConfig::CFG_KEY_DISTRIBUTION_LISTENING    = "distribution/listening";
const string CoreConfig::CFG_KEY_DISTRIBUTION_PEERS        = "distribution/peers";
const string CoreConfig::CFG_KEY_DISTRIBUTION_TCP          = "distribution/tcp";
const string CoreConfig::CFG_KEY_DISTRIBUTION_TCP_PORT     = "distribution/port";
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
