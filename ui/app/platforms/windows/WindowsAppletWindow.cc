// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "WindowsAppletWindow.hh"

#include <string>

#include "ui/Text.hh"
//#include "GUI.hh"
#include "Applet.hh"

#include "ui/TimerBoxControl.hh"
#include "commonui/nls.h"
#include "debug.hh"
#include "ui/windows/IToolkitWindows.hh"

using namespace workrave;

#if defined(interface)
#  undef interface
#endif

WindowsAppletWindow::WindowsAppletWindow(std::shared_ptr<IApplication> app)
  : toolkit(app->get_toolkit())
  , menu_model(app->get_menu_model())
  , menu_helper(menu_model)
  , apphold(toolkit)
{
  TRACE_ENTER("WindowsAppletWindow::WindowsAppletWindow");

  memset(&local_heartbeat_data, 0, sizeof(AppletHeartbeatData));
  memset(&local_menu_data, 0, sizeof(AppletMenuData));
  memset(&heartbeat_data, 0, sizeof(AppletHeartbeatData));
  memset(&menu_data, 0, sizeof(AppletMenuData));

  heartbeat_data.enabled = true;

  ::InitializeCriticalSection(&heartbeat_data_lock);
  thread_abort_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
  heartbeat_data_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);

  control = new TimerBoxControl(app, "applet", this);

  workrave::utils::connect(menu_model->signal_update(), this, [this]() { init_menu(); });
  workrave::utils::connect(menu_helper.signal_update(), this, [this](auto node) {
    if (auto n = std::dynamic_pointer_cast<menus::RadioNode>(node); !n)
      {
        init_menu();
      }
  });

  menu_helper.setup_event();

  init_menu();
  init_thread();

  TRACE_EXIT();
}

WindowsAppletWindow::~WindowsAppletWindow()
{
  TRACE_ENTER("WindowsAppletWindow::~WindowsAppletWindow");

  /* before this instance is destroyed we signal and wait for its worker thread to terminate. this
  isn't ideal because the gui will be blocked while we wait for termination if this destructor is
  called from the main thread. current conditions are acceptable, however. 2/12/2012
  */
  heartbeat_data.enabled = false;
  SetEvent(thread_abort_event);
  if (thread_handle)
    {
      WaitForSingleObject(thread_handle, INFINITE);
      CloseHandle(thread_handle);
    }

  if (thread_abort_event)
    CloseHandle(thread_abort_event);

  if (heartbeat_data_event)
    CloseHandle(heartbeat_data_event);

  DeleteCriticalSection(&heartbeat_data_lock);

  delete control;

  TRACE_EXIT();
}

static HWND
RecursiveFindWindow(HWND hwnd, LPCSTR lpClassName)
{
  static char buf[80];
  int num = GetClassNameA(hwnd, buf, sizeof(buf) - 1);
  buf[num] = 0;
  HWND ret = NULL;

  if (!stricmp(lpClassName, buf))
    {
      ret = hwnd;
    }
  else
    {
      HWND child = FindWindowEx(hwnd, 0, NULL, NULL);
      while (child != NULL)
        {
          ret = RecursiveFindWindow(child, lpClassName);
          if (ret)
            {
              break;
            }
          child = FindWindowEx(hwnd, child, NULL, NULL);
        }
    }
  return ret;
}

void
WindowsAppletWindow::set_slot(workrave::BreakId id, int slot)
{
  TRACE_ENTER_MSG("WindowsAppletWindow::set_slot", int(id) << ", " << slot);
  heartbeat_data.slots[slot] = (short)id;
  TRACE_EXIT();
}

void
WindowsAppletWindow::set_time_bar(workrave::BreakId id,
                                  int value,
                                  TimerColorId primary_color,
                                  int primary_val,
                                  int primary_max,
                                  TimerColorId secondary_color,
                                  int secondary_val,
                                  int secondary_max)
{
  TRACE_ENTER_MSG("WindowsAppletWindow::set_time_bar", int(id) << "=" << value);
  strncpy(heartbeat_data.bar_text[id], Text::time_to_string(value).c_str(), APPLET_BAR_TEXT_MAX_LENGTH - 1);
  heartbeat_data.bar_text[id][APPLET_BAR_TEXT_MAX_LENGTH - 1] = '\0';
  heartbeat_data.bar_primary_color[id] = (int)primary_color;
  heartbeat_data.bar_primary_val[id] = primary_val;
  heartbeat_data.bar_primary_max[id] = primary_max;
  heartbeat_data.bar_secondary_color[id] = (int)secondary_color;
  heartbeat_data.bar_secondary_val[id] = secondary_val;
  heartbeat_data.bar_secondary_max[id] = secondary_max;
  TRACE_EXIT();
}

