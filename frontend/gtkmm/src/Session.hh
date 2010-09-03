// Session.cc --- Monitor the gnome session
//
// Copyright (C) 2010 Rob Caelers <robc@krandor.nl>
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

#ifndef GNOMESESSION_HH
#define GNOMESESSION_HH

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_DBUS
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
  
#if defined(HAVE_DBUS) && defined(HAVE_GNOME)
public:
  void init_gnome();

private:  
  DBusGConnection *connection;
#endif // defined(HAVE_DBUS) && defined(HAVE_GNOME)

private:
  bool is_idle;

  //! Operation mode before the screen was locked.
  workrave::OperationMode mode_before_screenlock;
};

#endif // SESSION_HH
