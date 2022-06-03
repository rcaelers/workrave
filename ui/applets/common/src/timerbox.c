/* Copyright (C) 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "timerbox.h"

#include <gtk/gtk.h>

#include "timebar.h"

static void workrave_timerbox_class_init(WorkraveTimerboxClass *klass);
static void workrave_timerbox_init(WorkraveTimerbox *self);
static void workrave_timerbox_dispose(GObject *gobject);
static void workrave_timerbox_finalize(GObject *gobject);
static void workrave_timerbox_set_property(GObject *gobject, guint property_id, const GValue *value, GParamSpec *pspec);
static void workrave_timerbox_get_property(GObject *gobject, guint property_id, GValue *value, GParamSpec *pspec);

static void workrave_timerbox_update_sheep(WorkraveTimerbox *self, cairo_t *cr);
static void workrave_timerbox_update_time_bars(WorkraveTimerbox *self, cairo_t *cr);
static void workrave_timerbox_compute_dimensions(WorkraveTimerbox *self, int *width, int *height);
static GdkPixbuf *workrave_load_image(WorkraveTimerbox *self, const char *name);
static void workrave_timerbox_init_images(WorkraveTimerbox *self);
static void workrave_on_settings_changed(GSettings *gsettings, const gchar *key, void *user_data);

enum
{
  PROP_0,
  PROP_NAME
};

const int PADDING_X = 2;
const int PADDING_Y = 2;

struct _WorkraveTimerboxPrivate
{
  gchar *name;
  GdkPixbuf *normal_sheep_icon;
  GdkPixbuf *quiet_sheep_icon;
  GdkPixbuf *suspended_sheep_icon;
  WorkraveTimebar *slot_to_time_bar[BREAK_ID_SIZEOF];
  GdkPixbuf *break_to_icon[BREAK_ID_SIZEOF];
  WorkraveBreakId slot_to_break[BREAK_ID_SIZEOF];
  short break_to_slot[BREAK_ID_SIZEOF];
  gboolean break_visible[BREAK_ID_SIZEOF];
  gboolean enabled;
  short filled_slots;
  gboolean force_icon;
  gchar *mode;
  GSettings *settings;
};

G_DEFINE_TYPE_WITH_PRIVATE(WorkraveTimerbox, workrave_timerbox, G_TYPE_OBJECT);

static void
workrave_timerbox_class_init(WorkraveTimerboxClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GParamSpec *pspec;

  gobject_class->dispose = workrave_timerbox_dispose;
  gobject_class->finalize = workrave_timerbox_finalize;
  gobject_class->set_property = workrave_timerbox_set_property;
  gobject_class->get_property = workrave_timerbox_get_property;

  pspec = g_param_spec_string("workrave",
                              "Workrave construct prop",
                              "Set workrave's name",
                              "no-name-set" /* default value */,
                              G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_property(gobject_class, PROP_NAME, pspec);
}

static void
workrave_timerbox_init(WorkraveTimerbox *self)
{
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      priv->slot_to_time_bar[i] = g_object_new(WORKRAVE_TYPE_TIMEBAR, NULL);
      priv->break_visible[i] = FALSE;
      priv->slot_to_break[i] = BREAK_ID_NONE;
      priv->break_to_slot[i] = -1;
      priv->break_to_icon[i] = NULL;
    }
  priv->filled_slots = 0;
  priv->enabled = FALSE;
  priv->force_icon = FALSE;
  priv->mode = g_strdup("normal");
  priv->settings = g_settings_new("org.workrave.gui");
  g_signal_connect(priv->settings, "changed", G_CALLBACK(workrave_on_settings_changed), self);

  priv->normal_sheep_icon = NULL;
  priv->quiet_sheep_icon = NULL;
  priv->suspended_sheep_icon = NULL;

  workrave_timerbox_init_images(self);
}

