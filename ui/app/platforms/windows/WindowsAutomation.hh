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

#ifndef WORKRAVE_WINDOWS_AUTOMATION_HH
#define WORKRAVE_WINDOWS_AUTOMATION_HH

#include "WindowsResources.hh"

#include <uiautomation.h>
#include <wrl/client.h>
#include <chrono>
#include <span>
#include <stdexcept>
#include <utility>

namespace workrave::windows
{
  class CachedAutomationElements;
  class UiAutomation;

  class CachedAutomationElement
  {
  public:
    int process_id() const
    {
      int result = 0;
      check_hresult(element->get_CachedProcessId(&result), "Read cached automation process ID");
      return result;
    }

    bool is_offscreen() const
    {
      BOOL result = TRUE;
      check_hresult(element->get_CachedIsOffscreen(&result), "Read cached automation visibility");
      return result != FALSE;
    }

    CONTROLTYPEID control_type() const
    {
      CONTROLTYPEID result = 0;
      check_hresult(element->get_CachedControlType(&result), "Read cached automation control type");
      return result;
    }

    RECT bounds() const
    {
      RECT result{};
      check_hresult(element->get_CachedBoundingRectangle(&result), "Read cached automation bounds");
      return result;
    }

  private:
    friend class CachedAutomationElements;
    explicit CachedAutomationElement(Microsoft::WRL::ComPtr<IUIAutomationElement> element)
      : element(std::move(element))
    {
    }

    Microsoft::WRL::ComPtr<IUIAutomationElement> element;
  };

  class CachedAutomationElements
  {
  public:
    int size() const
    {
      int result = 0;
      check_hresult(elements->get_Length(&result), "Read cached automation element count");
      return result;
    }

    CachedAutomationElement at(int index) const
    {
      Microsoft::WRL::ComPtr<IUIAutomationElement> element;
      check_hresult(elements->GetElement(index, &element), "Read cached automation element");
      return CachedAutomationElement(std::move(element));
    }

  private:
    friend class UiAutomation;
    explicit CachedAutomationElements(Microsoft::WRL::ComPtr<IUIAutomationElementArray> elements)
      : elements(std::move(elements))
    {
    }

    Microsoft::WRL::ComPtr<IUIAutomationElementArray> elements;
  };

  // Apartment-bound; no native interface needs to escape into application code.
  class UiAutomation
  {
  public:
    UiAutomation()
    {
      check_hresult(CoCreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&automation)),
                    "Create UI Automation");
    }

    void set_timeouts(std::chrono::milliseconds timeout) const
    {
      if (!std::in_range<DWORD>(timeout.count()))
        {
          throw std::out_of_range("UI Automation timeout exceeds the DWORD range");
        }
      Microsoft::WRL::ComPtr<IUIAutomation2> automation2;
      const auto result = automation.As(&automation2);
      if (result == E_NOINTERFACE)
        {
          return;
        }
      check_hresult(result, "Query UI Automation timeouts");
      const auto milliseconds = static_cast<DWORD>(timeout.count());
      check_hresult(automation2->put_ConnectionTimeout(milliseconds), "Set UI Automation connection timeout");
      check_hresult(automation2->put_TransactionTimeout(milliseconds), "Set UI Automation transaction timeout");
    }

    CachedAutomationElements cached_descendants(HWND window, std::span<const PROPERTYID> properties) const
    {
      Microsoft::WRL::ComPtr<IUIAutomationElement> root;
      Microsoft::WRL::ComPtr<IUIAutomationCondition> condition;
      Microsoft::WRL::ComPtr<IUIAutomationCacheRequest> cache;
      Microsoft::WRL::ComPtr<IUIAutomationElementArray> elements;
      check_hresult(automation->ElementFromHandle(window, &root), "Find automation window");
      check_hresult(automation->CreateTrueCondition(&condition), "Create automation condition");
      check_hresult(automation->CreateCacheRequest(&cache), "Create automation cache");
      for (const auto property: properties)
        {
          check_hresult(cache->AddProperty(property), "Add cached automation property");
        }
      check_hresult(root->FindAllBuildCache(TreeScope_Descendants, condition.Get(), cache.Get(), &elements),
                    "Read cached automation descendants");
      return CachedAutomationElements(std::move(elements));
    }

  private:
    Microsoft::WRL::ComPtr<IUIAutomation> automation;
  };
} // namespace workrave::windows

#endif
