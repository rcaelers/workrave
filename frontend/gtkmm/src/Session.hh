// Session.cc --- Monitor the gnome session
//
// Copyright (C) 2010, 2011, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef SESSION_HH
#define SESSION_HH

#if defined(HAVE_DBUS_GIO)
#include <gio/gio.h>
#elif defined(HAVE_DBUSGLIB_GET_PRIVATE)
#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>
#endif

#include "ICore.hh"

class Session
{
public:
  Session();
  void init();

  void set_idle(bool idle);

#if defined(HAVE_DBUS_GIO)
public:
  void init_gnome();
private:
  
  static void on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);
  
#elif defined(HAVE_DBUSGLIB_GET_PRIVATE)
public:
  void init_gnome();

private:
  DBusGConnection *connection;
#endif // defined(HAVE_DBUSGLIB_GET_PRIVATE)

private:
  bool is_idle;
  bool taking;
};

#endif // SESSION_HH
