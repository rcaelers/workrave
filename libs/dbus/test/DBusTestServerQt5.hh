// Test.hh
//
// Copyright (C) 2013 Rob Caelers & Raymond Penners
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

#ifndef DBUSTESTSERVERQT5_HH
#define DBUSTESTSERVERQT5_HH

#include <string>
#include <set>

#include <QDBusArgument>
#include <QMetaType>

#include "DBusTestData.hh"
#include "DBusTestServer.hh"

#include "dbus/IDBus.hh"


class DBusTestServerQt5 : DBusTestServer
{
public:
  DBusTestServerQt5();
  virtual ~DBusTestServerQt5();

  void run(int argc, char **argv);

  void test_fire_signal();
  void test_fire_signal_without_args();
  void test_fire_signal_with_ref();
  
private:
  QCoreApplication *app;
  workrave::dbus::IDBus::Ptr dbus;
};

#endif // DBUSTESTSERVERQT5_HH
