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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <memory>
#include <utility>
#include <boost/asio.hpp>

#include "AutoUpdater.hh"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "commonui/nls.h"
#include "debug.hh"
#include "unfold/Unfold.hh"
#include "unfold/UnfoldErrors.hh"
#include "updater/Config.hh"

#include "AutoUpdateDialog.hh"

#include <gtkmm.h>

AutoUpdater::AutoUpdater(std::shared_ptr<IPluginContext> context)
  : context(context)
  , scheduler(g_main_context_default(), io_context.get_io_context())
  , updater(unfold::Unfold::create(io_context))

{
  TRACE_ENTRY();

  workrave::updater::Config::init(context->get_configurator());

  auto rc = updater->set_appcast("https://snapshots.workrave.org/snapshots/v1.11/appcast.xml");
  rc = updater->set_appcast("https://snapshots.workrave.org/snapshots/staging/v1.11/appcast.xml");
  if (!rc)
    {
      logger->info("Invalid appcast URL");
      return;
    }

  rc = updater->set_signature_verification_key("MCowBQYDK2VwAyEAZ1I+iYYFpFMPcSj15BnHl6x7uow2CdxT0t2BmUzMGXk=");

  if (!rc)
    {
      logger->info("Invalid signature key");
      return;
    }

  rc = updater->set_current_version(WORKRAVE_VERSION);
  if (!rc)
    {
      logger->info("Invalid version");
      return;
    }

  updater->set_configuration_prefix("Software\\Workrave");

  init_channels();

  updater->set_update_available_callback(
    [&]() -> boost::asio::awaitable<unfold::UpdateResponse> { return on_update_available(); });

  updater->set_periodic_update_check_interval(std::chrono::hours{24});
  updater->set_periodic_update_check_enabled(workrave::updater::Config::enabled()());
  updater->set_proxy(workrave::updater::Config::proxy_type()());
  updater->set_custom_proxy(workrave::updater::Config::proxy()());

  init_preferences();
  init_menu();

  workrave::updater::Config::channel().connect(this, [this](auto channel) {
    logger->info("Channel changed to {}", workrave::utils::enum_to_string(channel));
    init_channels();
  });

  workrave::updater::Config::proxy_type().connect(this, [this](auto proxy_type) {
    logger->info("Proxy type changed to {}", workrave::utils::enum_to_string(proxy_type));
    updater->set_proxy(proxy_type);
  });

  workrave::updater::Config::proxy().connect(this, [this](auto proxy) {
    logger->info("Proxy changed to {}", proxy);
    updater->set_custom_proxy(proxy);
  });

  updater->set_update_status_callback([&](unfold::outcome::std_result<void> state) -> void {
    logger->info("Update status changed");
    set_error_state(state);
  });

  updater->set_download_progress_callback([this](unfold::UpdateStage stage, double p) -> void {
    this->progress = p;
    unfold::coro::gtask<void> task = [this](unfold::UpdateStage stage) -> unfold::coro::gtask<void> {
      if (dialog)
        {
          dialog->set_stage(stage, progress);
        }
      co_return;
    }(stage);
    scheduler.spawn(std::move(task));
  });
}

AutoUpdater::~AutoUpdater()
{
  TRACE_ENTRY();
}

void
AutoUpdater::init_channels()
{
  TRACE_ENTRY();

  auto channel = workrave::updater::Config::channel()();
  logger->info("Current release channel: {}", workrave::utils::enum_to_string(channel));

  std::vector<std::string> allowed_channels;

  switch (channel)
    {
    case workrave::updater::Channel::Beta:
      allowed_channels.emplace_back("beta");
      break;

    case workrave::updater::Channel::Candidate:
      allowed_channels.emplace_back("rc");
      break;

    case workrave::updater::Channel::Stable:
      allowed_channels.emplace_back("stable");
      break;
    }

  auto rc = updater->set_allowed_channels(allowed_channels);
  if (!rc)
    {
      logger->info("Invalid allowed channels");
      return;
    }
}

void
AutoUpdater::init_preferences()
{
  std::vector<std::string> channels{_("Stable"), _("Release Candidate"), _("Beta")};
  std::vector<std::string> proxy_types{_("No proxy"), _("System proxy"), _("Custom proxy")};

  auto_update_def = ui::prefwidgets::PanelDef::create("auto-update", "auto-update", N_("Software updates"))
                    << (ui::prefwidgets::Frame::create(N_("Auto update"))
                        << ui::prefwidgets::Toggle::create(N_("Automatically check for updates"))
                             ->connect(&workrave::updater::Config::enabled())
                        << ui::prefwidgets::Choice::create(N_("Release channel:"))
                             ->connect(&workrave::updater::Config::channel(),
                                       {{workrave::updater::Channel::Stable, 0},
                                        {workrave::updater::Channel::Candidate, 1},
                                        {workrave::updater::Channel::Beta, 2}})
                             ->assign(channels)
                             ->when(&workrave::updater::Config::enabled())
                        << ui::prefwidgets::Choice::create(N_("Proxy Type:"))
                             ->connect(
                               &workrave::updater::Config::proxy_type(),
                               {{unfold::ProxyType::None, 0}, {unfold::ProxyType::System, 1}, {unfold::ProxyType::Custom, 2}})
                             ->assign(proxy_types)
                             ->when(&workrave::updater::Config::enabled())
                        << ui::prefwidgets::Entry::create(N_("Proxy:"))
                             ->connect(&workrave::updater::Config::proxy())
                             ->when(&workrave::updater::Config::proxy_type(), [](unfold::ProxyType t) {
                               // auto p = workrave::updater::Config::proxy_type()();
                               return t == unfold::ProxyType::Custom;
                             }));

  context->get_preferences_registry()->add_page("auto-update", N_("Software updates"), "workrave-update-symbolic");
  context->get_preferences_registry()->add(auto_update_def);
}

