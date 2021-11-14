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

#include "debug.hh"
#include <cstring>

#include <string>
#include <list>
#include <map>

#include "DBusGeneric.hh"
#include "dbus/DBusException.hh"

using namespace std;
using namespace workrave;
using namespace workrave::dbus;

void
DBusGeneric::connect(const std::string &object_path, const std::string &interface_name, void *cobject)
{
  DBusBinding *binding = find_binding(interface_name);
  if (binding == nullptr)
    {
      throw DBusException("No such interface");
    }

  auto it = objects.find(object_path);
  if (it != objects.end())
    {
      Interfaces &interfaces = it->second;

      interfaces[interface_name] = cobject;
    }
  else
    {
      Interfaces interfaces;
      interfaces[interface_name] = cobject;
      objects[object_path] = interfaces;
    }
}

void
DBusGeneric::disconnect(const std::string &object_path, const std::string &interface_name)
{
  auto it = objects.find(object_path);
  if (it != objects.end())
    {
      Interfaces &interfaces = it->second;

      interfaces.erase(interface_name);
    }
}

void
DBusGeneric::register_binding(const std::string &name, DBusBinding *interface)
{
  bindings[name] = interface;
}

DBusBinding *
DBusGeneric::find_binding(const std::string &interface_name) const
{
  DBusBinding *ret = nullptr;

  auto it = bindings.find(interface_name);
  if (it != bindings.end())
    {
      ret = it->second;
    }

  return ret;
}

void *
DBusGeneric::find_object(const std::string &path, const std::string &interface_name) const
{
  void *cobject = nullptr;

  auto object_it = objects.find(path);
  if (object_it != objects.end())
    {
      auto interface_it = object_it->second.find(interface_name);

      if (interface_it != object_it->second.end())
        {
          cobject = interface_it->second;
        }
    }

  return cobject;
}
