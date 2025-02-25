// Copyright (C) 2010, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#include "GnomeSession.hh"
#include "GtkUtil.hh"

#include "debug.hh"

enum GsmPresenceStatus
{
  GSM_PRESENCE_STATUS_AVAILABLE = 0,
  GSM_PRESENCE_STATUS_INVISIBLE,
  GSM_PRESENCE_STATUS_BUSY,
  GSM_PRESENCE_STATUS_IDLE,
};

GnomeSession::GnomeSession(std::shared_ptr<IPluginContext> context)
  : context(context)
{
  TRACE_ENTRY();
  init();
}

GnomeSession::~GnomeSession()
{
  TRACE_ENTRY();
}

void
GnomeSession::init()
{
  try
    {
      proxy = Gio::DBus::Proxy::create_for_bus_sync(
#if GLIBMM_CHECK_VERSION(2, 68, 0)
        Gio::DBus::BusType::SESSION,
#else
        Gio::DBus::BUS_TYPE_SESSION,
#endif
        "org.gnome.SessionManager",
        "/org/gnome/SessionManager/Presence",
        "org.gnome.SessionManager.Presence");

      proxy->signal_signal().connect(sigc::mem_fun(*this, &GnomeSession::on_signal));
    }
#if GLIBMM_CHECK_VERSION(2, 68, 0)
  catch (std::exception &e)
    {
      std::cerr << e.what() << std::endl;
    }
#else
  catch (const Glib::Exception &e)
    {
      spdlog::warn(std::string("Failed to subscribe to presence events from Gnome SessionManager:") + std::string(e.what()));
    }
#endif
}

void
GnomeSession::on_signal(const Glib::ustring &sender, const Glib::ustring &signal_name, const Glib::VariantContainerBase &params)
{
  try
    {
      if (signal_name == "StatusChanged")
        {
          Glib::Variant<uint32_t> session_status;
          params.get_child(session_status);
          context->get_toolkit()->signal_session_idle_changed()(session_status.get() == GSM_PRESENCE_STATUS_IDLE);
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
      spdlog::warn(std::string("Failed to process presence event from Gnome SessionManager:") + std::string(e.what()));
    }
#endif
}
