// InputMonitor.hh --- Input Monitor
//
// Copyright (C) 2001, 2002 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef INPUTMONITOR_HH
#define INPUTMONITOR_HH

#include <list>
#include "InputMonitorListenerInterface.hh"
#include "Mutex.hh"

class InputMonitorInterface;

class InputMonitor
  : public InputMonitorListenerInterface
{
public:
  static InputMonitor *get_instance();
  void unref();

  void add_listener(InputMonitorListenerInterface *listener);
  void remove_listener(InputMonitorListenerInterface *listener);

  virtual void action_notify();
  virtual void mouse_notify(int x, int y, int wheel = 0);

private:
  InputMonitor();
  ~InputMonitor();

  static list<InputMonitorListenerInterface *> listeners;
  static InputMonitor *instance;
  static int ref_count;
  static InputMonitorInterface *monitor;
  static Mutex mutex;
};

#endif // INPUTMONITOR_HH
