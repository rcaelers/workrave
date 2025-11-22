/* Copyright (C) 2011 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_APPLET_COMMON_TIMEBAR_H
#define WORKRAVE_APPLET_COMMON_TIMEBAR_H

#include <glib-object.h>
#include <gtk/gtk.h>

#define WORKRAVE_TYPE_TIMEBAR (workrave_timebar_get_type())
#define WORKRAVE_TIMEBAR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), WORKRAVE_TYPE_TIMEBAR, WorkraveTimebar))
#define WORKRAVE_IS_TIMEBAR(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), WORKRAVE_TYPE_TIMEBAR))
#define WORKRAVE_TIMEBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), WORKRAVE_TYPE_TIMEBAR, WorkraveTimebarClass))
#define WORKRAVE_IS_TIMEBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WORKRAVE_TYPE_TIMEBAR))
#define WORKRAVE_TIMEBAR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), WORKRAVE_TYPE_TIMEBAR, WorkraveTimebarClass))

typedef struct _WorkraveTimebar WorkraveTimebar;
typedef struct _WorkraveTimebarClass WorkraveTimebarClass;
typedef struct _WorkraveTimebarPrivate WorkraveTimebarPrivate;

typedef enum WorkraveColorId
{
  COLOR_ID_ACTIVE = 0,
  COLOR_ID_INACTIVE,
  COLOR_ID_OVERDUE,
  COLOR_ID_1_ACTIVE_DURING_BREAK,
  COLOR_ID_2_ACTIVE_DURING_BREAK,
  COLOR_ID_INACTIVE_OVER_ACTIVE,
  COLOR_ID_INACTIVE_OVER_OVERDUE,
  COLOR_ID_BG,
  COLOR_ID_SIZEOF
} WorkraveColorId;

struct _WorkraveTimebar
{
  GObject parent_instance;
};

struct _WorkraveTimebarClass
{
  GObjectClass parent_class;
};

GType workrave_timebar_get_type(void);

/*
 * Method definitions.
 */

void workrave_timebar_do_action(WorkraveTimebar *self, int param);
void workrave_timebar_draw(WorkraveTimebar *self, cairo_t *cr);

void workrave_timebar_set_progress(WorkraveTimebar *self, int value, int max_value, WorkraveColorId color);
void workrave_timebar_set_secondary_progress(WorkraveTimebar *self, int value, int max_value, WorkraveColorId color);
void workrave_timebar_set_text(WorkraveTimebar *self, const gchar *text);
void workrave_timebar_set_text_alignment(WorkraveTimebar *self, int align);
void workrave_timebar_get_dimensions(WorkraveTimebar *self, int *width, int *height);
void workrave_timebar_set_dimensions(WorkraveTimebar *self, int width, int height);

#endif /* WORKRAVE_APPLET_COMMON_TIMEBAR_H_ */
