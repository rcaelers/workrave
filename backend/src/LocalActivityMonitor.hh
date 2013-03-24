// LocalActivityMonitor.hh --- LocalActivityMonitor functionality
//
// Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007, 2010, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef LOCALACTIVITYMONITOR_HH
#define LOCALACTIVITYMONITOR_HH

#include "IActivityMonitor.hh"

#include "config/Config.hh"
#include "input-monitor/IInputMonitor.hh"
#include "input-monitor/IInputMonitorListener.hh"
#include "Mutex.hh"

#include "IActivityMonitorListener.hh"


using namespace workrave::config;
using namespace workrave::input_monitor;

class LocalActivityMonitor :
  public IActivityMonitor,
  public IInputMonitorListener,
  public IConfiguratorListener
{
public:
  typedef boost::shared_ptr<LocalActivityMonitor> Ptr;

public:
  static Ptr create(IConfigurator::Ptr configurator, const std::string &display_name);
  
  LocalActivityMonitor(IConfigurator::Ptr configurator, const std::string &display_name);
  virtual ~LocalActivityMonitor();

  // IActivityMonitor
  virtual void init();
  virtual void terminate();
  virtual void suspend();
  virtual void resume();
  virtual void force_idle();
  virtual ActivityState get_state();
  virtual void set_listener(IActivityMonitorListener::Ptr l);

  // IInputMonitorListener
  virtual void action_notify();
  virtual void mouse_notify(int x, int y, int wheel = 0);
  virtual void button_notify(bool is_press);
  virtual void keyboard_notify(bool repeat);
  
private:
  void call_listener();

  void load_config();
  void config_changed_notify(const std::string &key);
  void set_parameters(int noise, int activity, int idle);
  void get_parameters(int &noise, int &activity, int &idle);

  void process_state();
  
  //! State of the activity monitor.
  enum LocalActivityMonitorState
    {
      ACTIVITY_MONITOR_UNKNOWN,
      ACTIVITY_MONITOR_SUSPENDED,
      ACTIVITY_MONITOR_IDLE,
      ACTIVITY_MONITOR_FORCED_IDLE,
      ACTIVITY_MONITOR_NOISE,
      ACTIVITY_MONITOR_ACTIVE
    };
  
private:
  //! The Configurator.
  IConfigurator::Ptr configurator;

  //! 
  std::string display_name;
  
  //! The actual monitoring driver.
  IInputMonitor *input_monitor;

  //! the current state.
  LocalActivityMonitorState state;

  //! Internal locking
  Mutex lock;

  //! Previous X coordinate
  int prev_x;

  //! Previous Y coordinate
  int prev_y;

  //! Is the button currently pressed?
  bool button_is_pressed;

  //! Last time activity was detected
  gint64 last_action_time;

  //! First time the \c ACTIVITY_IDLE state was left.
  gint64 first_action_time;

  //! The noise threshold
  gint64 noise_threshold;

  //! The activity threshold.
  gint64 activity_threshold;

  //! The idle threshold.
  gint64 idle_threshold;

  //! Activity listener.
  IActivityMonitorListener::Ptr listener;
};

#endif // LOCALACTIVITYMONITOR_HH
