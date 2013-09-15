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

#include "debug.hh"

#include "BreakDBus.hh"

#ifdef HAVE_DBUS
#include "DBusWorkrave.hh"
#endif

#include "CoreConfig.hh"

using namespace workrave::dbus;
using namespace std;

BreakDBus::Ptr
BreakDBus::create(BreakId break_id,
                  BreakStateModel::Ptr break_state_model,
                  IDBus::Ptr dbus)
{
  return Ptr(new BreakDBus(break_id, break_state_model, dbus));
}


BreakDBus::BreakDBus(BreakId break_id, BreakStateModel::Ptr break_state_model, IDBus::Ptr dbus)
  : break_id(break_id),
    break_state_model(break_state_model),
    dbus(dbus)
{
  string break_name = CoreConfig::get_break_name(break_id);

  break_state_model->signal_break_stage_changed().connect(boost::bind(&BreakDBus::on_break_stage_changed, this, _1));

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


BreakDBus::~BreakDBus()
{
}

void
BreakDBus::on_break_stage_changed(BreakStage stage)
{
  (void) stage;
  
#ifdef HAVE_DBUS
  const char *progress = NULL;
  (void) progress;
  
  switch (stage)
    {
    case BreakStage::None:
      progress = "none";
      break;

    case BreakStage::Snoozed:
      progress = "none";
      break;

    case BreakStage::Delayed:
      // Do not send this stage.
      break;

    case BreakStage::Prelude:
      progress = "prelude";
      break;

    case BreakStage::Taking:
      progress = "break";
      break;
    }

  if (progress != NULL)
    {
      org_workrave_BreakInterface *iface = org_workrave_BreakInterface::instance(dbus);
      if (iface != NULL)
        {
          string break_name = CoreConfig::get_break_name(break_id);
          iface->Changed("/org/workrave/Workrave/Break/" + break_name, progress);
        }
    }
#endif
}
