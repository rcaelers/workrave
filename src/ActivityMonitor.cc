// ActivityMonitor.cc --- ActivityMonitor
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-09-20 23:59:14 pennersr>
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
#include "InputMonitor.hh"

//! Constructor.
ActivityMonitor::ActivityMonitor()
{
  TRACE_ENTER("ActivityMonitor::ActivityMonitor");

  input_monitor = InputMonitor::get_instance();

  activity_state = new ActivityStateMonitor();

  // TODO: perhaps move this to a start() method...
  input_monitor->add_listener(activity_state);

  TRACE_EXIT();
}


//! Destructor.
ActivityMonitor::~ActivityMonitor()
{
  TRACE_ENTER("ActivityMonitor::~ActivityMonitor");

  if (input_monitor != NULL)
    {
      input_monitor->remove_listener(activity_state);
      input_monitor->unref();
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
