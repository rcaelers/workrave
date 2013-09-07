// DBusTestServer.cc
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QtCore>
#include <QtDBus>

#include "debug.hh"

#include "DBusTestServerGio.hh"

#include "dbus/IDBus.hh"

#include "DBusGio.hh"

#define  WORKRAVE_TEST_PATH "/org/workrave/Workrave/Test"
#define  WORKRAVE_TEST_INTERFACE "org.workrave.TestInterface"
#define  WORKRAVE_TEST_SERVICE "org.workrave.Test"

using namespace std;

//! Constructor.
DBusTestServerGio::DBusTestServerGio() 
{
}


//! Destructor.
DBusTestServerGio::~DBusTestServerGio()
{
}

int main(int argc, char **argv)
{
  DBusTestServerGio s;
  s.run(argc, argv);
}

void
DBusTestServerGio::run(int argc, char **argv)
{
#ifdef TRACING
  Debug::name(std::string("server"));
#endif

  try
    {
      GMainContext *context = g_main_context_new();
      GMainLoop *loop = g_main_loop_new(context, FALSE);
      
      workrave::dbus::IDBus::Ptr dbus = workrave::dbus::DBusGio::create();
      
      dbus->init();
        
      extern void init_DBusTest(workrave::dbus::IDBus::Ptr dbus);
      init_DBusTest(dbus);

      dbus->connect(WORKRAVE_TEST_PATH, WORKRAVE_TEST_INTERFACE, this);
      dbus->register_object_path(WORKRAVE_TEST_PATH);
      dbus->register_service(WORKRAVE_TEST_SERVICE);

      g_main_loop_run(loop);
    }
  catch (workrave::dbus::DBusException &)
    {
      std::cerr << "server failed" <<std::endl;
    }
}
