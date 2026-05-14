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
#include <string.h>

#include <boost/archive/binary_oarchive.hpp>
#include <sstream>

#include "commonui/Text.hh"
#include "Applet.hh"

#include "ui/TimerBoxControl.hh"
#include "debug.hh"
#include "ui/windows/IToolkitWindows.hh"

using namespace workrave;

#if defined(interface)
#  undef interface
#endif
#undef slots

WindowsAppletWindow::WindowsAppletWindow(std::shared_ptr<IPluginContext> context)
  : context(context)
  , menu_model(context->get_menu_model())
  , menu_helper(menu_model)
  , apphold(context->get_toolkit())
{
  TRACE_ENTRY();

  memset(&local_heartbeat_data, 0, sizeof(AppletHeartbeatData));
  memset(&heartbeat_data, 0, sizeof(AppletHeartbeatData));

  heartbeat_data.enabled = true;

  ::InitializeCriticalSection(&heartbeat_data_lock);
  thread_abort_event = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
  heartbeat_data_event = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);

  control = new TimerBoxControl(context->get_core(), "applet", this);

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
  init_toolkit();
}

WindowsAppletWindow::~WindowsAppletWindow()
{
  TRACE_ENTRY();
  /* before this instance is destroyed we signal and wait for its worker thread to terminate. this
  isn't ideal because the gui will be blocked while we wait for termination if this destructor is
  called from the main thread. current conditions are acceptable, however. 2/12/2012
  */
  heartbeat_data.enabled = false;
  SetEvent(thread_abort_event);
  if (thread_handle != nullptr)
    {
      WaitForSingleObject(thread_handle, INFINITE);
      CloseHandle(thread_handle);
    }

  if (thread_abort_event != nullptr)
    {
      CloseHandle(thread_abort_event);
    }

  if (heartbeat_data_event != nullptr)
    {
      CloseHandle(heartbeat_data_event);
    }

  DeleteCriticalSection(&heartbeat_data_lock);

  delete control;
}

static HWND
RecursiveFindWindow(HWND hwnd, LPCSTR lpClassName)
{
  static char buf[80];
  int num = GetClassNameA(hwnd, buf, sizeof(buf) - 1);
  buf[num] = 0;
  HWND ret = nullptr;

  if (!stricmp(lpClassName, buf))
    {
      ret = hwnd;
    }
  else
    {
      HWND child = FindWindowEx(hwnd, nullptr, nullptr, nullptr);
      while (child != nullptr)
        {
          ret = RecursiveFindWindow(child, lpClassName);
          if (ret)
            {
              break;
            }
          child = FindWindowEx(hwnd, child, nullptr, nullptr);
        }
    }
  return ret;
}

void
WindowsAppletWindow::set_slot(workrave::BreakId id, int slot)
{
  TRACE_ENTRY_PAR(id, slot);
  heartbeat_data.slots[slot] = (short)id;
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
  TRACE_ENTRY_PAR(int(id), value);
  strncpy(heartbeat_data.bar_text[id], Text::time_to_string(value).c_str(), APPLET_BAR_TEXT_MAX_LENGTH - 1);
  heartbeat_data.bar_text[id][APPLET_BAR_TEXT_MAX_LENGTH - 1] = '\0';
  heartbeat_data.bar_primary_color[id] = (int)primary_color;
  heartbeat_data.bar_primary_val[id] = primary_val;
  heartbeat_data.bar_primary_max[id] = primary_max;
  heartbeat_data.bar_secondary_color[id] = (int)secondary_color;
  heartbeat_data.bar_secondary_val[id] = secondary_val;
  heartbeat_data.bar_secondary_max[id] = secondary_max;
}

void
WindowsAppletWindow::update_view()
{
  TRACE_ENTRY();
  BOOL entered = ::TryEnterCriticalSection(&heartbeat_data_lock);
  if (entered)
    {
      update_applet_window();

      if (applet_window != nullptr)
        {
          local_applet_window = applet_window;
          memcpy(&local_heartbeat_data, &heartbeat_data, sizeof(AppletHeartbeatData));
          SetEvent(heartbeat_data_event);
        }

      ::LeaveCriticalSection(&heartbeat_data_lock);
    }
}

