// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
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

#include "TaskbarLayout.hh"
#include "WindowsAutomation.hh"

#include <array>
#include <spdlog/spdlog.h>
#include <utility>

using namespace workrave::windows;

namespace
{
  bool is_taskbar_control(CONTROLTYPEID type)
  {
    return type == UIA_ButtonControlTypeId || type == UIA_ListItemControlTypeId || type == UIA_MenuItemControlTypeId
           || type == UIA_EditControlTypeId || type == UIA_CheckBoxControlTypeId || type == UIA_RadioButtonControlTypeId;
  }

  TaskbarLayout::Snapshot measure(const UiAutomation &automation)
  {
    TaskbarLayout::Snapshot result;
    HWND taskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (taskbar == nullptr || IsWindowVisible(taskbar) == FALSE)
      {
        return {};
      }
    result.bounds = get_window_rect(taskbar);
    if (result.bounds.right - result.bounds.left <= result.bounds.bottom - result.bounds.top)
      {
        return {};
      }

    constexpr std::array<PROPERTYID, 4> cached_properties{
      UIA_ProcessIdPropertyId,
      UIA_IsOffscreenPropertyId,
      UIA_ControlTypePropertyId,
      UIA_BoundingRectanglePropertyId,
    };
    const auto elements = automation.cached_descendants(taskbar, cached_properties);
    const auto length = elements.size();
    if (length < 0 || length > 2048)
      {
        return {};
      }
    bool found_controls = false;
    for (int i = 0; i < length; ++i)
      {
        const auto element = elements.at(i);
        // Our own applet and tooltip must not become obstacles on the next scan.
        if (std::cmp_equal(element.process_id(), GetCurrentProcessId()))
          {
            continue;
          }
        if (element.is_offscreen() || !is_taskbar_control(element.control_type()))
          {
            continue;
          }
        const auto rect = element.bounds();
        if (rect.bottom > result.bounds.top && rect.top < result.bounds.bottom && rect.right > rect.left)
          {
            result.occupied.push_back({.left = rect.left, .right = rect.right});
            found_controls = true;
          }
      }
    // Also protect the entire notification area (clock, overflow, system icons).
    HWND notify = FindWindowExW(taskbar, nullptr, L"TrayNotifyWnd", nullptr);
    if (!found_controls || notify == nullptr)
      {
        return {};
      }
    const auto tray = get_window_rect(notify);
    if (tray.right <= tray.left)
      {
        return {};
      }
    result.occupied.push_back({.left = tray.left, .right = result.bounds.right});
    result.taskbar = taskbar;
    result.measured = std::chrono::steady_clock::now();
    return result;
  }
} // namespace

TaskbarLayout::TaskbarLayout()
  : worker([this] { run(); })
{
}

TaskbarLayout::~TaskbarLayout()
{
  {
    std::scoped_lock lock(mutex);
    stopping = true;
  }
  wake.notify_one();
  worker.join();
}

TaskbarLayout::Snapshot
TaskbarLayout::snapshot()
{
  std::scoped_lock lock(mutex);
  return latest;
}

void
TaskbarLayout::run()
{
  // UI Automation always reports physical coordinates.
  SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  try
    {
      ComApartment apartment(COINIT_MULTITHREADED);
      UiAutomation automation;
      // Bound slow/unresponsive provider calls where the OS supports it.
      automation.set_timeouts(std::chrono::seconds(1));
      while (true)
        {
          Snapshot next;
          try
            {
              next = measure(automation);
            }
          catch (const std::exception &error)
            {
              // Keep the main UI alive and publish an unavailable snapshot.
              spdlog::warn("Cannot measure taskbar layout: {}", error.what());
            }
          catch (...)
            {
              spdlog::warn("Cannot measure taskbar layout: unknown error");
            }
          std::unique_lock lock(mutex);
          latest = std::move(next);
          if (wake.wait_for(lock, std::chrono::seconds(1), [this] { return stopping; }))
            {
              break;
            }
        }
    }
  catch (const std::exception &error)
    {
      spdlog::warn("Taskbar layout worker failed: {}", error.what());
    }
  catch (...)
    {
      spdlog::warn("Taskbar layout worker failed: unknown error");
    }
}
