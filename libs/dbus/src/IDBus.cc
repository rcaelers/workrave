// DBus.hh --- DBUS interface
//
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

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#if defined(HAVE_DBUS_QT5)
#include "DBusQt5.hh"
#elif defined(HAVE_DBUS_GIO)
#include "DBusGio.hh"
#elif defined(HAVE_DBUS_FREEDESKTOP) && defined(HAVE_GLIB)
#include "DBusFreedesktop.hh"
#else
#error "DBUS not supported on platform"
#endif

workrave::dbus::IDBus::Ptr
workrave::dbus::IDBus::create()
{
#if defined(HAVE_DBUS_QT5)
  return workrave::dbus::DBusQt5::create();
#elif defined(HAVE_DBUS_GIO)
  return workrave::dbus::DBusGio::create();
#elif defined(HAVE_DBUS_FREEDESKTOP) && defined(HAVE_GLIB)
  return workrave::dbus::DBusFreeDesktop::create();
#endif
}
