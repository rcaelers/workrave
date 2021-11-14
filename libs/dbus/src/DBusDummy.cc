// Copyright (C) 2007 - 2013 Rob Caelers <robc@krandor.nl>
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

#include "DBusDummy.hh"

using namespace std;
using namespace workrave;
using namespace workrave::dbus;

void
DBusDummy::init()
{
}

void
DBusDummy::register_service(const std::string &service, IDBusWatch *cb)
{
  (void)service;
  (void)cb;
}

void
DBusDummy::register_object_path(const string &object_path)
{
  (void)object_path;
}

bool
DBusDummy::is_available() const
{
  return false;
}

bool
DBusDummy::is_running(const std::string &name) const
{
  (void)name;
  return false;
}

void
DBusDummy::watch(const std::string &name, IDBusWatch *cb)
{
  (void)name;
  (void)cb;
}

void
DBusDummy::unwatch(const std::string &name)
{
  (void)name;
}

void
DBusDummy::connect(const std::string &object_path, const std::string &interface_name, void *cobject)
{
  (void)object_path;
  (void)interface_name;
  (void)cobject;
}

void
DBusDummy::disconnect(const std::string &object_path, const std::string &interface_name)
{
  (void)object_path;
  (void)interface_name;
}

void
DBusDummy::register_binding(const std::string &name, DBusBinding *interface)
{
  (void)name;
  (void)interface;
}

DBusBinding *
DBusDummy::find_binding(const std::string &interface_name) const
{
  (void)interface_name;
  return nullptr;
}