void
WindowsAppletWindow::update_view()
{
  TRACE_ENTER("WindowsAppletWindow::update_view");

  BOOL entered = ::TryEnterCriticalSection(&heartbeat_data_lock);
  if (entered)
    {
      update_applet_window();

      if (applet_window != NULL)
        {
          memcpy(&local_heartbeat_data, &heartbeat_data, sizeof(AppletHeartbeatData));
          if (!menu_sent)
            {
              memcpy(&local_menu_data, &menu_data, sizeof(AppletMenuData));
              local_applet_window = applet_window;
              menu_sent = true;
            }

          SetEvent(heartbeat_data_event);
        }

      ::LeaveCriticalSection(&heartbeat_data_lock);
    }

  TRACE_EXIT();
}

void
WindowsAppletWindow::update_menu()
{
  TRACE_ENTER("WindowsAppletWindow::update_menu");
  if (local_applet_window != NULL)
    {
      TRACE_MSG("sending" << local_menu_data.num_items);

      COPYDATASTRUCT msg;
      msg.dwData = APPLET_MESSAGE_MENU;
      msg.cbData = sizeof(AppletMenuData);
      msg.lpData = &local_menu_data;
      SendMessage(local_applet_window, WM_COPYDATA, 0, (LPARAM)&msg);
    }
  TRACE_EXIT();
}

void
WindowsAppletWindow::update_time_bars()
{
  TRACE_ENTER("WindowsAppletWindow::update_time_bars");
  if (local_applet_window != NULL)
    {
      COPYDATASTRUCT msg;
      msg.dwData = APPLET_MESSAGE_HEARTBEAT;
      msg.cbData = sizeof(AppletHeartbeatData);
      msg.lpData = &local_heartbeat_data;
      for (size_t i = 0; i < workrave::BREAK_ID_SIZEOF; i++)
        {
          TRACE_MSG("sending: slots[]=" << local_heartbeat_data.slots[i]);
        }
      SendMessage(local_applet_window, WM_COPYDATA, 0, (LPARAM)&msg);
    }
  TRACE_EXIT();
}

void
WindowsAppletWindow::update_applet_window()
{
  TRACE_ENTER("WindowsAppletWindow::get_applet_window");
  HWND previous_applet_window = applet_window;
  if (applet_window == NULL || !IsWindow(applet_window))
    {
      HWND taskbar = FindWindowA("Shell_TrayWnd", NULL);
      applet_window = RecursiveFindWindow(taskbar, APPLET_WINDOW_CLASS_NAME);
      menu_sent = false;
    }

  if (previous_applet_window == NULL && applet_window != NULL)
    {
      apphold.hold();
    }
  else if (previous_applet_window != NULL && applet_window == NULL)
    {
      apphold.release();
    }

  TRACE_EXIT();
}

void
WindowsAppletWindow::init_thread()
{
  TRACE_ENTER("WindowsAppletWindow::init_thread");
  DWORD thread_exit_code = 0;

  if (thread_id && thread_handle && GetExitCodeThread(thread_handle, &thread_exit_code) && (thread_exit_code == STILL_ACTIVE))
    return;
  TRACE_MSG("1");

  if (!thread_id)
    {
      // if there is no id but a handle then this instance's worker thread has exited or is exiting.
      if (thread_handle)
        CloseHandle(thread_handle);

      TRACE_MSG("2");

      thread_id = 0;
      SetLastError(0);
      thread_handle = (HANDLE)_beginthreadex(NULL, 0, run_event_pipe_static, this, 0, (unsigned int *)&thread_id);

      TRACE_MSG("3");

      if (!thread_handle || !thread_id)
        {
          TRACE_MSG("Thread could not be created. GetLastError : " << GetLastError());
        }
    }

  TRACE_EXIT();
}

unsigned __stdcall WindowsAppletWindow::run_event_pipe_static(void *param)
{
  TRACE_ENTER("WindowsAppletWindow::run_event_pipe_static");
  WindowsAppletWindow *pThis = (WindowsAppletWindow *)param;
  pThis->run_event_pipe();
  // invalidate the id to signal the thread is exiting
  pThis->thread_id = 0;
  TRACE_EXIT();
  return (DWORD)0;
}

