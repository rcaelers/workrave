// Copyright (C) 2001 - 2012 Rob Caelers & Raymond Penners
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

#ifndef WINDOWSAPPLETWINDOW_HH
#define WINDOWSAPPLETWINDOW_HH

#include <windows.h>
#include <process.h>
#include <string>

#include "ui/ITimerBoxView.hh"
#include "ui/TimerBoxControl.hh"
#include "ui/UiTypes.hh"
#include "utils/Signals.hh"

#include "Applet.hh"
#include "commonui/MenuModel.hh"
#include "commonui/MenuHelper.hh"
#include "ui/AppHold.hh"
#include "ui/Plugin.hh"

class WindowsAppletWindow
  : public Plugin<WindowsAppletWindow>
  , public ITimerBoxView
  , public workrave::utils::Trackable
{
public:
  WindowsAppletWindow(std::shared_ptr<IPluginContext> context);
  ~WindowsAppletWindow() override;

  std::string get_plugin_id() const override
  {
    return "workrave.WindowsAppletWindow";
  }

  void set_slot(workrave::BreakId id, int slot) override;
  void set_time_bar(workrave::BreakId id,
                    int value,
                    TimerColorId primary_color,
                    int primary_value,
                    int primary_max,
                    TimerColorId secondary_color,
                    int secondary_value,
                    int secondary_max) override;
  void set_icon(OperationModeIcon icon) override;
  void update_view() override;

  bool filter_func(MSG *event);

private:
  static unsigned __stdcall run_event_pipe_static(void *);

  void send_time_bars();
  void send_menu();

  bool is_visible() const;
  void init_menu(HWND dest);
  void init_thread();
  bool on_applet_command(int command);
  void update_applet_window();
  void init_menu_list(std::list<AppletMenuItem> &items, menus::Node::Ptr node);
  void init_menu();
  void init_toolkit();
  void run_event_pipe();

  std::shared_ptr<IPluginContext> context;
  MenuModel::Ptr menu_model;
  MenuHelper menu_helper;
  AppHold apphold;

  HWND applet_window{nullptr};
  bool menu_sent{false};
  TimerBoxControl *control{nullptr};

  AppletHeartbeatData local_heartbeat_data;
  AppletHeartbeatData heartbeat_data;

  HWND local_applet_window{nullptr};

  CRITICAL_SECTION heartbeat_data_lock;
  HANDLE heartbeat_data_event{nullptr};
  HANDLE thread_abort_event{nullptr};
  HANDLE thread_handle{nullptr};
  volatile unsigned thread_id{0};
};

#endif // WINDOWSAPPLETWINDOW_HH
