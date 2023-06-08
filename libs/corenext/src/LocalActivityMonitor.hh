// Copyright (C) 2001 - 2013 Rob Caelers <robc@krandor.nl>
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

#include <thread>
#include <mutex>

#include "IActivityMonitor.hh"

#include "config/Config.hh"
#include "utils/TimeSource.hh"
#include "utils/Signals.hh"
#include "utils/Enum.hh"
#include "input-monitor/IInputMonitor.hh"
#include "input-monitor/IInputMonitorListener.hh"

class LocalActivityMonitor
  : public IActivityMonitor
  , public workrave::input_monitor::IInputMonitorListener
  , public workrave::utils::Trackable
{
public:
  LocalActivityMonitor(workrave::config::IConfigurator::Ptr config, const char *display_name);
  ~LocalActivityMonitor() override = default;

  void init() override;
  void terminate() override;
  void suspend() override;
  void resume() override;
  void force_idle() override;
  bool is_active() override;
  void set_listener(IActivityMonitorListener::Ptr l) override;

  // IInputMonitorListener
  void action_notify() override;
  void mouse_notify(int x, int y, int wheel = 0) override;
  void button_notify(bool is_press) override;
  void keyboard_notify(bool repeat) override;

private:
  void call_listener();

  void load_config();
  void set_parameters(int noise, int activity, int idle, int sensitivity);
  void get_parameters(int &noise, int &activity, int &idle, int &sensitivity) const;

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
  workrave::config::IConfigurator::Ptr config;

  //!
  const char *display_name;

  //! The actual monitoring driver.
  workrave::input_monitor::IInputMonitor::Ptr input_monitor;

  //! the current state.
  LocalActivityMonitorState state{ACTIVITY_MONITOR_IDLE};

  //! Internal locking
  std::recursive_mutex lock;

  //! Previous X coordinate
  int prev_x{-10};

  //! Previous Y coordinate
  int prev_y{-10};

  //! Is the button currently pressed?
  bool button_is_pressed{false};

  //! Last time activity was detected
  int64_t last_action_time{0};

  //! First time the \c ACTIVITY_IDLE state was left.
  int64_t first_action_time{0};

  //! The noise threshold
  int64_t noise_threshold{1 * workrave::utils::TimeSource::TIME_USEC_PER_SEC};

  //! The activity threshold.
  int64_t activity_threshold{2 * workrave::utils::TimeSource::TIME_USEC_PER_SEC};

  //! The idle threshold.
  int64_t idle_threshold{5 * workrave::utils::TimeSource::TIME_USEC_PER_SEC};

  //! Mouse sensitivity
  int sensitivity{3};

  //! Activity listener.
  IActivityMonitorListener::Ptr listener;

  friend struct workrave::utils::enum_traits<LocalActivityMonitor::LocalActivityMonitorState>;
};

template<>
struct workrave::utils::enum_traits<LocalActivityMonitor::LocalActivityMonitorState>
{
  static constexpr std::array<std::pair<std::string_view, LocalActivityMonitor::LocalActivityMonitorState>, 6> names{
    {{"unknown", LocalActivityMonitor::ACTIVITY_MONITOR_UNKNOWN},
     {"suspended", LocalActivityMonitor::ACTIVITY_MONITOR_SUSPENDED},
     {"idle", LocalActivityMonitor::ACTIVITY_MONITOR_IDLE},
     {"forced-idle", LocalActivityMonitor::ACTIVITY_MONITOR_FORCED_IDLE},
     {"noise", LocalActivityMonitor::ACTIVITY_MONITOR_NOISE},
     {"active", LocalActivityMonitor::ACTIVITY_MONITOR_ACTIVE}}};
};

#endif // LOCALACTIVITYMONITOR_HH
