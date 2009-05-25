// AppletWindow.hh --- Applet window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009 Rob Caelers & Raymond Penners
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
// $Id$
//

#ifndef APPLETWINDOW_HH
#define APPLETWINDOW_HH

#include "preinclude.h"

#include <iostream>

#include "Orientation.hh"
using namespace workrave;

class TimerBoxControl;
class ITimerBoxView;

class AppletWindow
{
public:
  enum AppletState
    {
      APPLET_STATE_DISABLED,
      APPLET_STATE_VISIBLE,
      APPLET_STATE_PENDING
    };

  AppletWindow();
  virtual ~AppletWindow();

  virtual AppletState activate_applet() = 0;
  virtual void deactivate_applet() = 0;

  virtual void update_applet();

  virtual void set_timers_tooltip(std::string& tip);

protected:
  //! Box container all the timers.
  ITimerBoxView *timer_box_view;

  //! Box container controller.
  TimerBoxControl *timer_box_control;

};

#endif // APPLETWINDOW_HH
