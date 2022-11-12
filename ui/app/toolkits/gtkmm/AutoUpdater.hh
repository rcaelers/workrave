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

#ifndef AUTO_UPDATER_HH
#define AUTO_UPDATER_HH

#include <string>

#include <type_traits>

#include "ui/Plugin.hh"

#include "unfold/Unfold.hh"
#include "unfold/coro/gtask.hh"
#include "unfold/coro/IOContext.hh"

#include "ui/prefwidgets/Widgets.hh"

#include "utils/Logging.hh"

class AutoUpdateDialog;

class AutoUpdater : public Plugin<AutoUpdater>
{
public:
  explicit AutoUpdater(std::shared_ptr<IPluginContext> context);
  ~AutoUpdater() override;

  std::string get_plugin_id() const override
  {
    return "workrave.AutoUpdater";
  }

private:
  void init_channels();
  void init_preferences();
  void init_menu();
  boost::asio::awaitable<unfold::UpdateResponse> on_update_available();
  void on_check_for_update();
  unfold::coro::gtask<void> show_update();

private:
  using handler_type = boost::asio::async_result<std::decay<decltype(boost::asio::use_awaitable)>::type,
                                                 void(unfold::UpdateResponse)>::handler_type;

  std::shared_ptr<IPluginContext> context;
  unfold::coro::IOContext io_context;
  unfold::coro::glib::scheduler scheduler;
  std::shared_ptr<unfold::Unfold> updater;
  std::shared_ptr<AutoUpdateDialog> dialog;
  std::optional<handler_type> dialog_handler;

  std::shared_ptr<ui::prefwidgets::Frame> auto_update_def;

  using sv = std::string_view;
  static constexpr std::string_view CHECK_FOR_UPDATE = sv("workrave.check_for_updates");
  std::shared_ptr<spdlog::logger> logger{workrave::utils::Logging::create("updater")};
};

#endif // AUTO_UPDATER_HH