void
WindowsAppletWindow::send_menu()
{
  TRACE_ENTRY();
  auto toolkit_win = std::dynamic_pointer_cast<IToolkitWindows>(context->get_toolkit());
  if (toolkit_win && local_applet_window != nullptr && !menu_sent) // RACE?
    {
      AppletMenuData data;
      data.command_window = HandleToLong(toolkit_win->get_event_hwnd());
      init_menu_list(data.items, menu_model->get_root());

      std::ostringstream ss;
      boost::archive::binary_oarchive ar(ss);
      ar << data;
      std::string serialized_data = ss.str();

      COPYDATASTRUCT msg;
      msg.dwData = APPLET_MESSAGE_MENU;
      msg.cbData = serialized_data.size();
      msg.lpData = (LPVOID)serialized_data.data();
      SendMessage(local_applet_window, WM_COPYDATA, 0, (LPARAM)&msg);
      menu_sent = true;
    }
}

void
WindowsAppletWindow::send_time_bars()
{
  TRACE_ENTRY();
  if (local_applet_window != nullptr)
    {
      COPYDATASTRUCT msg;
      msg.dwData = APPLET_MESSAGE_HEARTBEAT;
      msg.cbData = sizeof(AppletHeartbeatData);
      msg.lpData = &local_heartbeat_data;
      for (size_t i = 0; i < workrave::BREAK_ID_SIZEOF; i++)
        {
          TRACE_MSG("sending: slots[]= {}", local_heartbeat_data.slots[i]);
        }
      SendMessage(local_applet_window, WM_COPYDATA, 0, (LPARAM)&msg);
    }
}

void
WindowsAppletWindow::update_applet_window()
{
  TRACE_ENTRY();
  HWND previous_applet_window = applet_window;
  if (applet_window == nullptr || !IsWindow(applet_window))
    {
      HWND taskbar = FindWindowA("Shell_TrayWnd", nullptr);
      applet_window = RecursiveFindWindow(taskbar, APPLET_WINDOW_CLASS_NAME);
      menu_sent = false;
    }

  if (previous_applet_window == nullptr && applet_window != nullptr)
    {
      apphold.hold();
    }
  else if (previous_applet_window != nullptr && applet_window == nullptr)
    {
      apphold.release();
    }
}

void
WindowsAppletWindow::init_thread()
{
  TRACE_ENTRY();
  DWORD thread_exit_code = 0;

  if (thread_id && thread_handle && GetExitCodeThread(thread_handle, &thread_exit_code) && (thread_exit_code == STILL_ACTIVE))
    {
      return;
    }

  if (!thread_id)
    {
      // if there is no id but a handle then this instance's worker thread has exited or is exiting.
      if (thread_handle)
        {
          CloseHandle(thread_handle);
        }

      thread_id = 0;
      SetLastError(0);
      thread_handle = (HANDLE)_beginthreadex(nullptr, 0, run_event_pipe_static, this, 0, (unsigned int *)&thread_id);

      if (!thread_handle || !thread_id)
        {
          TRACE_MSG("Thread could not be created. GetLastError : {}", GetLastError());
        }
    }
}

unsigned __stdcall WindowsAppletWindow::run_event_pipe_static(void *param)
{
  TRACE_ENTRY();
  auto *pThis = (WindowsAppletWindow *)param;
  pThis->run_event_pipe();
  // invalidate the id to signal the thread is exiting
  pThis->thread_id = 0;
  return (DWORD)0;
}

void
WindowsAppletWindow::run_event_pipe()
{
  const DWORD current_thread_id = GetCurrentThreadId();
  TRACE_ENTRY_PAR(current_thread_id);

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
        {
          break;
        }

      if (heartbeat_data.enabled && (wait_result == (WAIT_OBJECT_0 + 1)))
        {
          EnterCriticalSection(&heartbeat_data_lock);

          send_time_bars();
          send_menu();
          local_applet_window = nullptr;

          LeaveCriticalSection(&heartbeat_data_lock);
        }
    }
}

void
WindowsAppletWindow::set_icon(OperationModeIcon icon)
{
}

