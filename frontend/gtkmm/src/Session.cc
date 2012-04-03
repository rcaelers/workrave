// Session.cc --- Monitor the user session
//
// Copyright (C) 2010, 2011 Rob Caelers <robc@krandor.nl>
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Session.hh"

#include "nls.h"
#include "debug.hh"

#include "IConfigurator.hh"
#include "GUIConfig.hh"
#include "CoreFactory.hh"
#include "IBreak.hh"

using namespace workrave;
using namespace std;

Session::Session()
  : is_idle(false),
    taking(false)
{
}


void
Session::init()
{
#if defined(HAVE_DBUSGLIB_GET_PRIVATE) || defined(HAVE_DBUS_GIO)
  init_gnome();
#endif
}

void
Session::set_idle(bool new_idle)
{
  TRACE_ENTER_MSG("Session::set_idle", new_idle);

  bool auto_natural = false;
  IConfigurator *config = CoreFactory::get_configurator();
  config->get_value(GUIConfig::CFG_KEY_BREAK_AUTO_NATURAL % BREAK_ID_REST_BREAK, auto_natural);
  ICore *core = CoreFactory::get_core();

  if (core->get_usage_mode() == USAGE_MODE_READING)
    {
      core->force_idle();
    }

  if (auto_natural)
    {
      TRACE_MSG("Automatic natural break enabled");
      if (new_idle && !is_idle)
        {
          TRACE_MSG("Now idle");
          IBreak *rest_break = core->get_break(BREAK_ID_REST_BREAK);

          taking = rest_break->is_taking();
          TRACE_MSG("taking " << taking);
          if (!taking)
            {
              core->set_operation_mode_override( OPERATION_MODE_SUSPENDED, "screensaver" );
            }
        }
      else if (!new_idle && is_idle && !taking)
        {
          TRACE_MSG("No longer idle");
          IBreak *rest_break = core->get_break(BREAK_ID_REST_BREAK);

          core->remove_operation_mode_override( "screensaver" );
          if (core->get_operation_mode() == OPERATION_MODE_NORMAL &&
              rest_break->get_elapsed_idle_time() < rest_break->get_auto_reset()
              && rest_break->is_enabled()
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
  (void) proxy;
  (void) sender_name;
  
  Session *self = (Session *)user_data;
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
	GError *error = NULL;

  GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                                    G_DBUS_PROXY_FLAGS_NONE,
                                                    NULL,
                                                    "org.gnome.SessionManager",
                                                    "/org/gnome/SessionManager/Presence",
                                                    "org.gnome.SessionManager.Presence",
                                                    NULL,
                                                    &error);

  if (error != NULL)
    {
      TRACE_MSG("Error: " << error->message);
      g_error_free(error);
    }

  if (error == NULL && proxy != NULL)
    {
      g_signal_connect(proxy, "g-signal", G_CALLBACK(on_signal), this);
    }
}

#elif defined(HAVE_DBUSGLIB_GET_PRIVATE)

static void
status_changed_cb(DBusGProxy *proxy, int session_status, void *data)
{
  TRACE_ENTER_MSG("status_changed_cb", session_status);
  (void) proxy;
  Session *self = (Session *)data;

  self->set_idle(session_status == 3);

  TRACE_EXIT();
}

void
Session::init_gnome()
{
  DBusGProxy *proxy;
  GError *err = NULL;

  connection = dbus_g_bus_get_private(DBUS_BUS_SESSION, NULL, &err);
  if (connection == NULL)
    {
      g_warning("DBUS session bus not available: %s", err ? err->message : "");
      g_error_free(err);
      return;
    }

  proxy = dbus_g_proxy_new_for_name(connection,
                                    "org.gnome.SessionManager",
                                    "/org/gnome/SessionManager/Presence",
                                    "org.gnome.SessionManager.Presence");

  if (proxy != NULL)
    {
      dbus_g_proxy_add_signal(proxy, "StatusChanged", G_TYPE_UINT, G_TYPE_INVALID);
      dbus_g_proxy_connect_signal(proxy, "StatusChanged", G_CALLBACK(status_changed_cb), this, NULL);
    }
}

#endif
