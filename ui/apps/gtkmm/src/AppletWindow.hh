// AppletWindow.hh --- Applet window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2011, 2012 Rob Caelers & Raymond Penners
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

#ifndef APPLETWINDOW_HH
#define APPLETWINDOW_HH

#include "commonui/preinclude.h"

#include "IAppletWindow.hh"

class TimerBoxControl;
class ITimerBoxView;

class AppletWindow : public IAppletWindow
{
public:
  AppletWindow();
  virtual ~AppletWindow();

  virtual void init_applet();
  virtual void update_applet();
  virtual void set_applet_tooltip(const std::string &tip);
  virtual sigc::signal<void, AppletState> &signal_state_changed();
  virtual sigc::signal<void> &signal_request_activate();

protected:
  //! Box container all the timers.
  ITimerBoxView *timer_box_view;

  //! Box container controller.
  TimerBoxControl *timer_box_control;

  //! Signal to indicate that the applet changed state.
  sigc::signal<void, AppletState> state_changed_signal;

  //! Signal to indicate that the applet requests to be activated.
  sigc::signal<void> request_activate_signal;
};

#endif // APPLETWINDOW_HH
