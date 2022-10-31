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

#ifndef EDGE_HH
#define EDGE_HH

#include <string>
#include <windows.h>
#include <wrl/client.h>

#include "WebView2.h"

#if defined(PLATFORM_OS_WINDOWS)
#  pragma push_macro("ERROR")
#  pragma push_macro("IN")
#  pragma push_macro("OUT")
#  pragma push_macro("WINDING")
#  undef ERROR
#  undef IN
#  undef OUT
#  undef WINDING
#endif

#include <gtkmm.h>

#if defined(PLATFORM_OS_WINDOWS)
#  pragma pop_macro("ERROR")
#  pragma pop_macro("IN")
#  pragma pop_macro("OUT")
#  pragma pop_macro("WINDING")
#endif

class Edge : public Gtk::DrawingArea
{
public:
  Edge() = default;

  void set_content(const std::string &content);

  static bool is_supported();

private:
  void init();
  void update_bounds();
  void update_settings();

  HRESULT on_environment_created(HRESULT result, ICoreWebView2Environment *environment);
  HRESULT on_controller_created(HRESULT result, ICoreWebView2Controller *controller);

  void on_size_allocate(Gtk::Allocation &allocation) override;
  void on_realize() override;

private:
  HWND hwnd_{};
  std::string pending_content_;
  Microsoft::WRL::ComPtr<ICoreWebView2> webview_;
  Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller_;
};

#endif
