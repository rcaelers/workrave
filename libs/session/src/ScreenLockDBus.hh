// SystemLockDBus.hh -- support for locking the system using DBus
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

#ifndef SYSTEMLOCKDBUS_HH
#define SYSTEMLOCKDBUS_HH

#include "session/IScreenLockMethod.hh"
#include "utils/DBusProxy.hh"

class ScreenLockDBus : public IScreenLockMethod
{
public:
  // dbus_lock_method - method to execute on the interface in order to lock the system
  // dbus_method_to_check_existence - method to execute in order to check if the interface
  //                                 is implemented
  // all parameters are owned by the caller and should be destroyed after
  // destroyal of this object
  ScreenLockDBus(GDBusConnection *connection,
                 const char *dbus_name,
                 const char *dbus_path,
                 const char *dbus_interface,
                 const char *dbus_lock_method,
                 const char *dbus_method_to_check_existence);

  ~ScreenLockDBus() override = default;

  bool is_lock_supported() override
  {
    return proxy.is_valid();
  };
  bool lock() override;

private:
  const char *dbus_lock_method;
  DBusProxy proxy;
};

#endif
