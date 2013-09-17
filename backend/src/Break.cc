// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#include "Break.hh"
#include "CoreConfig.hh"

using namespace std;
using namespace workrave::config;
using namespace workrave::dbus;

Break::Ptr
Break::create(BreakId break_id,
              IApp *app,
              Timer::Ptr timer,
              LocalActivityMonitor::Ptr activity_monitor,
              Statistics::Ptr statistics,
              IConfigurator::Ptr configurator,
              IDBus::Ptr dbus)
{
  return Ptr(new Break(break_id, app, timer, activity_monitor, statistics, configurator, dbus));
}

Break::Break(BreakId break_id,
             IApp *app,
             Timer::Ptr timer,
             LocalActivityMonitor::Ptr activity_monitor,
             Statistics::Ptr statistics,
             IConfigurator::Ptr configurator,
             IDBus::Ptr dbus)
  : break_id(break_id),
    timer(timer)
{
  break_state_model = BreakStateModel::create(break_id, app, timer, activity_monitor, configurator);
  break_statistics = BreakStatistics::create(break_id, break_state_model, timer, statistics);
  break_configuration = BreakConfig::create(break_id, break_state_model, timer, configurator);
  break_dbus = BreakDBus::create(break_id, break_state_model, dbus);
}

Break::~Break()
{
}

boost::signals2::signal<void(BreakEvent)> &
Break::signal_break_event()
{
  return break_state_model->signal_break_event();
}

string
Break::get_name() const
{
  return CoreConfig::get_break_name(break_id);
}

bool
Break::is_enabled() const
{
  return break_configuration->is_enabled();
}

bool
Break::is_running() const
{
  return timer->is_running();
}

bool
Break::is_taking() const
{
  return break_state_model->is_taking();
}

bool
Break::is_active() const
{
  return break_state_model->is_active();
}

int64_t
Break::get_elapsed_time() const
{
  return timer->get_elapsed_time();
}

int64_t
Break::get_elapsed_idle_time() const
{
  return timer->get_elapsed_idle_time();
}

int64_t
Break::get_auto_reset() const
{
  return timer->get_auto_reset();
}

bool
Break::is_auto_reset_enabled() const
{
  return timer->is_auto_reset_enabled();
}

int64_t
Break::get_limit() const
{
  return timer->get_limit();
}

bool
Break::is_limit_enabled() const
{
  return timer->is_limit_enabled();
}

int64_t
Break::get_total_overdue_time() const
{
  return timer->get_total_overdue_time();
}

void
Break::process()
{
  break_state_model->process();
}

void
Break::postpone_break()
{
  break_state_model->postpone_break();
}

void
Break::skip_break()
{
  break_state_model->skip_break();
}

void
Break::start_break()
{
  break_state_model->start_break();
}

void
Break::stop_break()
{
  break_state_model->stop_break();
}

void
Break::force_start_break(BreakHint break_hint)
{
  break_state_model->force_start_break(break_hint);
}

void
Break::override(BreakId id)
{
  break_state_model->override(id);
}

bool
Break::is_microbreak_used_for_activity() const
{
  return break_configuration->is_microbreak_used_for_activity();
}
