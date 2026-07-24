// Copyright (C) 2001 - 2014 Rob Caelers & Raymond Penners
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

#include "debug.hh"

#include "BreakDBus.hh"

#if defined(HAVE_DBUS)
#  include "DBusWorkraveNext.hh"
#endif

#include "core/CoreConfig.hh"

using namespace workrave;
using namespace workrave::dbus;
using namespace std;

BreakDBus::BreakDBus(BreakId break_id, BreakStateModel::Ptr break_state_model, std::shared_ptr<IDBus> dbus)
  : break_id(break_id)
  , break_state_model(break_state_model)
  , dbus(dbus)
{
  string break_name = CoreConfig::get_break_name(break_id);

  connect(break_state_model->signal_break_stage_changed(), this, [this](auto &&stage) {
    on_break_stage_changed(std::forward<decltype(stage)>(stage));
  });
  connect(break_state_model->signal_break_event(), this, [this](auto &&event) {
    on_break_event(std::forward<decltype(event)>(event));
  });

  try
    {
      string path = string("/org/workrave/Workrave/Break/" + break_name);
      dbus->connect(path, "org.workrave.BreakInterface", this);
      dbus->register_object_path(path);
    }
  catch (dbus::DBusException &)
    {
    }
}

void
BreakDBus::on_break_event(BreakEvent event)
{
#if defined(HAVE_DBUS)
  org_workrave_BreakInterface *iface = org_workrave_BreakInterface::instance(dbus);
  if (iface != nullptr)
    {
      string break_name = CoreConfig::get_break_name(break_id);
      iface->BreakEvent("/org/workrave/Workrave/Break/" + break_name, event);
    }
#endif
}

void
BreakDBus::on_break_stage_changed(BreakStage stage)
{
  (void)stage;
  (void)break_id;

#if defined(HAVE_DBUS)
  std::string progress = Break::get_stage_text(stage);

  if (!progress.empty())
    {
      org_workrave_BreakInterface *iface = org_workrave_BreakInterface::instance(dbus);
      if (iface != nullptr)
        {
          string break_name = CoreConfig::get_break_name(break_id);
          iface->BreakStateChanged("/org/workrave/Workrave/Break/" + break_name, progress);
        }
    }
#endif
}
