// Copyright (C) 2022 Rob Caelers <rob.caelers@gmail.com>
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
#include "utils/Signals.hh"

class AutoUpdateDialog;

class AutoUpdater
  : public Plugin<AutoUpdater>
  , public workrave::utils::Trackable
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

  void set_status(unfold::outcome::std_result<void> status);
  void set_status(std::string status);

private:
  using handler_type = boost::asio::async_result<std::decay<decltype(boost::asio::use_awaitable)>::type,
                                                 void(unfold::UpdateResponse)>::handler_type;

  std::shared_ptr<IPluginContext> context;
  unfold::coro::IOContext io_context;
  unfold::coro::glib::scheduler scheduler;
  std::shared_ptr<unfold::Unfold> updater;
  std::shared_ptr<AutoUpdateDialog> dialog;
  std::optional<handler_type> dialog_handler;

  std::shared_ptr<ui::prefwidgets::Def> auto_update_def;
  double progress{0.0};

  using sv = std::string_view;
  static constexpr std::string_view CHECK_FOR_UPDATE = sv("workrave.check_for_updates");
  std::shared_ptr<spdlog::logger> logger{workrave::utils::Logging::create("updater")};
};

#endif // AUTO_UPDATER_HH