static void
workrave_timerbox_dispose(GObject *gobject)
{
  WorkraveTimerbox *self = WORKRAVE_TIMERBOX(gobject);
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);

  g_clear_pointer(&priv->normal_sheep_icon, g_object_unref);
  g_clear_pointer(&priv->quiet_sheep_icon, g_object_unref);
  g_clear_pointer(&priv->suspended_sheep_icon, g_object_unref);

  g_clear_pointer(&priv->mode, g_free);
  g_clear_pointer(&priv->settings, g_object_unref);
  g_clear_pointer(&priv->name, g_free);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      g_object_unref(priv->break_to_icon[i]);
      g_object_unref(priv->slot_to_time_bar[i]);
    }

  /* Chain up to the parent class */
  G_OBJECT_CLASS(workrave_timerbox_parent_class)->dispose(gobject);
}

static void
workrave_timerbox_finalize(GObject *gobject)
{
  /* Chain up to the parent class */
  G_OBJECT_CLASS(workrave_timerbox_parent_class)->finalize(gobject);
}

static void
workrave_timerbox_set_property(GObject *gobject, guint property_id, const GValue *value, GParamSpec *pspec)
{
  WorkraveTimerbox *self = WORKRAVE_TIMERBOX(gobject);
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);

  switch (property_id)
    {
    case PROP_NAME:
      g_free(priv->name);
      priv->name = g_value_dup_string(value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, property_id, pspec);
      break;
    }
}

static void
workrave_timerbox_get_property(GObject *gobject, guint property_id, GValue *value, GParamSpec *pspec)
{
  WorkraveTimerbox *self = WORKRAVE_TIMERBOX(gobject);
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);

  switch (property_id)
    {
    case PROP_NAME:
      g_value_set_string(value, priv->name);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, property_id, pspec);
      break;
    }
}

static void
workrave_timerbox_update_sheep(WorkraveTimerbox *self, cairo_t *cr)
{
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);

  if ((priv->enabled && priv->filled_slots == 0) || priv->force_icon)
    {
      if (!priv->enabled || g_strcmp0("normal", priv->mode) == 0)
        {
          gdk_cairo_set_source_pixbuf(cr, priv->normal_sheep_icon, 0, 0);
        }
      else if (g_strcmp0("suspended", priv->mode) == 0)
        {
          gdk_cairo_set_source_pixbuf(cr, priv->suspended_sheep_icon, 0, 0);
        }
      else if (g_strcmp0("quiet", priv->mode) == 0)
        {
          gdk_cairo_set_source_pixbuf(cr, priv->quiet_sheep_icon, 0, 0);
        }
      cairo_paint(cr);
    }
}

static void
workrave_timerbox_update_time_bars(WorkraveTimerbox *self, cairo_t *cr)
{
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);

  if (priv->enabled)
    {
      int x = 0, y = 0;
      int bar_width, bar_height;

      if (priv->force_icon)
        {
          x += gdk_pixbuf_get_width(priv->normal_sheep_icon);
        }

      workrave_timebar_get_dimensions(priv->slot_to_time_bar[0], &bar_width, &bar_height);

      int icon_width = gdk_pixbuf_get_width(priv->break_to_icon[0]);
      int icon_height = gdk_pixbuf_get_height(priv->break_to_icon[0]);

      int icon_bar_width = icon_width + 2 * PADDING_X + bar_width;
      int icon_dy = 0;
      int bar_dy = 0;

      if (bar_height > icon_height)
        {
          icon_dy = (bar_height - icon_height + 1) / 2;
        }
      else
        {
          bar_dy = (icon_height - bar_height + 1) / 2;
        }

      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          WorkraveBreakId bid = priv->slot_to_break[i];

          if (bid != BREAK_ID_NONE)
            {
              WorkraveTimebar *bar = priv->slot_to_time_bar[bid];

              cairo_surface_t *surface = cairo_get_target(cr);
              cairo_surface_t *bar_surface = cairo_surface_create_for_rectangle(surface,
                                                                                x + icon_width + PADDING_X,
                                                                                y + bar_dy,
                                                                                bar_width,
                                                                                bar_height);
              cairo_t *bar_cr = cairo_create(bar_surface);
              workrave_timebar_draw(bar, bar_cr);
              cairo_surface_destroy(bar_surface);
              cairo_destroy(bar_cr);

              gdk_cairo_set_source_pixbuf(cr, priv->break_to_icon[bid], x, y + icon_dy);
              cairo_fill(cr);
              cairo_paint(cr);

              x += icon_bar_width + 2 * PADDING_X;
            }
        }
    }
}

