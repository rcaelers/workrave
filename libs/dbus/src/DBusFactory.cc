// Copyright (C) 2007, 2008, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#include <memory>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "dbus/DBusFactory.hh"

#if defined(HAVE_DBUS_DUMMY)
#  include "DBusDummy.hh"
#elif defined(HAVE_DBUS_QT)
#  include "DBusQt.hh"
#elif defined(HAVE_DBUS_GIO)
#  include "DBusGio.hh"
#else
#  include "DBusDummy.hh"
#endif

std::shared_ptr<workrave::dbus::IDBus>
workrave::dbus::DBusFactory::create()
{
#if defined(HAVE_DBUS_DUMMY)
  return std::make_shared<workrave::dbus::DBusDummy>();
#elif defined(HAVE_DBUS_QT)
  return std::make_shared<workrave::dbus::DBusQt>();
#elif defined(HAVE_DBUS_GIO)
  return std::make_shared<workrave::dbus::DBusGio>();
#else
  return std::make_shared<workrave::dbus::DBusDummy>();
#endif
}
