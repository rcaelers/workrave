/* Copyright (C) 2014 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_APPLET_COMMON_TIMERBOX_CONTROL_H
#define WORKRAVE_APPLET_COMMON_TIMERBOX_CONTROL_H

#include <glib-object.h>

#include "timerbox.h"

#define WORKRAVE_TIMERBOX_CONTROL_TYPE (workrave_timerbox_control_get_type())
#define WORKRAVE_TIMERBOX_CONTROL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), WORKRAVE_TIMERBOX_CONTROL_TYPE, WorkraveTimerboxControl))
#define WORKRAVE_IS_TIMERBOX_CONTROL(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), WORKRAVE_TIMERBOX_CONTROL_TYPE))
#define WORKRAVE_TIMERBOX_CONTROL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), WORKRAVE_TIMERBOX_CONTROL_TYPE, WorkraveTimerboxControlClass))
#define WORKRAVE_IS_TIMERBOX_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WORKRAVE_TIMERBOX_CONTROL_TYPE))
#define WORKRAVE_TIMERBOX_CONTROL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), WORKRAVE_TIMERBOX_CONTROL_TYPE, WorkraveTimerboxControlClass))

typedef struct _WorkraveTimerboxControl WorkraveTimerboxControl;
typedef struct _WorkraveTimerboxControlClass WorkraveTimerboxControlClass;
typedef struct _WorkraveTimerboxControlPrivate WorkraveTimerboxControlPrivate;

struct _WorkraveTimerboxControl
{
  GObject parent_instance;
};

struct _WorkraveTimerboxControlClass
{
  GObjectClass parent_class;
};

GType workrave_timerbox_control_get_type(void);

enum WorkraveTimerboxControlTrayIconMode
{
  WORKRAVE_TIMERBOX_CONTROL_TRAY_ICON_MODE_ALWAYS,
  WORKRAVE_TIMERBOX_CONTROL_TRAY_ICON_MODE_NEVER,
  WORKRAVE_TIMERBOX_CONTROL_TRAY_ICON_MODE_FOLLOW,
};

/*
 * Method definitions.
 */

GtkImage *workrave_timerbox_control_get_image(WorkraveTimerboxControl *self);
WorkraveTimerbox *workrave_timerbox_control_get_timerbox(WorkraveTimerboxControl *self);

GDBusProxy *workrave_timerbox_control_get_applet_proxy(WorkraveTimerboxControl *self);
GDBusProxy *workrave_timerbox_control_get_core_proxy(WorkraveTimerboxControl *self);
GDBusProxy *workrave_timerbox_control_get_control_proxy(WorkraveTimerboxControl *self);

void workrave_timerbox_control_set_tray_icon_mode(WorkraveTimerboxControl *self, enum WorkraveTimerboxControlTrayIconMode mode);
void workrave_timerbox_control_set_tray_icon_visible_when_not_running(WorkraveTimerboxControl *self, gboolean show);

GVariant *workrave_timerbox_control_get_menus(WorkraveTimerboxControl *self);

#endif /* WORKRAVE_APPLET_COMMON_TIMERBOX_CONTROL_H */
