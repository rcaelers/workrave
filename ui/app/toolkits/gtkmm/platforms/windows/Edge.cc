// Copyright (C) 2022 Rob Caelers <rob.caelers@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "Edge.hh"

#include <filesystem>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <shlobj.h>
#include <gdk/gdkwin32.h>

#include "callback.hh"
#include "utils/StringUtils.hh"

#if !defined(PLATFORM_OS_WINDOWS_NATIVE)
// clang-format off
__CRT_UUID_DECL(ICoreWebView2, 0x76eceacb,0x0462,0x4d94, 0xac,0x83,0x42,0x3a,0x67,0x93,0x77,0x5e);
__CRT_UUID_DECL(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler, 0x6c4819f3, 0xc9b7, 0x4260, 0x81,0x27, 0xc9,0xf5,0xbd,0xe7,0xf6,0x8c);
__CRT_UUID_DECL(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler, 0x4e8a3389, 0xc9d8, 0x4bd2, 0xb6,0xb5, 0x12,0x4f,0xee,0x6c,0xc1,0x4d);
// clang-format on
#endif

namespace
{
  std::filesystem::path get_special_folder(REFKNOWNFOLDERID folder)
  {
    PWSTR path = nullptr;
    auto hr = ::SHGetKnownFolderPath(folder, KF_FLAG_DEFAULT, nullptr, &path);
    if (SUCCEEDED(hr))
      {
        auto p = std::filesystem::path(path);
        ::CoTaskMemFree(path);
        return std::filesystem::canonical(p);
      }
    return {};
  }
} // namespace

void
Edge::set_content(const std::string &content)
{
  if (webview_ == nullptr)
    {
      pending_content_ = content;
      return;
    }

  webview_->NavigateToString(workrave::utils::utf8_to_utf16(content).c_str());
}

bool
Edge::is_supported()
{
  wchar_t *version = nullptr;
  HRESULT hr = GetAvailableCoreWebView2BrowserVersionString(nullptr, &version);
  if (FAILED(hr) || (version == nullptr))
    {
      spdlog::error("failed to obtain webview2 version ({0:x})", static_cast<unsigned int>(hr));
      return false;
    }
  spdlog::info("webview2 version: {}", workrave::utils::utf16_to_utf8(version));
  CoTaskMemFree(version);
  return true;
}

void
Edge::on_size_allocate(Gtk::Allocation &allocation)
{
  Gtk::DrawingArea::on_size_allocate(allocation);
  update_bounds();
}

void
Edge::on_realize()
{
  Gtk::DrawingArea::on_realize();

  GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(gobj()));
  hwnd_ = static_cast<HWND>(GDK_WINDOW_HWND(window));

  init();
}

void
Edge::init()
{
  auto appdata = get_special_folder(FOLDERID_LocalAppData);

  HRESULT hr = ::CreateCoreWebView2EnvironmentWithOptions(
    nullptr,
    appdata.wstring().c_str(),
    nullptr,
    Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([this](auto r, auto *e) {
      return on_environment_created(r, e);
    }).Get());

  if (FAILED(hr))
    {
      spdlog::error("failed to create webview2 environment ({0:x})", static_cast<unsigned int>(hr));
    }
}

void
Edge::update_bounds()
{
  if (webview_ != nullptr)
    {
      RECT bounds;
      GetClientRect(hwnd_, &bounds);
      controller_->put_Bounds(bounds);
    }
}

void
Edge::update_settings()
{
  if (webview_ == nullptr)
    {
      return;
    }

  ICoreWebView2Settings *settings = nullptr;
  HRESULT hr = webview_->get_Settings(&settings);
  if (FAILED(hr))
    {
      spdlog::error("failed to obtain webview2 settings ({0:x})", static_cast<unsigned int>(hr));
      return;
    }

  if (settings != nullptr)
    {
      settings->put_IsStatusBarEnabled(FALSE);
      settings->put_AreDevToolsEnabled(FALSE);
      settings->put_AreDefaultContextMenusEnabled(FALSE);
    }
}

HRESULT
Edge::on_environment_created(HRESULT result, ICoreWebView2Environment *environment)
{
  if (FAILED(result))
    {
      spdlog::error("failed to create webview2 environment ({0:x})", static_cast<unsigned int>(result));
      return result;
    }

  HRESULT hr = environment->CreateCoreWebView2Controller(
    hwnd_,
    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([this](auto r, auto *v) {
      return on_controller_created(r, v);
    }).Get());
  if (FAILED(result))
    {
      spdlog::error("failed to create webview2 controller ({0:x})", static_cast<unsigned int>(result));
      return result;
    }

  return hr;
}

HRESULT
Edge::on_controller_created(HRESULT result, ICoreWebView2Controller *controller)
{
  if (FAILED(result))
    {
      spdlog::error("failed to create webview2 controller ({0:x})", static_cast<unsigned int>(result));
      return result;
    }

  controller_ = controller;

  HRESULT hr = controller->get_CoreWebView2(&webview_);
  if (FAILED(hr))
    {
      spdlog::error("failed to obtain webview ({0:x})", static_cast<unsigned int>(result));
      return result;
    }

  update_settings();
  update_bounds();
  controller_->put_IsVisible(TRUE);

  if (!pending_content_.empty())
    {
      set_content(pending_content_);
      pending_content_.clear();
    }

  return S_OK;
}
