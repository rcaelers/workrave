// SystemStateChangeUPower.hh -- shutdown/suspend/hibernate using systemd-logind
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

#ifndef SYSTEMSTATECHANGEUPOWER_HH_
#define SYSTEMSTATECHANGEUPOWER_HH_

#include "utils/DBusProxy.hh"

#include "session/ISystemStateChangeMethod.hh"

//  - upower:
//    - http://upower.freedesktop.org/docs/UPower.html
//    - methods: CanSuspend, CanHibernate,
//    - these are implemented since a long time ago
//    - http://upower.freedesktop.org/docs/KbdBacklight.html
//      - control of keyboard backlight, may be usable in addition to dpms
//    - we may use libupower, but it is unnecessary

class SystemStateChangeUPower : public ISystemStateChangeMethod
{
public:
  explicit SystemStateChangeUPower(GDBusConnection *connection);
  ~SystemStateChangeUPower() override = default;

  bool suspend() override
  {
    return execute("Suspend");
  }
  bool hibernate() override
  {
    return execute("Hibernate");
  }

  bool canSuspend() override
  {
    return can_suspend;
  }
  bool canHibernate() override
  {
    return can_hibernate;
  }

  static const char *dbus_name;

private:
  bool check_method(const char *method_name);
  bool check_property(const char *property_name);
  bool execute(const char *method_name);

  bool can_suspend;
  bool can_hibernate;

  DBusProxy proxy;
  DBusProxy property_proxy;
};

#endif /* SYSTEMSTATECHANGEUPOWER_HH_ */
