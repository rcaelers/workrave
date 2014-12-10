/*
 * Copyright (C) 2014 Rob Caelers <robc@krandor.nl>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __WORKRAVE_TIMERBOX_CONTROL_H__
#define __WORKRAVE_TIMERBOX_CONTROL_H__

#include <glib-object.h>

#include "timerbox.h"

#define WORKRAVE_TIMERBOX_CONTROL_TYPE                  (workrave_timerbox_control_get_type())
#define WORKRAVE_TIMERBOX_CONTROL(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj), WORKRAVE_TIMERBOX_CONTROL_TYPE, WorkraveTimerboxControl))
#define WORKRAVE_IS_TIMERBOX_CONTROL(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj), WORKRAVE_TIMERBOX_CONTROL_TYPE))
#define WORKRAVE_TIMERBOX_CONTROL_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass), WORKRAVE_TIMERBOX_CONTROL_TYPE, WorkraveTimerboxControlClass))
#define WORKRAVE_IS_TIMERBOX_CONTROL_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), WORKRAVE_TIMERBOX_CONTROL_TYPE))
#define WORKRAVE_TIMERBOX_CONTROL_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj), WORKRAVE_TIMERBOX_CONTROL_TYPE, WorkraveTimerboxControlClass))

typedef struct _WorkraveTimerboxControl        WorkraveTimerboxControl;
typedef struct _WorkraveTimerboxControlClass   WorkraveTimerboxControlClass;
typedef struct _WorkraveTimerboxControlPrivate WorkraveTimerboxControlPrivate;

struct _WorkraveTimerboxControl
{
  GObject parent_instance;

  /*< private >*/
  WorkraveTimerboxControlPrivate *priv;
};

struct _WorkraveTimerboxControlClass
{
  GObjectClass parent_class;
};

GType workrave_timerbox_control_get_type(void);

/*
 * Method definitions.
 */

/* TODO: add gobject introspection */

GtkImage *workrave_timerbox_control_get_image(WorkraveTimerboxControl *self);
WorkraveTimerbox *workrave_timerbox_control_get_timerbox(WorkraveTimerboxControl *self);
GDBusProxy *workrave_timerbox_control_get_applet_proxy(WorkraveTimerboxControl *self);
GDBusProxy *workrave_timerbox_control_get_core_proxy(WorkraveTimerboxControl *self);
GDBusProxy *workrave_timerbox_control_get_control_proxy(WorkraveTimerboxControl *self);
void workrave_timerbox_control_show_tray_icon_when_not_running(WorkraveTimerboxControl *self, gboolean show);

#endif /* __WORKRAVE_TIMERBOX_CONTROL_H__ */
