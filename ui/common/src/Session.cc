// Session.cc --- Monitor the user session
//
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

#include "Session.hh"

#include "debug.hh"

#include "config/IConfigurator.hh"
#include "commonui/GUIConfig.hh"
#include "commonui/Backend.hh"
#include "core/IBreak.hh"

using namespace workrave;
using namespace workrave::config;
using namespace std;

void
Session::init()
{
#if defined(HAVE_DBUS_GIO)
  init_gnome();
#endif
}

void
Session::set_idle(bool new_idle)
{
  TRACE_ENTER_MSG("Session::set_idle", new_idle);

  bool auto_natural = GUIConfig::break_auto_natural(BREAK_ID_REST_BREAK)();
  ICore::Ptr core = Backend::get_core();

  if (core->get_usage_mode() == UsageMode::Reading)
    {
      core->force_idle();
    }

  if (new_idle && !is_idle)
    {
      TRACE_MSG("Now idle");
      IBreak::Ptr rest_break = core->get_break(BREAK_ID_REST_BREAK);

      taking = rest_break->is_taking();
      TRACE_MSG("taking " << taking);
      if (!taking)
        {
          core->set_operation_mode_override(OperationMode::Suspended, "screensaver");
        }
    }
  else if (!new_idle && is_idle && !taking)
    {
      TRACE_MSG("No longer idle");
      core->remove_operation_mode_override("screensaver");

      if (auto_natural)
        {
          TRACE_MSG("Automatic natural break enabled");

          IBreak::Ptr rest_break = core->get_break(BREAK_ID_REST_BREAK);

          if (core->get_operation_mode() == OperationMode::Normal
              && rest_break->get_elapsed_idle_time() < rest_break->get_auto_reset() && rest_break->is_enabled()
              && !rest_break->is_taking())
            {
              bool overdue = (rest_break->get_limit() < rest_break->get_elapsed_time());

              if (overdue)
                {
                  core->force_break(BREAK_ID_REST_BREAK, BREAK_HINT_NONE);
                }
              else
                {
                  core->force_break(BREAK_ID_REST_BREAK, BREAK_HINT_NATURAL_BREAK);
                }
            }
        }
    }

  is_idle = new_idle;
  TRACE_EXIT();
}

#if defined(HAVE_DBUS_GIO)

void
Session::on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data)
{
  (void)proxy;
  (void)sender_name;

  auto *self = static_cast<Session *>(user_data);
  int session_status;

  if (g_strcmp0(signal_name, "StatusChanged") == 0)
    {
      g_variant_get(parameters, "(u)", &session_status);
      self->set_idle(session_status == 3);
    }
}

void
Session::init_gnome()
{
  TRACE_ENTER("Session::init_gnome");
  GError *error = nullptr;

  GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                                    G_DBUS_PROXY_FLAGS_NONE,
                                                    nullptr,
                                                    "org.gnome.SessionManager",
                                                    "/org/gnome/SessionManager/Presence",
                                                    "org.gnome.SessionManager.Presence",
                                                    nullptr,
                                                    &error);

  if (error != nullptr)
    {
      TRACE_MSG("Error: " << error->message);
      g_error_free(error);
    }

  if (error == nullptr && proxy != nullptr)
    {
      g_signal_connect(proxy, "g-signal", G_CALLBACK(on_signal), this);
    }
}

#endif