void
WindowsAppletWindow::run_event_pipe()
{
  const DWORD current_thread_id = GetCurrentThreadId();
  TRACE_ENTER_MSG("WindowsAppletWindow::run_event_pipe [ id: ", current_thread_id << " ]");

  while (thread_id == current_thread_id)
    {
      /* JS: thread_abort_event must be first in the array of events.
      the index returned by WaitForMultipleObjectsEx() corresponds to the first
      signaled event in the array if more than one is signaled
      */
      HANDLE events[2] = {thread_abort_event, heartbeat_data_event};
      int const events_count = (sizeof(events) / sizeof(events[0]));

      DWORD wait_result = WaitForMultipleObjectsEx(events_count, events, FALSE, INFINITE, FALSE);

      if ((wait_result == WAIT_FAILED) || (wait_result == (WAIT_OBJECT_0 + 0)))
        break;

      if (heartbeat_data.enabled && (wait_result == (WAIT_OBJECT_0 + 1)))
        {
          EnterCriticalSection(&heartbeat_data_lock);

          update_time_bars();
          update_menu();

          LeaveCriticalSection(&heartbeat_data_lock);
        }
    }

  TRACE_EXIT();
}

void
WindowsAppletWindow::set_geometry(Orientation orientation, int size)
{
  (void)orientation;
  (void)size;
}

bool
WindowsAppletWindow::on_applet_command(int command)
{
  TRACE_ENTER_MSG("WindowsAppletWindow::on_applet_command", command);
  auto node = menu_helper.find_node(command);
  if (node)
    {
      node->activate();
    }
  TRACE_EXIT();
  return false;
}

bool
WindowsAppletWindow::filter_func(MSG *msg)
{
  TRACE_ENTER("WindowsAppletWindow::filter_func");
  bool ret = true;

  switch (msg->message)
    {
    case WM_USER:
      {
        int cmd = (int)msg->wParam;
        toolkit->create_oneshot_timer(0, [this, cmd]() { on_applet_command(cmd); });
        ret = false;
      }
      break;

    case WM_USER + 1:
      {
        control->force_cycle();
        ret = false;
      }
      break;
    }
  TRACE_EXIT();
  return ret;
}

bool
WindowsAppletWindow::is_visible() const
{
  return applet_window != NULL && heartbeat_data.enabled;
}

void
WindowsAppletWindow::init()
{
  workrave::utils::connect(toolkit->signal_timer(), this, [this]() { control->update(); });
}

void
WindowsAppletWindow::init_menu()
{
  TRACE_ENTER("WindowsAppletWindow::init_menu");
  auto toolkit_win = std::dynamic_pointer_cast<IToolkitWindows>(toolkit);
  if (toolkit_win)
    {
      menu_data.command_window = HandleToLong(toolkit_win->get_event_hwnd());
      workrave::utils::connect(toolkit_win->hook_event(), this, [this](MSG *msg) { return filter_func(msg); });
    }

  menu_data.num_items = 0;
  menu_sent = false;

  process_menu(menu_model->get_root());
  TRACE_EXIT();
}

void
WindowsAppletWindow::process_menu(menus::Node::Ptr node, bool popup)
{
  int command = menu_helper.allocate_command(node->get_id());

  if (auto n = std::dynamic_pointer_cast<menus::SubMenuNode>(node); n)
    {
      if (menu_data.num_items > 0)
        {
          popup = true;
        }

      for (auto &menu_to_add: n->get_children())
        {
          process_menu(menu_to_add, popup);
        }
      if (popup)
        {
          add_menu(n->get_text(), 0, 0);
        }
    }

  else if (auto n = std::dynamic_pointer_cast<menus::RadioGroupNode>(node); n)
    {
      for (auto &menu_to_add: n->get_children())
        {
          process_menu(menu_to_add, popup);
        }
    }

  else if (auto n = std::dynamic_pointer_cast<menus::ActionNode>(node); n)
    {
      add_menu(n->get_text(), command, (popup ? WindowsAppletWindow::MENU_FLAG_POPUP : 0));
    }

  else if (auto n = std::dynamic_pointer_cast<menus::ToggleNode>(node); n)
    {
      add_menu(n->get_text(),
               command,
               WindowsAppletWindow::MENU_FLAG_TOGGLE | (popup ? WindowsAppletWindow::MENU_FLAG_POPUP : 0)
                 | (n->is_checked() ? WindowsAppletWindow::MENU_FLAG_SELECTED : 0));
    }

  else if (auto n = std::dynamic_pointer_cast<menus::RadioNode>(node); n)
    {
      add_menu(n->get_text(),
               command,
               WindowsAppletWindow::MENU_FLAG_TOGGLE | (popup ? WindowsAppletWindow::MENU_FLAG_POPUP : 0)
                 | ((n->is_checked() ? WindowsAppletWindow::MENU_FLAG_SELECTED : 0)));
    }

  else if (auto n = std::dynamic_pointer_cast<menus::SeparatorNode>(node); n)
    {
      // not supported
    }

  else
    {
    }
}

void
WindowsAppletWindow::add_menu(const std::string &text, short cmd, int flags)
{
  AppletMenuItemData *d = &menu_data.items[menu_data.num_items++];
  d->command = cmd;
  strcpy(d->text, text.c_str());
  d->flags = flags;
}
