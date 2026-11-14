// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Rob Caelers

#include "TaskbarLayout.hh"

#include <uiautomation.h>
#include <wrl/client.h>
#include <spdlog/spdlog.h>
#include <utility>

using Microsoft::WRL::ComPtr;

namespace
{
  bool is_taskbar_control(CONTROLTYPEID type)
  {
    return type == UIA_ButtonControlTypeId || type == UIA_ListItemControlTypeId || type == UIA_MenuItemControlTypeId
           || type == UIA_EditControlTypeId || type == UIA_CheckBoxControlTypeId || type == UIA_RadioButtonControlTypeId;
  }

  TaskbarLayout::Snapshot measure(IUIAutomation *automation)
  {
    TaskbarLayout::Snapshot result;
    HWND taskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (taskbar == nullptr || IsWindowVisible(taskbar) == FALSE || GetWindowRect(taskbar, &result.bounds) == FALSE
        || result.bounds.right - result.bounds.left <= result.bounds.bottom - result.bounds.top)
      {
        return {};
      }

    ComPtr<IUIAutomationElement> root;
    ComPtr<IUIAutomationCondition> condition;
    ComPtr<IUIAutomationElementArray> elements;
    ComPtr<IUIAutomationCacheRequest> cache;
    if (FAILED(automation->ElementFromHandle(taskbar, &root)) || FAILED(automation->CreateTrueCondition(&condition))
        || FAILED(automation->CreateCacheRequest(&cache)) || FAILED(cache->AddProperty(UIA_ProcessIdPropertyId))
        || FAILED(cache->AddProperty(UIA_IsOffscreenPropertyId)) || FAILED(cache->AddProperty(UIA_ControlTypePropertyId))
        || FAILED(cache->AddProperty(UIA_BoundingRectanglePropertyId))
        || FAILED(root->FindAllBuildCache(TreeScope_Descendants, condition.Get(), cache.Get(), &elements)))
      {
        return {};
      }

    int length = 0;
    if (FAILED(elements->get_Length(&length)) || length > 2048)
      {
        return {};
      }
    bool found_controls = false;
    for (int i = 0; i < length; ++i)
      {
        ComPtr<IUIAutomationElement> element;
        CONTROLTYPEID type = 0;
        int process = 0;
        BOOL offscreen = TRUE;
        RECT rect{};
        if (FAILED(elements->GetElement(i, &element)) || FAILED(element->get_CachedProcessId(&process)))
          {
            return {};
          }
        // Our own applet and tooltip must not become obstacles on the next scan.
        if (std::cmp_equal(process, GetCurrentProcessId()))
          {
            continue;
          }
        if (FAILED(element->get_CachedIsOffscreen(&offscreen)) || FAILED(element->get_CachedControlType(&type)))
          {
            return {};
          }
        if (offscreen != FALSE || !is_taskbar_control(type))
          {
            continue;
          }
        if (FAILED(element->get_CachedBoundingRectangle(&rect)))
          {
            return {};
          }
        if (rect.bottom > result.bounds.top && rect.top < result.bounds.bottom && rect.right > rect.left)
          {
            result.occupied.push_back({.left = rect.left, .right = rect.right});
            found_controls = true;
          }
      }
    // Also protect the entire notification area (clock, overflow, system icons).
    RECT tray{};
    HWND notify = FindWindowExW(taskbar, nullptr, L"TrayNotifyWnd", nullptr);
    if (!found_controls || notify == nullptr || GetWindowRect(notify, &tray) == FALSE || tray.right <= tray.left)
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
  if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
    {
      return;
    }
  {
    ComPtr<IUIAutomation> automation;
    if (SUCCEEDED(CoCreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&automation))))
      {
        // Bound slow/unresponsive provider calls where the OS supports it.
        ComPtr<IUIAutomation2> automation2;
        if (SUCCEEDED(automation.As(&automation2)))
          {
            automation2->put_ConnectionTimeout(1000);
            automation2->put_TransactionTimeout(1000);
          }
        while (true)
          {
            Snapshot next;
            try
              {
                next = measure(automation.Get());
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
  }
  CoUninitialize();
}
