// ActivityMonitor.cc --- ActivityMonitor
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <assert.h>

#include <algorithm>

#include "ActivityMonitor.hh"
#include "ActivityStateMonitor.hh"
#if defined(HAVE_X)
#include "X11InputMonitor.hh"
#elif defined(WIN32)
#include "Win32InputMonitor.hh"
#endif

//! Constructor.
ActivityMonitor::ActivityMonitor(char *display)
{
  TRACE_ENTER("ActivityMonitor::ActivityMonitor");

#if defined(HAVE_X)
  input_monitor = new X11InputMonitor(display);
#elif defined(WIN32)
  input_monitor = new Win32InputMonitor();
#endif

  activity_state = new ActivityStateMonitor();

  // TODO: perhaps move this to a start() method...
  input_monitor->init(activity_state);

 
  TRACE_EXIT();
}


//! Destructor.
ActivityMonitor::~ActivityMonitor()
{
  TRACE_ENTER("ActivityMonitor::~ActivityMonitor");

  if (input_monitor != NULL)
    {
      delete input_monitor;
    }

  if (activity_state != NULL)
    {
      delete activity_state;
    }

  TRACE_EXIT();
}
 

void
ActivityMonitor::terminate()
{
  TRACE_ENTER("ActivityMonitor::terminate");

  TRACE_MSG("Terminating input monitor");
  input_monitor->terminate();

  TRACE_MSG("deleting state monitor");

  if (activity_state != NULL)
    {
      delete activity_state;
      activity_state = NULL;
    }

  TRACE_EXIT();
}


//! Suspends the activity monitoring.
void
ActivityMonitor::suspend()
{
  activity_state->suspend();
}


//! Resumes the activity monitoring.
void
ActivityMonitor::resume()
{
  activity_state->resume();
}


//! Get current state()
ActivityState
ActivityMonitor::get_current_state() const
{
  if (activity_state != NULL)
    {
      return activity_state->get_current_state();
    }
  else
    {
      return ACTIVITY_UNKNOWN;
    }
}


//! Forces the state to be idle.
void
ActivityMonitor::force_idle()
{
  if (activity_state != NULL)
    {
      activity_state->force_idle();
    }
}


void
ActivityMonitor::set_parameters(int noise, int activity, int idle)
{ 
  activity_state->set_parameters(noise, activity, idle);
}

void
ActivityMonitor::get_parameters(int &noise, int &activity, int &idle)
{ 
  activity_state->get_parameters(noise, activity, idle);
}

void
ActivityMonitor::get_statistics(ActivityMonitorStatistics &stats) const
{
  activity_state->get_statistics(stats);
}

void
ActivityMonitor::set_statistics(const ActivityMonitorStatistics &stats)
{
  activity_state->set_statistics(stats);
}
