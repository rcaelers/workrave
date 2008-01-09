// Break.cc
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include "Break.hh"

#include "IConfigurator.hh"
#include "ICore.hh"
#include "CoreFactory.hh"

#include "BreakControl.hh"
#include "Timer.hh"
#include "TimerActivityMonitor.hh"

using namespace std;

const string Break::CFG_KEY_TIMER_PREFIX = "timers/";

const string Break::CFG_KEY_TIMER_LIMIT = "/limit";
const string Break::CFG_KEY_TIMER_AUTO_RESET = "/auto_reset";
const string Break::CFG_KEY_TIMER_RESET_PRED = "/reset_pred";
const string Break::CFG_KEY_TIMER_SNOOZE = "/snooze";
const string Break::CFG_KEY_TIMER_MONITOR = "/monitor";
const string Break::CFG_KEY_TIMER_ACTIVITY_SENSITIVE = "/activity_sensitive";

const string Break::CFG_KEY_BREAK_PREFIX = "gui/breaks/";

const string Break::CFG_KEY_BREAK_MAX_PRELUDES = "/max_preludes";
const string Break::CFG_KEY_BREAK_ENABLED = "/enabled";


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
      // FIXME: Rename to micro_break, but in a backwards compatible manner.
      "micro_pause",
      3*60, 30, "", 150,
      3,
    },

    {
      "rest_break",
      45*60, 10*60, "", 180,
      3,
    },

    {
      "daily_limit",
      14400, 0, "day/4:00", 20 * 60,
      3,
    }
  };



//! Constucts a new Break
Break::Break() :
  break_id(BREAK_ID_NONE),
  config(NULL),
  application(NULL),
  timer(NULL),
  break_control(NULL),
  enabled(true)
{
  TRACE_ENTER("Break:Break");
  TRACE_EXIT()
}


//! Initializes the break.
void
Break::init(BreakId id, IApp *app)
{
  TRACE_ENTER("Break::init");

  break_id = id;
  config = CoreFactory::get_configurator();
  application = app;

  Defaults &def = default_config[break_id];

  break_name = def.name;
  timer = new Timer();
  timer->set_id(break_name);
  break_control = new BreakControl(break_id, app, timer);

  init_timer();
  init_break_control();
  init_defaults();

  TRACE_EXIT()
}

//! Destructor.
Break::~Break()
{
  TRACE_ENTER("Break:~Break");
  delete break_control;
  delete timer;
  TRACE_EXIT();
}


string
Break::expand(const string &key, BreakId id)
{
  string str = key;
  string::size_type pos = 0;
  string name = get_name(id);

  while ((pos = str.find("%b", pos)) != string::npos)
    {
      str.replace(pos, 2, name);
      pos++;
    }

  return str;
}


string
Break::expand(const string &key)
{
  string str = key;
  string::size_type pos = 0;
  string name = get_name();

  while ((pos = str.find("%b", pos)) != string::npos)
    {
      str.replace(pos, 2, name);
      pos++;
    }

  return str;
}


void
Break::init_defaults()
{
  Defaults &def = default_config[break_id];

  config->set_delay(timer_prefix + CFG_KEY_TIMER_LIMIT, 2);
  config->set_delay(timer_prefix + CFG_KEY_TIMER_AUTO_RESET, 2);

  // Convert old settings.

  config->rename_key(expand("gui/breaks/%b/max_preludes"),
                     expand("breaks/%b/max_preludes"));

  config->rename_key(expand("gui/breaks/%b/enabled"),
                     expand("breaks/%b/enabled"));

  config->rename_key(expand("gui/breaks/%b/enabled"),
                     expand("breaks/%b/enabled"));

  config->remove_key(expand("gui/breaks/%b/max_postpone"));

  // Set defaults.

  config->set_value(expand("timers/%b/limit"), def.limit,
                    CONFIG_FLAG_DEFAULT);
  config->set_value(expand("timers/%b/auto_reset"), def.auto_reset,
                    CONFIG_FLAG_DEFAULT);
  config->set_value(expand("timers/%b/reset_pred"), def.resetpred,
                    CONFIG_FLAG_DEFAULT);
  config->set_value(expand("timers/%b/snooze"), def.snooze,
                    CONFIG_FLAG_DEFAULT);
  config->set_value(expand("timers/%b/monitor"), "",
                    CONFIG_FLAG_DEFAULT);
  config->set_value(expand("timers/%b/activity_sensitive"), true,
                    CONFIG_FLAG_DEFAULT);
  config->set_value(expand("breaks/%b/max_preludes"), def.max_preludes,
                    CONFIG_FLAG_DEFAULT);
  config->set_value(expand("breaks/%b/enabled"), true,
                    CONFIG_FLAG_DEFAULT);

}

//! Returns the id of the break
BreakId
Break::get_id() const
{
  return break_id;
}


//! Returns the name of the break (used in configuration)
string
Break::get_name() const
{
  return break_name;
}


//! Returns the name of the break (used in configuration)
string
Break::get_name(BreakId id)
{
  Defaults &def = default_config[id];
  return def.name;
}


