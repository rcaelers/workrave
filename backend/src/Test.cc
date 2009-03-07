// DBus.cc --- The main controller
//
// Copyright (C) 2006, 2007, 2008 Rob Caelers <robc@krandor.nl>
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

static const char rcsid[] = "$Id: DBus.cc 1217 2007-07-01 21:09:19Z rcaelers $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_DBUS
#include "DBus.hh"
#endif

#ifdef HAVE_TESTS

#include "nls.h"

#include "Test.hh"
#include "CoreFactory.hh"
#include "Core.hh"
#include "IApp.hh"
#include "FakeActivityMonitor.hh"
#include "Network.hh"

Test *Test::instance = NULL;

void
Test::quit()
{
  Core *core = Core::get_instance();

  core->application->terminate();
}


void
Test::fake_monitor(int state)
{
#ifdef HAVE_DISTRIBUTION
  Core *core = Core::get_instance();
  core->fake_monitor->set_state((ActivityState)state);
#endif
}

int
Test::get_remote_state()
{
#ifdef HAVE_DISTRIBUTION
  Core *core = Core::get_instance();
  ActivityState f = core->fake_monitor->get_current_state();
  bool r = core->network->get_remote_active();

  if (!r)
    {
      return (int) f;
    }
  else
    {
      return (int) r;
    }
#endif
}


void
Test::set_time(time_t ts)
{
  Core *core = Core::get_instance();

  if (ts != 0)
    {
      core->manual_clock = true;
      core->current_time = ts;
    }
  else
    {
      core->manual_clock = false;
      core->current_time = time(NULL);
    }
}


void
Test::init_time(time_t ts)
{
  Core *core = Core::get_instance();

  if (ts != 0)
    {
      core->manual_clock = true;
      core->current_time = ts;

      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          Timer *timer = core->get_timer((BreakId)i);

          int auto_reset = (int)timer->get_auto_reset();

          timer->stop_timer();
          timer->set_state(0, auto_reset, 0);
          timer->reset_timer();
        }
    }
  else
    {
      core->manual_clock = false;
      core->current_time = time(NULL);
    }
}

void
Test::tick(int ticks)
{
  Core *core = Core::get_instance();

  if (ticks != 0)
    {
      core->manual_clock = true;

      core->heartbeat();
      for (int i = 1; i < ticks; i++)
        {
          core->current_time = core->current_time + 1;
          core->heartbeat();
        }
    }
  else
    {
      core->manual_clock = false;
    }
}




void
Test::get_activity_history(LinkedHistoryManager::Activity **activity)
{
  Core *core = Core::get_instance();

  Network *router = core->get_networking();
  LinkedHistoryManager *linked_history = router->linked_history;

  *activity = linked_history->activity_log;
}

void
Test::get_settings_history(LinkedHistoryManager::Settings **settings)
{
  Core *core = Core::get_instance();

  Network *router = core->get_networking();
  LinkedHistoryManager *linked_history = router->linked_history;

  *settings = linked_history->settings_log;
}

#endif