void
AutoUpdater::init_menu()
{
  auto menu_model = context->get_menu_model();
  auto section = menu_model->find_section("workrave.section.tail");
  auto item = menus::ActionNode::create(CHECK_FOR_UPDATE, N_("Check for _Updates"), [this] { on_check_for_update(); });
  section->add_before(item, "workrave.about");
  menu_model->update();
}

unfold::coro::gtask<void>
AutoUpdater::show_update()
{
  auto update_info = updater->get_update_info();
  if (!update_info)
    {
      co_return;
    }

  dialog = std::make_shared<AutoUpdateDialog>(update_info, [this](auto choice) {
    auto response = unfold::UpdateResponse::Later;
    switch (choice)
      {
      case AutoUpdateDialog::UpdateChoice::Now:
        {
          response = unfold::UpdateResponse::Install;
          unfold::coro::gtask<void> task = [this]() -> unfold::coro::gtask<void> {
            if (dialog)
              {
                dialog->start_install();
              }
            co_return;
          }();
          scheduler.spawn(std::move(task));
        }
        break;

      case AutoUpdateDialog::UpdateChoice::Later:
        response = unfold::UpdateResponse::Later;
        dialog->close();
        break;

      case AutoUpdateDialog::UpdateChoice::Skip:
        response = unfold::UpdateResponse::Skip;
        dialog->close();
        break;
      }
    if (dialog_handler)
      {
        (*dialog_handler)(response);
        dialog_handler.reset();
      }
  });
  dialog->signal_hide().connect([this]() { dialog.reset(); });
  dialog->show();
  dialog->raise();
}

boost::asio::awaitable<unfold::UpdateResponse>
AutoUpdater::on_update_available()
{
  logger->info("Update available");

  if (dialog_handler)
    {
      logger->info("Update dialog already open");
      co_return unfold::UpdateResponse::Later;
    }

  auto response = co_await boost::asio::async_initiate<decltype(boost::asio::use_awaitable), void(unfold::UpdateResponse)>(
    [this](auto &&handler) {
      dialog_handler.emplace(std::forward<decltype(handler)>(handler));
      unfold::coro::gtask<void> task = show_update();
      scheduler.spawn(std::move(task));
    },
    boost::asio::use_awaitable);

  co_return response;
}

void
AutoUpdater::on_check_for_update()
{
  boost::asio::co_spawn(
    *io_context.get_io_context(),
    [&]() -> boost::asio::awaitable<void> {
      try
        {
          auto rc = co_await updater->check_for_update_and_notify();
          if (!rc)
            {
              logger->info("Check for update failed");
            }
        }
      catch (std::exception &e)
        {
          logger->info("Exception in on_check_for_update: {}", e.what());
        }
    },
    boost::asio::detached);
}

void
AutoUpdater::set_error_state(unfold::outcome::std_result<void> state)
{
  logger->info("Error state {}", state.error().message());
  auto errc = unfold::to_errc(state.error());

  if (errc)
    {
      switch (*errc)
        {
        case unfold::UnfoldErrc::Success:
          break;
        case unfold::UnfoldErrc::InvalidAppcast:
          set_status(_("Invalid appcast"));
          break;
        case unfold::UnfoldErrc::AppcastDownloadFailed:
          set_status(_("Failed to download appcast"));
          break;
        case unfold::UnfoldErrc::InstallerDownloadFailed:
          set_status(_("Failed to download installer"));
          break;
        case unfold::UnfoldErrc::InstallerVerificationFailed:
          set_status(_("Failed to validate installer integrity. Aborting upgrade"));
          break;
        case unfold::UnfoldErrc::InstallerExecutionFailed:
          set_status(_("Failed to execute installer"));
          break;
        case unfold::UnfoldErrc::InvalidArgument:
        case unfold::UnfoldErrc::InternalError:
        default:
          set_status(_("Internal failure. Aborting upgrade"));
          break;
        }
    }
  else
    {
      set_status(_("Internal failure. Aborting upgrade"));
    }
}

void
AutoUpdater::set_status(std::string status)
{
  unfold::coro::gtask<void> task = [this](std::string status) -> unfold::coro::gtask<void> {
    if (dialog)
      {
        dialog->set_status(status);
      }
    co_return;
  }(status);
  scheduler.spawn(std::move(task));
}
