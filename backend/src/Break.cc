// Break.cc
//
// Copyright (C) 2001 - 2011 Rob Caelers & Raymond Penners
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

#include "debug.hh"

#include "Break.hh"

#include "IConfigurator.hh"
#include "ICore.hh"
#include "CoreFactory.hh"

#include "BreakControl.hh"
#include "Timer.hh"
#include "TimerActivityMonitor.hh"

#include "CoreConfig.hh"

using namespace std;


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
  enabled(true),
  usage_mode(USAGE_MODE_NORMAL)
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
  load_timer_config();

  timer->enable();
  timer->stop_timer();

  config->add_listener(CoreConfig::CFG_KEY_TIMER % break_id, this);
}


//! Load the configuration of the timer.
void
Break::load_timer_config()
{
  TRACE_ENTER("Break::load_timer_config");
  // Read break limit.
  int limit;
  config->get_value(CoreConfig::CFG_KEY_TIMER_LIMIT % break_id, limit);
  timer->set_limit(limit);
  timer->set_limit_enabled(limit > 0);

  // Read autoreset interval
  int autoreset;
  config->get_value(CoreConfig::CFG_KEY_TIMER_AUTO_RESET % break_id, autoreset);
  timer->set_auto_reset(autoreset);
  timer->set_auto_reset_enabled(autoreset > 0);

  // Read reset predicate
  string reset_pred;
  config->get_value(CoreConfig::CFG_KEY_TIMER_RESET_PRED % break_id, reset_pred);
  if (reset_pred != "")
    {
      timer->set_auto_reset(reset_pred);
    }

  // Read the snooze time.
  int snooze;
  config->get_value(CoreConfig::CFG_KEY_TIMER_SNOOZE % break_id, snooze);
  timer->set_snooze_interval(snooze);

  // Load the monitor setting for the timer.
  string monitor_name;

  bool ret = config->get_value(CoreConfig::CFG_KEY_TIMER_MONITOR % break_id, monitor_name);

  TRACE_MSG(ret << " " << monitor_name);
  if (ret && monitor_name != "")
    {
      Core *core = Core::get_instance();
      Timer *master = core->get_timer(monitor_name);
      if (master != NULL)
        {
          TRACE_MSG("found master timer");
          TimerActivityMonitor *am = new TimerActivityMonitor(master);
          timer->set_activity_monitor(am);
        }
    }
  else
    {
      timer->set_activity_monitor(NULL);
    }
  TRACE_EXIT();
}


// Initialize the break control.
void
Break::init_break_control()
{
  load_break_control_config();
  config->add_listener(CoreConfig::CFG_KEY_BREAK % break_id, this);
}


void
Break::load_break_control_config()
{
  // Maximum number of prelude windows.
  int max_preludes;
  config->get_value(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % break_id, max_preludes);
  break_control->set_max_preludes(max_preludes);

  // Break enabled?
  enabled = true;
  config->get_value(CoreConfig::CFG_KEY_BREAK_ENABLED % break_id, enabled);
}


void
Break::override(BreakId id)
{
  int max_preludes;
  config->get_value(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % break_id, max_preludes);

  if (break_id != id)
    {
      int override_max_preludes;
      config->get_value(CoreConfig::CFG_KEY_BREAK_MAX_PRELUDES % id, override_max_preludes);
      if (override_max_preludes != -1 && override_max_preludes < max_preludes)
        {
          max_preludes = override_max_preludes;
        }
    }

  break_control->set_max_preludes(max_preludes);
}

bool
Break::get_timer_activity_sensitive() const
{
  return timer->get_activity_sensitive();
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


bool
Break::is_taking() const
{
  return break_control->is_taking();
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

void
Break::set_usage_mode(UsageMode mode)
{ 
  TRACE_ENTER_MSG("Break::set_usage_mode", mode); 
  if (usage_mode != mode)
    {
      usage_mode = mode;

      TRACE_MSG("changing");
      if (mode == USAGE_MODE_NORMAL)
        {
          timer->set_activity_sensitive(true);
        }
      else if (mode == USAGE_MODE_READING)
        {
          timer->set_activity_sensitive(false);
        }
    }
  TRACE_EXIT();
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

  if (starts_with(key, CoreConfig::CFG_KEY_BREAKS, name))
    {
      TRACE_MSG("break: " << name);
      load_break_control_config();
    }
  else if (starts_with(key, CoreConfig::CFG_KEY_TIMERS, name))
    {
      TRACE_MSG("timer: " << name);
      load_timer_config();
    }
  TRACE_EXIT();
}
