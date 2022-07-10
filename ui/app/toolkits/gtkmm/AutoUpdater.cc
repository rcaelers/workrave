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

#include "AutoUpdater.hh"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "unfold/Unfold.hh"

#include <gtkmm.h>

// class AutoUpdater::Private
// {
// public:
//   Private() = default;

// public:
//   std::shared_ptr<unfold::Unfold> updater;
// };

AutoUpdater::AutoUpdater()
{
  // priv = std::make_unique<Private>();

  unfold::coro::IOContext io_context{1};
  updater = unfold::Unfold::create(io_context);

  auto rc = updater->set_appcast("https://snapshots.workrave.org/snapshots/v1.11/testappcast.xml");
  if (!rc)
    {
      spdlog::info("Invalid appcast URL");
      return;
    }

  rc = updater->set_signature_verification_key("MCowBQYDK2VwAyEA0vkFT/GcU/NEM9xoDqhiYK3/EaTXVAI95MOt+SnjCpM=");
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
}

// unfold::coro::gtask<void>
// AutoUpdater::check_for_updates(std::shared_ptr<unfold::Unfold> updater)
// {
//   auto update_available = co_await updater->check_for_updates();
//   if (!update_available)
//     {
//       co_return;
//     }

//   if (update_available.value())
//     {
//       auto update_info = updater->get_update_info();
//       auto *dlg = new UpdateDialog(update_info);
//       dlg->show();
//       dlg->signal_response().connect([app](int response) {
//         spdlog::info("User response: {} ", response);
//         app->quit();
//       });

//       // TODO: leak
//     }
// }
