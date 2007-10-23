// DBus.cc --- The main controller
//
// Copyright (C) 2006, 2007 Rob Caelers <robc@krandor.nl>
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "DBus.hh"
#include "workrave-server-bindings.h"

#include "CoreFactory.hh"
#include "ICore.hh"
#include "IConfigurator.hh"
#include "IApp.hh"

#include "Core.hh"

using namespace workrave;

//! Signals
enum {
  MICROBREAK,
  RESTBREAK,
  DAILYLIMIT,
  LAST_SIGNAL
};

G_DEFINE_TYPE (WorkraveService, workrave_service, G_TYPE_OBJECT);
static guint signals[LAST_SIGNAL] = { 0 };

/* The main service objects */
static WorkraveService *the_service = NULL;
static DBusGConnection *connection = NULL;

using namespace std;
using namespace workrave;

/* -----------------------------------------------------------------------------
 * PUBLIC METHODS
 */

WorkraveService*
workrave_service_new()
{
  return (WorkraveService *)g_object_new(WORKRAVE_TYPE_SERVICE, NULL);
}


void
workrave_dbus_server_init(Core *core)
{
  DBusGProxy *driver_proxy;
  GError *err = NULL;
  guint request_name_result;

  g_return_if_fail(connection == NULL);
  g_return_if_fail(the_service == NULL);

  connection = dbus_g_bus_get(DBUS_BUS_SESSION, &err);
  if (connection == NULL)
    {
      g_warning ("DBUS Service registration failed: %s", err ? err->message : "");
      g_error_free(err);
      return;
    }

  dbus_connection_set_exit_on_disconnect(dbus_g_connection_get_connection(connection), FALSE);

  driver_proxy = dbus_g_proxy_new_for_name(connection,
                                           DBUS_SERVICE_DBUS,
                                           DBUS_PATH_DBUS,
                                           DBUS_INTERFACE_DBUS);

  char *name = DBUS_SERVICE_WORKRAVE;
  char *env = getenv("WORKRAVE_DBUS_NAME");
  if (env != NULL)
    {
      name = env;
    }

  if (!org_freedesktop_DBus_request_name(driver_proxy,
                                         name,
                                         0,
                                         &request_name_result,
                                         &err))
    {
      g_warning ("DBUS Service name request failed.");
      g_clear_error (&err);
    }

  if (request_name_result == DBUS_REQUEST_NAME_REPLY_EXISTS)
    {
      g_warning ("DBUS Service already started elsewhere");
      return;
    }

  dbus_g_object_type_install_info(WORKRAVE_TYPE_SERVICE,
                                  &dbus_glib_workrave_object_info);

  the_service = (WorkraveService *)g_object_new(WORKRAVE_TYPE_SERVICE, NULL);
  the_service->core = core;

  dbus_g_connection_register_g_object(connection,
                                      "/org/workrave/Workrave",
                                      G_OBJECT(the_service));
}

void
workrave_dbus_server_cleanup ()
{
    /* The DBUS Glib bindings (apparently) note when this goes
       away and unregister it for incoming calls */
    if (the_service)
        g_object_unref (the_service);
    the_service = NULL;

    if (connection)
        dbus_g_connection_unref (connection);
    connection = NULL;
}


/* -----------------------------------------------------------------------------
 * HELPERS
 */


/* -----------------------------------------------------------------------------
 * DBUS METHODS
 */

gboolean
workrave_core_set_operation_mode(WorkraveService *svc, gchar *mode, GError **error)
{
  (void) svc;
  OperationMode coremode = OPERATION_MODE_NORMAL;

  if (mode == NULL)
    {
      g_set_error(error, WORKRAVE_DBUS_ERROR, 0, "Invalid operation mode: (null)");
      return FALSE;
    }

  if (strcmp(mode, "suspended") == 0)
    {
      coremode = OPERATION_MODE_SUSPENDED;
    }
  else if (strcmp(mode, "quiet") == 0)
    {
      coremode = OPERATION_MODE_QUIET;
    }
  else if (strcmp(mode, "normal") == 0)
    {
      coremode = OPERATION_MODE_NORMAL;
    }
  else
    {
      g_set_error(error, WORKRAVE_DBUS_ERROR, 0, "Invalid operation mode: %s", mode);
      return FALSE;
    }

  the_service->core->set_operation_mode(coremode);

  return TRUE;
}


