// Copyright (C) 2024 Rob Caelers <robc@krandor.nl>
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

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "LoginSession.hh"
#include "GtkUtil.hh"

#include "debug.hh"

LoginSession::LoginSession(std::shared_ptr<IPluginContext> context)
  : context(context)
{
  TRACE_ENTRY();
  init();
}

LoginSession::~LoginSession()
{
  TRACE_ENTRY();
}

void
LoginSession::init()
{
  try
    {
      proxy = Gio::DBus::Proxy::create_for_bus_sync(
#if GLIBMM_CHECK_VERSION(2, 68, 0)
        Gio::DBus::BusType::SYSTEM,
#else
        Gio::DBus::BUS_TYPE_SYSTEM,
#endif
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager");

      proxy->signal_signal().connect(sigc::mem_fun(*this, &LoginSession::on_signal));
    }
#if GLIBMM_CHECK_VERSION(2, 68, 0)
  catch (std::exception &e)
    {
      std::cerr << e.what() << std::endl;
    }
#else
  catch (const Glib::Exception &e)
    {
      spdlog::warn(std::string("Failed to subscribe to events from Login manager") + std::string(e.what()));
    }
#endif
}

void
LoginSession::on_signal(const Glib::ustring &sender, const Glib::ustring &signal_name, const Glib::VariantContainerBase &params)
{
  try
    {
      if (signal_name == "PrepareForSleep")
        {
          Glib::Variant<bool> start;
          params.get_child(start);
          auto core = context->get_core();
          core->set_powersave(start.get());
        }
    }
#if GLIBMM_CHECK_VERSION(2, 68, 0)
  catch (std::exception &e)
    {
      std::cerr << e.what() << std::endl;
    }
#else
  catch (const Glib::Exception &e)
    {
      spdlog::warn(std::string("Failed to process event from Login Manager:") + std::string(e.what()));
    }
#endif
}
