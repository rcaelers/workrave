// InputMonitor.cc --- Input Monitor
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

#include "InputMonitor.hh"

#if defined(HAVE_X)
#include "X11InputMonitor.hh"
#elif defined(WIN32)
#include "Win32InputMonitor.hh"
#endif

InputMonitor *InputMonitor::instance = NULL;
InputMonitorInterface *InputMonitor::monitor = NULL;
int InputMonitor::ref_count = 0;
list<InputMonitorListenerInterface *> InputMonitor::listeners;

InputMonitor::InputMonitor()
{
  ref_count = 0;
#if defined(HAVE_X)
  monitor = new X11InputMonitor();
#elif defined(WIN32)
  monitor = new Win32InputMonitor();
#else
#error InputMonitor not ported
#endif
  monitor->init(this);
  listeners.clear();
}

InputMonitor::~InputMonitor()
{
  ref_count--;
  if (! ref_count)
    {
      monitor->terminate();
    }
  instance = NULL;
}

InputMonitor *
InputMonitor::get_instance()
{
  if (! instance)
    {
      instance = new InputMonitor();
    }
  instance->ref_count++;
  return instance;
}

void
InputMonitor::add_listener(InputMonitorListenerInterface *listener)
{
  listeners.push_back(listener);
}

void
InputMonitor::remove_listener(InputMonitorListenerInterface *listener)
{
  listeners.remove(listener);
}

void
InputMonitor::action_notify()
{
  for (list<InputMonitorListenerInterface *>::const_iterator i
         = listeners.begin(); i != listeners.end(); i++)
    {
      (*i)->action_notify();
    }
}

void
InputMonitor::mouse_notify(int x, int y, int wheel = 0)
{
  for (list<InputMonitorListenerInterface *>::const_iterator i
         = listeners.begin(); i != listeners.end(); i++)
    {
      (*i)->mouse_notify(x, y, wheel);
    }
}