gboolean
workrave_core_get_operation_mode(WorkraveService *svc, gchar **mode, GError **error)
{
  (void) svc;
  (void) error;

  OperationMode coremode = the_service->core->get_operation_mode();
  char *ret = NULL;

  switch(coremode)
    {
    case OPERATION_MODE_NORMAL:
      ret = g_strdup("normal");
      break;
    case OPERATION_MODE_QUIET:
      ret = g_strdup("quiet");
      break;
    case OPERATION_MODE_SUSPENDED:
      ret = g_strdup("suspended");
      break;
    default:
      break;
    }

  if (ret != NULL)
    {
      *mode = ret;
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

gboolean
workrave_core_report_activity(WorkraveService *svc, gchar *who, gboolean act, GError **error)
{
  (void) svc;
  (void) who;
  (void) act;
  (void) error;

  Core::get_instance()->report_external_activity(who, act);

  return TRUE;
}

gboolean
workrave_core_is_timer_running(WorkraveService *svc, guint id, gboolean *value, GError **error)
{
  (void) svc;
  (void) error;

  if (id > BREAK_ID_SIZEOF)
    {
      g_set_error(error, WORKRAVE_DBUS_ERROR, 0, "Invalid timer: %d", id);
      return FALSE;
    }

  Timer *timer = svc->core->get_timer((BreakId)id);
  *value = timer->get_state() == STATE_RUNNING;

  return TRUE;
}

gboolean
workrave_core_get_timer_idle(WorkraveService *svc, guint id, guint *value, GError **error)
{
  (void) svc;
  (void) error;

  if (id > BREAK_ID_SIZEOF)
    {
      g_set_error(error, WORKRAVE_DBUS_ERROR, 0, "Invalid timer: %d", id);
      return FALSE;
    }

  Timer *timer = svc->core->get_timer((BreakId)id);
  *value = timer->get_elapsed_idle_time();

  return TRUE;
}

gboolean
workrave_core_get_timer_elapsed(WorkraveService *svc, guint id, guint *value, GError **error)
{
  (void) svc;
  (void) error;

  if (id > BREAK_ID_SIZEOF)
    {
      g_set_error(error, WORKRAVE_DBUS_ERROR, 0, "Invalid timer: %d", id);
      return FALSE;
    }

  Timer *timer = svc->core->get_timer((BreakId)id);
  *value = timer->get_elapsed_time();

  return TRUE;
}

gboolean
workrave_core_get_time(WorkraveService *svc, guint *value, GError **error)
{
  (void) svc;
  (void) error;

  *value = svc->core->get_time();

  return TRUE;
}

gboolean
workrave_core_is_active(WorkraveService *svc, gboolean *active, GError **error)
{
  (void) svc;
  (void) error;

  *active = svc->core->get_current_monitor_state() == ACTIVITY_ACTIVE;

  return TRUE;
}


gboolean workrave_config_set_string(WorkraveService *svc, gchar *key, gchar *value, GError **error)
{
  (void) svc;
  (void) error;

  IConfigurator *config = CoreFactory::get_configurator();

  string s = value;
  if (!config->set_value(key, s, CONFIG_FLAG_IMMEDIATE))
    {
      g_set_error(error, WORKRAVE_DBUS_ERROR, 0, "Cannot set key %s", key);
      return FALSE;
    }
  return TRUE;
}

gboolean workrave_config_set_int   (WorkraveService *svc, gchar *key, int value, GError **error)
{
  (void) svc;
  (void) error;

  IConfigurator *config = CoreFactory::get_configurator();

  if (!config->set_value(key, value, CONFIG_FLAG_IMMEDIATE))
    {
      g_set_error(error, WORKRAVE_DBUS_ERROR, 0, "Cannot set key %s", key);
      return FALSE;
    }
  return TRUE;
}

gboolean workrave_config_set_bool  (WorkraveService *svc, gchar *key, bool value, GError **error)
{
  (void) svc;
  (void) error;

  IConfigurator *config = CoreFactory::get_configurator();

  if (!config->set_value(key, value, CONFIG_FLAG_IMMEDIATE))
    {
      g_set_error(error, WORKRAVE_DBUS_ERROR, 0, "Cannot set key %s", key);
      return FALSE;
    }
  return TRUE;
}

gboolean workrave_config_set_double(WorkraveService *svc, gchar *key, double value, GError **error)
{
  (void) svc;
  (void) error;

  IConfigurator *config = CoreFactory::get_configurator();

  if (!config->set_value(key, value, CONFIG_FLAG_IMMEDIATE))
    {
      g_set_error(error, WORKRAVE_DBUS_ERROR, 0, "Cannot set key %s", key);
      return FALSE;
    }
  return TRUE;
}

gboolean workrave_config_get_string(WorkraveService *svc, gchar *key, gchar **value, GError **error)
{
  (void) svc;
  (void) error;

  IConfigurator *config = CoreFactory::get_configurator();
  string s;

  if (!config->get_value(key, s))
    {
      g_set_error(error, WORKRAVE_DBUS_ERROR, 0, "Key not set %s", key);
      return FALSE;
    }

  *value = g_strdup(s.c_str());
  return TRUE;
}

gboolean workrave_config_get_int   (WorkraveService *svc, gchar *key, int *value, GError **error)
{
  (void) svc;
  (void) error;

  IConfigurator *config = CoreFactory::get_configurator();

  if (!config->get_value(key, *value))
    {
      g_set_error(error, WORKRAVE_DBUS_ERROR, 0, "Key not set %s", key);
      return FALSE;
    }

  return TRUE;
}

gboolean workrave_config_get_bool  (WorkraveService *svc, gchar *key, bool *value, GError **error)
{
  (void) svc;
  (void) error;

  IConfigurator *config = CoreFactory::get_configurator();

  if (!config->get_value(key, *value))
    {
      g_set_error(error, WORKRAVE_DBUS_ERROR, 0, "Key not set %s", key);
      return FALSE;
    }

  return TRUE;
}

gboolean workrave_config_get_double(WorkraveService *svc, gchar *key, double *value, GError **error)
{
  (void) svc;
  (void) error;

  IConfigurator *config = CoreFactory::get_configurator();

  if (!config->get_value(key, *value))
    {
      g_set_error(error, WORKRAVE_DBUS_ERROR, 0, "Key not set %s", key);
      return FALSE;
    }

  return TRUE;
}


/* -----------------------------------------------------------------------------
 * DBUS SIGNALS
 */

void
workrave_core_send_break_stage_signal(BreakId break_id, const gchar *progress)
{
  switch (break_id)
    {
    case BREAK_ID_MICRO_BREAK:
      g_signal_emit(WORKRAVE_SERVICE(the_service), signals[MICROBREAK], 0, progress);
      break;
    case BREAK_ID_REST_BREAK:
      g_signal_emit(WORKRAVE_SERVICE(the_service), signals[RESTBREAK], 0, progress);
      break;
    case BREAK_ID_DAILY_LIMIT:
      g_signal_emit(WORKRAVE_SERVICE(the_service), signals[DAILYLIMIT], 0, progress);
      break;
    default:
      break;
    }
}


/* -----------------------------------------------------------------------------
 * OBJECT
 */

static void
workrave_service_init(WorkraveService *svc)
{
  (void) svc;
}


static void
workrave_service_class_init(WorkraveServiceClass *klass)
{
  GObjectClass *gclass;

  gclass = G_OBJECT_CLASS(klass);

  signals[MICROBREAK] =
    g_signal_new ("microbreak_changed", WORKRAVE_TYPE_SERVICE,
                  (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED),
                  G_STRUCT_OFFSET (WorkraveServiceClass, microbreak),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE,
                  1,
                  (GSignalFlags)G_TYPE_STRING);

  signals[RESTBREAK] =
    g_signal_new ("restbreak_changed", WORKRAVE_TYPE_SERVICE,
                  (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED),
                  G_STRUCT_OFFSET (WorkraveServiceClass, restbreak),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_STRING);

  signals[DAILYLIMIT] =
    g_signal_new ("dailylimit_changed", WORKRAVE_TYPE_SERVICE,
                  (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED),
                  G_STRUCT_OFFSET (WorkraveServiceClass, dailylimit),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_STRING);
}
