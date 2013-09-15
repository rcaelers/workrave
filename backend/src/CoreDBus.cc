// Copyright (C) 2013 Rob Caelers
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

#include "dbus/IDBus.hh"

#ifdef HAVE_DBUS
#include "DBusWorkrave.hh"
#endif
#include "CoreModes.hh"

using namespace workrave::dbus;

CoreDBus::Ptr
CoreDBus::create(CoreModes::Ptr modes, IDBus::Ptr dbus)
{
  return Ptr(new CoreDBus(modes, dbus));
}

CoreDBus::CoreDBus(CoreModes::Ptr modes, IDBus::Ptr dbus) : dbus(dbus)
{
  modes->signal_operation_mode_changed().connect(boost::bind(&CoreDBus::on_operation_mode_changed, this, _1)); 
  modes->signal_usage_mode_changed().connect(boost::bind(&CoreDBus::on_usage_mode_changed, this, _1));
}

void
CoreDBus::on_operation_mode_changed(const OperationMode operation_mode)
{
#ifdef HAVE_DBUS
  org_workrave_CoreInterface *iface = org_workrave_CoreInterface::instance(dbus);
  if (iface != NULL)
    {
      iface->OperationModeChanged("/org/workrave/Workrave/Core", operation_mode);
    }
#endif      
}

void
CoreDBus::on_usage_mode_changed(const UsageMode usage_mode)
{
#ifdef HAVE_DBUS
  org_workrave_CoreInterface *iface = org_workrave_CoreInterface::instance(dbus);
  if (iface != NULL)
    {
      iface->UsageModeChanged("/org/workrave/Workrave/Core", usage_mode);
    }
#endif
}