bool
WindowsAppletWindow::on_applet_command(int command)
{
  TRACE_ENTRY_PAR(command);
  auto node = menu_helper.find_node(command);
  if (node)
    {
      node->activate();
    }
  return false;
}

bool
WindowsAppletWindow::filter_func(MSG *msg)
{
  TRACE_ENTRY();
  bool ret = true;

  switch (msg->message)
    {
    case WM_USER:
      {
        int cmd = (int)msg->wParam;
        context->get_toolkit()->create_oneshot_timer(0, [this, cmd]() { on_applet_command(cmd); });
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
  return ret;
}

bool
WindowsAppletWindow::is_visible() const
{
  return applet_window != nullptr && heartbeat_data.enabled;
}

void
WindowsAppletWindow::init_toolkit()
{
  TRACE_ENTRY();

  workrave::utils::connect(context->get_toolkit()->signal_timer(), this, [this]() { control->update(); });
  auto toolkit_win = std::dynamic_pointer_cast<IToolkitWindows>(context->get_toolkit());
  if (toolkit_win)
    {
      workrave::utils::connect(toolkit_win->hook_event(), this, [this](MSG *msg) { return filter_func(msg); });
    }
}

void
WindowsAppletWindow::init_menu()
{
  menu_sent = false;
}

void
WindowsAppletWindow::init_menu_list(std::list<AppletMenuItem> &items, menus::Node::Ptr node)
{
  uint32_t command = menu_helper.allocate_command(node->get_id());

  uint8_t flags = MENU_ITEM_FLAG_NONE;

  if (node->is_visible())
    {
      flags |= MENU_ITEM_FLAG_VISIBLE;
    }

  if (auto n = std::dynamic_pointer_cast<menus::SubMenuNode>(node); n)
    {
      bool add_sub_menu = !items.empty();

      if (add_sub_menu)
        {
          items.emplace_back(n->get_text(), node->get_dynamic_text(), node->get_id(), command, MenuItemType::SubMenuBegin, flags);
        }
      for (auto &menu_to_add: n->get_children())
        {
          init_menu_list(items, menu_to_add);
        }
      if (add_sub_menu)
        {
          items.emplace_back(n->get_text(), node->get_dynamic_text(), node->get_id(), command, MenuItemType::SubMenuEnd, flags);
        }
    }

  else if (auto n = std::dynamic_pointer_cast<menus::SectionNode>(node); n)
    {
      for (auto &menu_to_add: n->get_children())
        {
          init_menu_list(items, menu_to_add);
        }
    }

  else if (auto n = std::dynamic_pointer_cast<menus::RadioGroupNode>(node); n)
    {
      items.emplace_back(n->get_text(), node->get_dynamic_text(), node->get_id(), command, MenuItemType::RadioGroupBegin, flags);
      for (auto &menu_to_add: n->get_children())
        {
          init_menu_list(items, menu_to_add);
        }
      items.emplace_back(n->get_text(), node->get_dynamic_text(), node->get_id(), command, MenuItemType::RadioGroupEnd, flags);
    }

  else if (auto n = std::dynamic_pointer_cast<menus::ActionNode>(node); n)
    {
      items.emplace_back(n->get_text(), node->get_dynamic_text(), node->get_id(), command, MenuItemType::Action, flags);
    }

  else if (auto n = std::dynamic_pointer_cast<menus::ToggleNode>(node); n)
    {
      if (n->is_checked())
        {
          flags |= MENU_ITEM_FLAG_ACTIVE;
        }
      items.emplace_back(n->get_text(), node->get_dynamic_text(), node->get_id(), command, MenuItemType::Check, flags);
    }

  else if (auto n = std::dynamic_pointer_cast<menus::RadioNode>(node); n)
    {
      if (n->is_checked())
        {
          flags |= MENU_ITEM_FLAG_ACTIVE;
        }
      items.emplace_back(n->get_text(), node->get_dynamic_text(), node->get_id(), command, MenuItemType::Radio, flags);
    }

  else if (auto n = std::dynamic_pointer_cast<menus::SeparatorNode>(node); n)
    {
      items.emplace_back(n->get_text(), node->get_dynamic_text(), node->get_id(), command, MenuItemType::Separator, flags);
    }
}
