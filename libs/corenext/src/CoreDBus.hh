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

#ifndef COREDBUS_HH
#define COREDBUS_HH

#include "dbus/IDBus.hh"
#include "utils/Signals.hh"

#include "CoreModes.hh"

#include <string>

class CoreDBus : public workrave::utils::Trackable
{
public:
  using Ptr = std::shared_ptr<CoreDBus>;

  CoreDBus(CoreModes::Ptr modes, std::shared_ptr<workrave::dbus::IDBus> dbus);

private:
  void on_operation_mode_changed(workrave::OperationMode operation_mode);
  void on_usage_mode_changed(workrave::UsageMode usage_mode);

private:
  std::shared_ptr<workrave::dbus::IDBus> dbus;
};

#endif // COREDBUS_HH
