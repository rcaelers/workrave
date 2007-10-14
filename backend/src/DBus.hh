// DBus.hh --- The main controller
//
// Copyright (C) 2006, 2007 Rob Caelers
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
// $Id$
//

#ifndef WRDBUS_H
#define WRDBUS_H

#define DBUS_SERVICE_WORKRAVE      "org.workrave.Workrave"

#include <glib/gerror.h>
#include <glib-object.h>

#include "ICore.hh"

using namespace workrave;

class Core;

G_BEGIN_DECLS

#define WORKRAVE_DBUS_ERROR  g_quark_from_static_string ("workrave")

typedef struct _WorkraveService WorkraveService;
typedef struct _WorkraveServiceClass WorkraveServiceClass;

#define WORKRAVE_TYPE_SERVICE               (workrave_service_get_type ())
#define WORKRAVE_SERVICE(object)            (G_TYPE_CHECK_INSTANCE_CAST((object), WORKRAVE_TYPE_SERVICE, WorkraveService))
#define WORKRAVE_SERVICE_CLASS(klass)       (G_TYPE_CHACK_CLASS_CAST((klass), WORKRAVE_TYPE_SERVICE, WorkraveServiceClass))
#define WORKRAVE_IS_SERVICE(object)         (G_TYPE_CHECK_INSTANCE_TYPE((object), WORKRAVE_TYPE_SERVICE))
#define WORKRAVE_IS_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE((klass), WORKRAVE_TYPE_SERVICE))
#define WORKRAVE_SERVICE_GET_CLASS(object)  (G_TYPE_INSTANCE_GET_CLASS((object), WORKRAVE_TYPE_SERVICE, WorkraveServiceClass))

struct _WorkraveService {
  GObject base;

  /* <public> */
  Core *core;
};

struct _WorkraveServiceClass {
  GObjectClass base;

  void (*microbreak)(WorkraveService *svc, const gchar *progress);
  void (*restbreak)(WorkraveService *svc, const gchar *progress);
  void (*dailylimit)(WorkraveService *svc, const gchar *progress);
};

void workrave_dbus_server_init(Core *core);
GType workrave_service_get_type(void);


gboolean workrave_core_set_operation_mode(
    WorkraveService *svc,
    gchar *mode,
    GError **error);

gboolean workrave_core_get_operation_mode(
    WorkraveService *svc,
    gchar **mode,
    GError **error);

gboolean workrave_core_report_activity(
    WorkraveService *svc,
    gchar *who,
    gboolean act,
    GError **error);

gboolean workrave_network_listen(
    WorkraveService *svc,
    guint port,
    GError **error);

gboolean workrave_network_connect(
    WorkraveService *svc,
    gchar *host,
    guint port,
    gchar **link_id,
    GError **error);

gboolean workrave_network_disconnect(
    WorkraveService *svc,
    gchar *linkid,
    GError **error);

gboolean workrave_debug_quit(
    WorkraveService *svc,
    GError **error);

gboolean workrave_debug_fake_monitor(
    WorkraveService *svc,
    guint state,
    GError **error);

gboolean workrave_debug_get_remote_state(
    WorkraveService *svc,
    guint *state,
    GError **error);

gboolean workrave_debug_tick(
    WorkraveService *svc,
    guint ticks,
    GError **error);

gboolean workrave_debug_set_time(
    WorkraveService *svc,
    guint time,
    GError **error);

gboolean workrave_debug_get_activity_history(
  WorkraveService *svc,
  GPtrArray  **history,
  GError **error);

gboolean workrave_debug_get_settings_history(
  WorkraveService *svc,
  GPtrArray  **history,
  GError **error);

void workrave_core_send_break_stage_signal(
    BreakId break_id,
    const gchar *progress);

gboolean workrave_core_is_timer_running(
    WorkraveService *svc,
    guint id,
    gboolean *value,
    GError **error);

gboolean workrave_core_get_timer_elapsed(
    WorkraveService *svc,
    guint id,
    guint *value,
    GError **error);

gboolean workrave_core_get_timer_idle(
    WorkraveService *svc,
    guint id,
    guint *value,
    GError **error);

gboolean workrave_core_get_time(
    WorkraveService *svc,
    guint *value,
    GError **error);

gboolean workrave_core_is_active(
    WorkraveService *svc,
    gboolean *active,
    GError **error);

gboolean workrave_config_set_string(WorkraveService *svc, gchar *value, gchar *value, GError **error);
gboolean workrave_config_set_int   (WorkraveService *svc, gchar *value, int value, GError **error);
gboolean workrave_config_set_bool  (WorkraveService *svc, gchar *value, bool value, GError **error);
gboolean workrave_config_set_double(WorkraveService *svc, gchar *value, double value, GError **error);

gboolean workrave_config_get_string(WorkraveService *svc, gchar *value, gchar **value, GError **error);
gboolean workrave_config_get_int   (WorkraveService *svc, gchar *value, int *value, GError **error);
gboolean workrave_config_get_bool  (WorkraveService *svc, gchar *value, bool *value, GError **error);
gboolean workrave_config_get_double(WorkraveService *svc, gchar *value, double *value, GError **error);

G_END_DECLS

#endif // WRDBUS_H