//! Returns the timer.
Timer *
Break::get_timer() const
{
  return timer;
}


//! Returns the Break controller.
BreakControl *
Break::get_break_control()
{
  return break_control;
}


// Initialize the timer based.
void
Break::init_timer()
{
  timer_prefix = CFG_KEY_TIMER_PREFIX + break_name;

  load_timer_config();

  timer->enable();
  timer->stop_timer();

  config->add_listener(CFG_KEY_TIMER_PREFIX + break_name, this);
}


//! Load the configuration of the timer.
void
Break::load_timer_config()
{
  // Read break limit.
  int limit;
  config->get_value(timer_prefix + CFG_KEY_TIMER_LIMIT, limit);
  timer->set_limit(limit);
  timer->set_limit_enabled(limit > 0);

  // Read autoreset interval
  int autoreset;
  config->get_value(timer_prefix + CFG_KEY_TIMER_AUTO_RESET, autoreset);
  timer->set_auto_reset(autoreset);
  timer->set_auto_reset_enabled(autoreset > 0);

  // Read reset predicate
  string reset_pred;
  config->get_value(timer_prefix + CFG_KEY_TIMER_RESET_PRED, reset_pred);
  if (reset_pred != "")
    {
      timer->set_auto_reset(reset_pred);
    }

  // Read the snooze time.
  int snooze;
  config->get_value(timer_prefix + CFG_KEY_TIMER_SNOOZE, snooze);
  timer->set_snooze_interval(snooze);

  // Read the activity insensitive flag
  bool sensitive;
  config->get_value(timer_prefix + CFG_KEY_TIMER_ACTIVITY_SENSITIVE, sensitive);
  timer->set_activity_sensitive(sensitive);

  // Load the monitor setting for the timer.
  string monitor_name;

  bool ret = config->get_value(timer_prefix + CFG_KEY_TIMER_MONITOR, monitor_name);

  if (ret && monitor_name != "")
    {
      Core *core = Core::get_instance();
      Timer *master = core->get_timer(monitor_name);
      if (master != NULL)
        {
          TimerActivityMonitor *am = new TimerActivityMonitor(master);
          timer->set_activity_monitor(am);
        }
    }
  else
    {
      timer->set_activity_monitor(NULL);
    }
}


// Initialize the break control.
void
Break::init_break_control()
{
  break_prefix = CFG_KEY_BREAK_PREFIX + break_name;

  load_break_control_config();
  config->add_listener(CFG_KEY_BREAK_PREFIX + break_name, this);
}


void
Break::load_break_control_config()
{
  // Maximum number of prelude windows.
  int max_preludes;
  config->get_value(break_prefix + CFG_KEY_BREAK_MAX_PRELUDES, max_preludes);
  break_control->set_max_preludes(max_preludes);

  // Break enabled?
  enabled = true;
  config->get_value(break_prefix + CFG_KEY_BREAK_ENABLED, enabled);
}


bool
Break::get_timer_activity_sensitive() const
{
  bool sensitive;
  config->get_value(timer_prefix + CFG_KEY_TIMER_ACTIVITY_SENSITIVE, sensitive);

  return sensitive;
}

bool
Break::is_enabled() const
{
  return enabled;
}

bool
Break::is_running() const
{
  TimerState state = timer->get_state();
  return state == STATE_RUNNING;
}

time_t
Break::get_elapsed_time() const
{
  return timer->get_elapsed_time();
}

time_t
Break::get_elapsed_idle_time() const
{
  return timer->get_elapsed_idle_time();
}

time_t
Break::get_auto_reset() const
{
  return timer->get_auto_reset();
}

bool
Break::is_auto_reset_enabled() const
{
  return timer->is_auto_reset_enabled();
}

time_t
Break::get_limit() const
{
  return timer->get_limit();
}

bool
Break::is_limit_enabled() const
{
  return timer->is_limit_enabled();
}

bool
Break::starts_with(const string &key, string prefix, string &name)
{
  TRACE_ENTER_MSG("Break::starts_with", key << " " << prefix);
  bool ret = false;

  // Search prefix (just in case some Configurator added a leading /)
  string::size_type pos = key.rfind(prefix);
  string k;

  if (pos != string::npos)
    {
      TRACE_MSG(pos);
      k = key.substr(pos + prefix.length());
      pos = k.find('/');

      if (pos != string::npos)
        {
          name = k.substr(0, pos);
        }
      ret = true;
    }

  TRACE_EXIT();
  return ret;
}


//! Notification that the configuration changed.
void
Break::config_changed_notify(const string &key)
{
  TRACE_ENTER_MSG("Break::config_changed_notify", key);
  string name;

  if (starts_with(key, CFG_KEY_BREAK_PREFIX, name))
    {
      TRACE_MSG("break: " << name);
      load_break_control_config();
    }
  else if (starts_with(key, CFG_KEY_TIMER_PREFIX, name))
    {
      TRACE_MSG("timer: " << name);
      load_timer_config();
    }
  TRACE_EXIT();
}