static void
workrave_timerbox_compute_dimensions(WorkraveTimerbox *self, int *width, int *height)
{
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);

  int bar_width, bar_height;
  workrave_timebar_get_dimensions(priv->slot_to_time_bar[0], &bar_width, &bar_height);

  int icon_width = gdk_pixbuf_get_width(priv->break_to_icon[0]);
  int icon_height = gdk_pixbuf_get_height(priv->break_to_icon[0]);

  if (priv->enabled && priv->filled_slots > 0)
    {
      *width = priv->filled_slots * (icon_width + 4 * PADDING_X + bar_width) - 2 * PADDING_X;
      *height = MAX(icon_height, bar_height);

      if (priv->force_icon)
        {
          *width += gdk_pixbuf_get_width(priv->normal_sheep_icon) + PADDING_X;
        }
    }
  else
    {
      *width = gdk_pixbuf_get_width(priv->normal_sheep_icon);
      *height = gdk_pixbuf_get_height(priv->normal_sheep_icon);
    }
}

/**
 * workrave_timerbox_set_slot:
 * @self: a @WorkraveTimerbox
 * @slot: slot to set
 * @brk: break to put in slot
 *
 */
void
workrave_timerbox_set_slot(WorkraveTimerbox *self, int slot, WorkraveBreakId brk)
{
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);

  WorkraveBreakId old_brk = priv->slot_to_break[slot];
  if (old_brk != brk)
    {
      if (old_brk != BREAK_ID_NONE)
        {
          priv->break_visible[old_brk] = FALSE;
          priv->break_to_slot[old_brk] = -1;
          priv->filled_slots--;
        }
      priv->slot_to_break[slot] = brk;
      if (brk != BREAK_ID_NONE)
        {
          int old_slot = priv->break_to_slot[brk];
          if (old_slot >= 0)
            {
              priv->slot_to_break[old_slot] = BREAK_ID_NONE;
            }
          else
            {
              priv->filled_slots++;
            }
          priv->break_visible[brk] = TRUE;
          priv->break_to_slot[brk] = slot;
        }
    }
}

/**
 * workrave_timerbox_update:
 * @self: a @WorkraveTimerbox
 * @image: a @GtkImage where the timerbox will be drawn into
 *
 */
void
workrave_timerbox_update(WorkraveTimerbox *self, GtkImage *image)
{
  int width = 24;
  int height = 24;

  workrave_timerbox_compute_dimensions(self, &width, &height);

  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cairo_t *cr = cairo_create(surface);

  workrave_timerbox_draw(self, cr);

  GdkPixbuf *pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, width, height);
  gtk_image_set_from_pixbuf(image, pixbuf);

  g_object_unref(pixbuf);
  cairo_surface_destroy(surface);
  cairo_destroy(cr);
}

/**
 * workrave_timerbox_draw:
 * @self: a @WorkraveTimerbox
 * @cr: a @cairo_t where the timerbox will be drawn into
 *
 */
void
workrave_timerbox_draw(WorkraveTimerbox *self, cairo_t *cr)
{
  cairo_save(cr);
  cairo_set_source_rgba(cr, 0, 0, 0, 0);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(cr);
  cairo_restore(cr);

  workrave_timerbox_update_time_bars(self, cr);
  workrave_timerbox_update_sheep(self, cr);
}

/**
 * workrave_timerbox_get_width:
 * @self: a @WorkraveTimerbox
 *
 * Return value: The width of the @WorkraveTimerbox
 *
 */
int
workrave_timerbox_get_width(WorkraveTimerbox *self)
{
  int width;
  int height;
  workrave_timerbox_compute_dimensions(self, &width, &height);
  return width;
}

