// DBus.hh --- The main controller
//
// Copyright (C) 2006 Rob Caelers
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef WRDBUS_H
#define WRDBUS_H

#define DBUS_SERVICE_WORKRAVE      "org.workrave.Workrave"

#include <glib/gerror.h>
#include <glib-object.h>

#include "CoreInterface.hh"

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
};

struct _WorkraveServiceClass {
  GObjectClass base;

  void (*microbreak)(WorkraveService *svc, const gchar *progress);
  void (*restbreak)(WorkraveService *svc, const gchar *progress);
  void (*dailylimit)(WorkraveService *svc, const gchar *progress);
};

void workrave_dbus_server_init(CoreInterface *core);
GType workrave_service_get_type(void);

gboolean workrave_service_set_operation_mode(WorkraveService *svc, gchar *mode, GError **error);
gboolean workrave_service_get_operation_mode(WorkraveService *svc, gchar **mode, GError **error);
gboolean workrave_service_report_activity(WorkraveService *svc, gchar *who, gchar *act, GError **error);
 
void workrave_service_send_break_stage_signal(BreakId break_id, gchar *progress);

G_END_DECLS

#endif // WRDBUS_H
