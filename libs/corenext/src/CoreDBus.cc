// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#include "CoreDBus.hh"

#include "dbus/IDBus.hh"

#if defined(HAVE_DBUS)
#  include "DBusWorkraveNext.hh"
#endif
#include "CoreModes.hh"

using namespace workrave;
using namespace workrave::dbus;

CoreDBus::CoreDBus(CoreModes::Ptr modes, IDBus::Ptr dbus)
  : dbus(dbus)
{
  connect(modes->signal_operation_mode_changed(), this, [this](auto &&mode) {
    on_operation_mode_changed(std::forward<decltype(mode)>(mode));
  });
  connect(modes->signal_usage_mode_changed(), this, [this](auto &&mode) {
    on_usage_mode_changed(std::forward<decltype(mode)>(mode));
  });
}

void
CoreDBus::on_operation_mode_changed(OperationMode operation_mode)
{
#if defined(HAVE_DBUS)
  org_workrave_CoreInterface *iface = org_workrave_CoreInterface::instance(dbus);
  if (iface != nullptr)
    {
      iface->OperationModeChanged("/org/workrave/Workrave/Core", operation_mode);
    }
#endif
}

void
CoreDBus::on_usage_mode_changed(UsageMode usage_mode)
{
#if defined(HAVE_DBUS)
  org_workrave_CoreInterface *iface = org_workrave_CoreInterface::instance(dbus);
  if (iface != nullptr)
    {
      iface->UsageModeChanged("/org/workrave/Workrave/Core", usage_mode);
    }
#endif
}
