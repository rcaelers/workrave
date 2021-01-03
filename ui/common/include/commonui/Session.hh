// Session.cc --- Monitor the gnome session
//
// Copyright (C) 2010, 2011, 2013 Rob Caelers <robc@krandor.nl>
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

#include <memory>

#if defined(HAVE_DBUS_GIO)
#include <gio/gio.h>
#endif

class Session
{
public:
  using Ptr = std::shared_ptr<Session>;

  Session();
  void init();

  void set_idle(bool idle);

#if defined(HAVE_DBUS_GIO)
private:
  void init_gnome();
  static void on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);
#endif

private:
  bool is_idle{false};
  bool taking{false};
};

#endif // SESSION_HH
