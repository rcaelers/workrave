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

#ifndef COREDBUS_HH
#define COREDBUS_HH

#include "dbus/IDBus.hh"
#include "utils/ScopedConnections.hh"

#include "CoreModes.hh"

#include <string>
#include <map>

class CoreDBus
{
public:
  typedef boost::shared_ptr<CoreDBus> Ptr;

  static CoreDBus::Ptr create(CoreModes::Ptr modes, workrave::dbus::IDBus::Ptr dbus);
  
  CoreDBus(CoreModes::Ptr modes, workrave::dbus::IDBus::Ptr dbus);

private:
  void on_operation_mode_changed(const OperationMode m);
  void on_usage_mode_changed(const UsageMode m);

private:
  workrave::dbus::IDBus::Ptr dbus;
  scoped_connections connections;
};

#endif // COREDBUS_HH
