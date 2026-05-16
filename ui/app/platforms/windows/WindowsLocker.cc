// Copyright (C) 2001 - 2021 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "ui/windows/WindowsLocker.hh"

#include <algorithm>
#include <string>
#include <windows.h>

#include <spdlog/spdlog.h>

#include "debug.hh"

namespace
{
  auto get_window_title(HWND window) -> std::string
  {
    int length = GetWindowTextLengthA(window);
    std::string text(static_cast<size_t>(length) + 1, '\0');
    int copied = GetWindowTextA(window, text.data(), static_cast<int>(text.size()));
    text.resize(static_cast<size_t>(std::max(copied, 0)));
    return text;
  }
} // namespace

class Hook
{
public:
  void enable();
  void disable();

  static Hook *instance();

  Hook(const Hook &other) = delete;
  Hook &operator=(const Hook &other) = delete;
  Hook(Hook &&other) noexcept = delete;
  Hook &operator=(Hook &&other) noexcept = delete;

private:
  Hook() = default;
  ~Hook();

  static LRESULT CALLBACK hook_callback(INT nCode, WPARAM wParam, LPARAM lParam);

  HHOOK hook = nullptr;
};

Hook::~Hook()
{
  disable();
}

Hook *
Hook::instance()
{
  static Hook *the_instance = nullptr;

  if (the_instance == nullptr)
    {
      the_instance = new Hook();
    }
  return the_instance;
}

void
Hook::enable()
{
  if (hook == nullptr)
    {
      hook = SetWindowsHookEx(WH_KEYBOARD_LL, hook_callback, GetModuleHandle(nullptr), 0);
      if (hook == nullptr)
        {
          spdlog::warn("Failed to install Windows break keyboard hook: {}", GetLastError());
        }
    }
}

void
Hook::disable()
{
  if (hook != nullptr)
    {
      UnhookWindowsHookEx(hook);
      hook = nullptr;
    }
}

LRESULT CALLBACK
Hook::hook_callback(INT nCode, WPARAM wParam, LPARAM lParam)
{
  Hook *self = Hook::instance();

  bool handled = false;

  if (nCode == HC_ACTION)
    {
      auto *data = (KBDLLHOOKSTRUCT *)lParam;

      // bool is_key_down = ((wParam == WM_KEYDOWN) || (wParam == WM_SYSKEYDOWN));

      BOOL ctrl_down = GetAsyncKeyState(VK_CONTROL) >> ((sizeof(SHORT) * 8) - 1);

      if ((data->vkCode == VK_ESCAPE && ctrl_down) ||                   // Ctrl+Esc
          (data->vkCode == VK_TAB && data->flags & LLKHF_ALTDOWN) ||    // Alt+TAB
          (data->vkCode == VK_ESCAPE && data->flags & LLKHF_ALTDOWN) || // Alt+Esc
          (data->vkCode == VK_LWIN || data->vkCode == VK_RWIN))         // Start Menu
        {
          handled = true;
        }
    }

  return (handled ? TRUE : CallNextHookEx(self->hook, nCode, wParam, lParam));
}

WindowsLocker::WindowsLocker()
{
}

bool
WindowsLocker::can_lock()
{
  return false;
}

void
WindowsLocker::prepare_lock()
{
  active_window = GetForegroundWindow();

  if (active_window != nullptr)
    {
      std::string text = get_window_title(active_window);
      TRACE_MSG("Save active window: {}", text);
      spdlog::info("Save active window: {} {}", text, reinterpret_cast<intptr_t>(active_window));
    }
}

void
WindowsLocker::lock()
{
  Hook::instance()->enable();

  UINT previous_state = 0;
  SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, TRUE, &previous_state, 0);
}

void
WindowsLocker::unlock()
{
  UINT previous_state = 0;
  SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, FALSE, &previous_state, 0);
  Hook::instance()->disable();

  if (active_window != nullptr)
    {
      std::string text = get_window_title(active_window);
      TRACE_MSG("Restore active window: {}", text);
      spdlog::info("Restore active window: {} {}", text, reinterpret_cast<intptr_t>(active_window));
      SetForegroundWindow(active_window);
      active_window = nullptr;
    }
}
