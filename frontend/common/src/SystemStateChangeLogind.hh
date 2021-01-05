// SystemStateChangeLogind.hh -- shutdown/suspend/hibernate using systemd-logind
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

#ifndef SYSTEMSTATECHANGELOGIND_HH_
#define SYSTEMSTATECHANGELOGIND_HH_

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "DBusProxy.hh"

#include "ISystemStateChangeMethod.hh"

// http://www.freedesktop.org/wiki/Software/systemd/logind/
//
// feature introduction date: http://cgit.freedesktop.org/systemd/systemd/
//                                    log/src/login/org.freedesktop.login1.conf
//      - (Can)PowerOff/Reboot - II 2012r.
//      - Suspend/HIbernate - V 2012r.
//      - HybridSuspend - XII 2012r.

class SystemStateChangeLogind : public ISystemStateChangeMethod
{
public:
  SystemStateChangeLogind(GDBusConnection *connection);
  virtual ~SystemStateChangeLogind(){};

  // PowerOff(), Reboot(), Suspend(), Hibernate(), HybridSleep()
  virtual bool shutdown() { return execute("PowerOff"); }
  virtual bool suspend() { return execute("Suspend"); }
  virtual bool hibernate() { return execute("Hibernate"); }
  virtual bool suspendHybrid() { return execute("HybridSleep"); }

  virtual bool canShutdown() { return can_shutdown; }
  virtual bool canSuspend() { return can_suspend; }
  virtual bool canHibernate() { return can_hibernate; }
  virtual bool canSuspendHybrid() { return can_suspend_hybrid; }

  static const char *dbus_name;

private:
  bool check_method(const char *method_name);
  bool execute(const char *method_name);

  bool can_shutdown;
  bool can_suspend;
  bool can_hibernate;
  bool can_suspend_hybrid;

  DBusProxy proxy;
};

#endif /* SYSTEMSTATECHANGELOGIND_HH_ */
