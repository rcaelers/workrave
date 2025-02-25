// SystemStateChangeConsolekit.hh -- shutdown using ConsoleKit
//
// Copyright (C) 2014 Mateusz Jończyk <mat.jonczyk@o2.pl>
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
//

#ifndef SYSTEMSTATECHANGECONSOLEKIT_HH_
#define SYSTEMSTATECHANGECONSOLEKIT_HH_

#include "utils/DBusProxy.hh"

#include "session/ISystemStateChangeMethod.hh"

//  - ConsoleKit:
//    - http://www.freedesktop.org/software/ConsoleKit/doc/ConsoleKit.html#dbus-reference
//    - only restart / shutdown,
//    - it is literally dead as Ubuntu is switching to logind for 14.04

class SystemStateChangeConsolekit : public ISystemStateChangeMethod
{
public:
  static const char *dbus_name;
  explicit SystemStateChangeConsolekit(GDBusConnection *connection);
  ~SystemStateChangeConsolekit() override = default;

  bool shutdown() override;
  bool canShutdown() override
  {
    return can_shutdown;
  }

private:
  bool can_shutdown;

  DBusProxy proxy;
};

#endif /* SYSTEMSTATECHANGECONSOLEKIT_HH_ */