/**
 * workrave_timerbox_get_height:
 * @self: a @WorkraveTimerbox
 *
 * Return value: The width of the @WorkraveTimerbox
 *
 */
int
workrave_timerbox_get_height(WorkraveTimerbox *self)
{
  int width;
  int height;
  workrave_timerbox_compute_dimensions(self, &width, &height);
  return height;
}

/**
 * workrave_timerbox_get_time_bar:
 * @self: a @WorkraveTimerbox
 * @timer: the ID of the #WorkraveTimebar to return
 *
 * Return value: (transfer none): The @WorkraveTimebar of the specified timer
 *
 */
WorkraveTimebar *
workrave_timerbox_get_time_bar(WorkraveTimerbox *self, WorkraveBreakId timer)
{
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);
  return priv->slot_to_time_bar[timer];
}

/**
 * workrave_timerbox_set_enabled:
 * @self: a @WorkraveTimerbox
 * @enabled:
 *
 */
void
workrave_timerbox_set_enabled(WorkraveTimerbox *self, gboolean enabled)
{
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);
  priv->enabled = enabled;
}

/**
 * workrave_timerbox_set_force_icon:
 * @self: a @WorkraveTimerbox
 * @force: Whether the icon is always visible
 *
 */
void
workrave_timerbox_set_force_icon(WorkraveTimerbox *self, gboolean force)
{
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);
  priv->force_icon = force;
}

/**
 * workrave_timerbox_set_operation_mode:
 * @self: a @WorkraveTimerbox
 * @mode:
 *
 */
void
workrave_timerbox_set_operation_mode(WorkraveTimerbox *self, gchar *mode)
{
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);
  g_free(priv->mode);
  priv->mode = g_strdup(mode);
}

static GdkPixbuf *
workrave_load_image(WorkraveTimerbox *self, const char *name)
{
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);
  GdkPixbuf *ret = NULL;

  char *file = NULL;
  char *theme = g_settings_get_string(priv->settings, "icontheme");
  if (theme != NULL)
    {
      file = g_build_filename(WORKRAVE_PKGDATADIR, "images", theme, name, NULL);
      if (!g_file_test(file, G_FILE_TEST_EXISTS))
        {
          g_free(file);
          file = NULL;
        }
      g_free(theme);
    }

  if (file == NULL)
    {
      file = g_build_filename(WORKRAVE_PKGDATADIR, "images", name, NULL);
    }

  ret = gdk_pixbuf_new_from_file(file, NULL);
  g_free(file);

  return ret;
}

static void
workrave_timerbox_init_images(WorkraveTimerbox *self)
{
  WorkraveTimerboxPrivate *priv = workrave_timerbox_get_instance_private(self);

  const char *icons[] = {"timer-micro-break.png", "timer-rest-break.png", "timer-daily.png"};

  if (priv->normal_sheep_icon != NULL)
    {
      g_object_unref(priv->normal_sheep_icon);
    }
  if (priv->quiet_sheep_icon != NULL)
    {
      g_object_unref(priv->quiet_sheep_icon);
    }
  if (priv->suspended_sheep_icon != NULL)
    {
      g_object_unref(priv->suspended_sheep_icon);
    }

  priv->normal_sheep_icon = workrave_load_image(self, "workrave-icon-medium.png");
  priv->quiet_sheep_icon = workrave_load_image(self, "workrave-quiet-icon-medium.png");
  priv->suspended_sheep_icon = workrave_load_image(self, "workrave-suspended-icon-medium.png");

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (priv->break_to_icon[i] != NULL)
        {
          g_object_unref(priv->break_to_icon[i]);
        }
      priv->break_to_icon[i] = workrave_load_image(self, icons[i]);
    }
}

static void
workrave_on_settings_changed(GSettings *gsettings, const gchar *key, void *user_data)
{
  WorkraveTimerbox *self = WORKRAVE_TIMERBOX(user_data);
  workrave_timerbox_init_images(self);
}
