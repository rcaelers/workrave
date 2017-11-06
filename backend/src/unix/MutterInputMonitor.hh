// Copyright (C) 2016 Rob Caelers <robc@krandor.nl>
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

#ifndef MUTTERINPUTMONITOR_HH
#define MUTTERINPUTMONITOR_HH

#include "InputMonitor.hh"

#include <gio/gio.h>

#include "Runnable.hh"
#include "Thread.hh"

class MutterInputMonitor :
  public InputMonitor,
  public Runnable
{
public:
  MutterInputMonitor();

  //! Destructor.
  virtual ~MutterInputMonitor();

  //! Initialize
  virtual bool init();

  //! Terminate the monitor.
  virtual void terminate();

private:
  static void on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);

  //! The monitor's execution thread.
  virtual void run();

  bool register_active_watch();
  bool unregister_active_watch();
  bool register_idle_watch();
  bool unregister_idle_watch();

private:
  GDBusProxy *proxy = NULL;
  bool active = false;
  guint watch_active = 0;
  guint watch_idle = 0;

  bool abort = false;
  Thread *monitor_thread = NULL;
  // TOOO: replace deprecated functions
  GMutex *mutex = NULL;
  GCond *cond = NULL;
};

#endif // MUTTERINPUTMONITOR_HH
