// AppletWindow.cc --- Applet info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Rob Caelers & Raymond Penners
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
// TODO: release CORBA memory.
// TODO: refactor. split into 4 classes.

static const char rcsid[] = "$Id$";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "AppletWindow.hh"
#include "ITimerBoxView.hh"
#include "TimerBoxControl.hh"

AppletWindow::AppletWindow()
  : timer_box_view(0),
    timer_box_control(0)
{
}

AppletWindow::~AppletWindow()
{
}

void
AppletWindow::set_timers_tooltip(std::string& tip)
{
  if (timer_box_view)
    {
      timer_box_view->set_tip(tip);
    }
}


void
AppletWindow::update_applet()
{
  TRACE_ENTER("AppletWindow::update_applet");
  if (timer_box_control)
    {
      timer_box_control->update();
    }
  TRACE_EXIT();
}
