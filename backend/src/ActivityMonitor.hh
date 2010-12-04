// ActivityMonitor.hh --- ActivityMonitor functionality
//
// Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007, 2010 Rob Caelers <robc@krandor.nl>
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

#ifndef ACTIVITYMONITOR_HH
#define ACTIVITYMONITOR_HH

#include "IActivityMonitor.hh"
#include "IInputMonitorListener.hh"
#include "Mutex.hh"

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

class ActivityListener;
class IInputMonitor;

class ActivityMonitor :
  public IInputMonitorListener,
  public IActivityMonitor
{
public:
  ActivityMonitor();
  virtual ~ActivityMonitor();

  void terminate();
  void suspend();
  void resume();
  void force_idle();
  void shift_time(int delta);

  ActivityState get_current_state();

  void set_parameters(int noise, int activity, int idle);
  void get_parameters(int &noise, int &activity, int &idle);

  void set_listener(ActivityMonitorListener *l);

  void action_notify();
  void mouse_notify(int x, int y, int wheel = 0);
  void button_notify(bool is_press);
  void keyboard_notify(bool repeat);
  
private:
  void call_listener();

private:
  //! The actual monitoring driver.
  IInputMonitor *input_monitor;

  //! the current state.
  ActivityState activity_state;

  //! Internal locking
  Mutex lock;

  //! Previous X coordinate
  int prev_x;

  //! Previous Y coordinate
  int prev_y;

  //! Previous X-click coordinate
  int click_x;

  //! Previous Y-click coordinate
  int click_y;

  //! Is the button currently pressed?
  bool button_is_pressed;

  //! Last time activity was detected
  GTimeVal last_action_time;

  //! First time the \c ACTIVITY_IDLE state was left.
  GTimeVal first_action_time;

  //! The noise threshold
  GTimeVal noise_threshold;

  //! The activity threshold.
  GTimeVal activity_threshold;

  //! The idle threshold.
  GTimeVal idle_threshold;

  //! Activity listener.
  ActivityMonitorListener *listener;
};

#endif // ACTIVITYMONITOR_HH
