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
#include "Mutex.hh"
#include "debug.hh"

#if defined(HAVE_X)
#include "X11InputMonitor.hh"
#elif defined(WIN32)
#include "Win32InputMonitor.hh"
#endif

InputMonitor *InputMonitor::instance = NULL;
InputMonitorInterface *InputMonitor::monitor = NULL;
int InputMonitor::ref_count = 0;
list<InputMonitorListenerInterface *> InputMonitor::listeners;
Mutex InputMonitor::mutex;

InputMonitor::InputMonitor()
{
  TRACE_ENTER("InputMonitor::InputMonitor");
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
  TRACE_ENTER("InputMonitor::~InputMonitor");
  mutex.lock();
  monitor->terminate();
  mutex.unlock();
}

void
InputMonitor::unref()
{
  mutex.lock();
  ref_count--;
  if (! ref_count)
    {
      delete instance;
      instance = NULL;
    }
  mutex.unlock();
}

InputMonitor *
InputMonitor::get_instance()
{
  TRACE_ENTER("InputMonitor::get_instance");
  mutex.lock();
  if (! instance)
    {
      instance = new InputMonitor();
    }
  ref_count++;
  mutex.unlock();
  return instance;
}

void
InputMonitor::add_listener(InputMonitorListenerInterface *listener)
{
  TRACE_ENTER_MSG("InputMonitor::add_listener", listener);
  mutex.lock();
  listeners.push_back(listener);
  mutex.unlock();
}

void
InputMonitor::remove_listener(InputMonitorListenerInterface *listener)
{
  TRACE_ENTER_MSG("InputMonitor::remove_listener", listener);

  mutex.lock();
  listeners.remove(listener);
  mutex.unlock();
}

void
InputMonitor::action_notify()
{
  mutex.lock();
  for (list<InputMonitorListenerInterface *>::const_iterator i
         = listeners.begin(); i != listeners.end(); i++)
    {
      (*i)->action_notify();
    }
  mutex.unlock();
}

void
InputMonitor::mouse_notify(int x, int y, int wheel = 0)
{
  mutex.lock();
  for (list<InputMonitorListenerInterface *>::const_iterator i
         = listeners.begin(); i != listeners.end(); i++)
    {
      (*i)->mouse_notify(x, y, wheel);
    }
  mutex.unlock();
}
