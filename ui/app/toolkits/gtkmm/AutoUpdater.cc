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

#include <memory>
#include <utility>
#include <boost/asio.hpp>

#include "AutoUpdater.hh"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "commonui/nls.h"
#include "debug.hh"
#include "unfold/Unfold.hh"
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
  if (!rc)
    {
      spdlog::info("Invalid appcast URL");
      return;
    }

  rc = updater->set_signature_verification_key("MCowBQYDK2VwAyEAZ1I+iYYFpFMPcSj15BnHl6x7uow2CdxT0t2BmUzMGXk=");

  if (!rc)
    {
      spdlog::info("Invalid signature key");
      return;
    }

  rc = updater->set_current_version(WORKRAVE_VERSION);
  if (!rc)
    {
      spdlog::info("Invalid version");
      return;
    }

  updater->set_configuration_prefix("Software\\Workrave");

  init_channels();

  updater->set_update_available_callback(
    [&]() -> boost::asio::awaitable<unfold::UpdateResponse> { return on_update_available(); });

  updater->set_periodic_update_check_interval(std::chrono::hours{24});
  updater->set_periodic_update_check_enabled(workrave::updater::Config::enabled()());

  init_preferences();
  init_menu();
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
      spdlog::info("Invalid allowed channels");
      return;
    }
}

void
AutoUpdater::init_preferences()
{
  std::vector<std::string> channels{"Stable", "Release Candidate", "Beta"};

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
                             ->when(&workrave::updater::Config::enabled()));

  context->get_preferences_registry()->add_page("auto-update", N_("Software updates"), "update-symbolic");
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
        response = unfold::UpdateResponse::Install;
        break;

      case AutoUpdateDialog::UpdateChoice::Later:
        response = unfold::UpdateResponse::Later;
        break;

      case AutoUpdateDialog::UpdateChoice::Skip:
        response = unfold::UpdateResponse::Skip;
        break;
      }
    dialog->close();
    if (dialog_handler)
      {
        (*dialog_handler)(response);
        dialog_handler.reset();
      }
  });
  dialog->show();
  dialog->raise();
}

boost::asio::awaitable<unfold::UpdateResponse>
AutoUpdater::on_update_available()
{
  spdlog::info("Update available");

  if (dialog_handler)
    {
      spdlog::info("Update dialog already open");
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
              spdlog::info("Check for update failed");
            }
        }
      catch (std::exception &e)
        {
          spdlog::info("Exception in on_check_for_update: {}", e.what());
        }
    },
    boost::asio::detached);
}
