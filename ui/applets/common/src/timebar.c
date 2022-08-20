/* Copyright (C) 2011, 2013 Rob Caelers <robc@krandor.nl>
 * All rights reserved.
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

#include "timebar.h"

#include <cairo.h>
#include <gtk/gtk.h>

static void workrave_timebar_class_init(WorkraveTimebarClass *klass);
static void workrave_timebar_init(WorkraveTimebar *self);
static void workrave_timebar_dispose(GObject *self);
static void workrave_timebar_prepare(WorkraveTimebar *self);

static void workrave_timebar_draw_filled_box(WorkraveTimebar *self, cairo_t *cr, int x, int y, int width, int height);
static void workrave_timebar_draw_frame(WorkraveTimebar *self, cairo_t *cr, int width, int height);
static void workrave_timebar_compute_bar_dimensions(WorkraveTimebar *self, int *bar_width, int *sbar_width, int *bar_height);

const int MARGINX = 4;
const int MARGINY = 2;
const int MIN_HORIZONTAL_BAR_WIDTH = 12;
const int MIN_HORIZONTAL_BAR_HEIGHT = 20; // stolen from gtk's progress bar
const int BORDER_SIZE = 2;

enum
{
  PROP_0,
  PROP_NAME
};

struct _WorkraveTimebarPrivate
{
  //! Color of the time-bar.
  WorkraveColorId bar_color;

  //! Color of the time-bar.
  WorkraveColorId secondary_bar_color;

  //! Color of the text.
  GdkRGBA bar_text_color;

  //! The current value.
  int bar_value;

  //! The maximum value.
  int bar_max_value;

  //! The current value.
  int secondary_bar_value;

  //! The maximum value.
  int secondary_bar_max_value;

  //! Text to show;
  gchar *bar_text;

  int width;
  int height;

  PangoContext *pango_context;
  PangoLayout *pango_layout;
};

G_DEFINE_TYPE_WITH_PRIVATE(WorkraveTimebar, workrave_timebar, G_TYPE_OBJECT);

static GdkRGBA bar_colors[COLOR_ID_SIZEOF];

static void
set_color(cairo_t *cr, GdkRGBA color)
{
  cairo_set_source_rgb(cr, color.red, color.green, color.blue);
}

static void
workrave_timebar_class_init(WorkraveTimebarClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  gobject_class->dispose = workrave_timebar_dispose;

  gdk_rgba_parse(&bar_colors[COLOR_ID_ACTIVE], "lightblue");
  gdk_rgba_parse(&bar_colors[COLOR_ID_INACTIVE], "lightgreen");
  gdk_rgba_parse(&bar_colors[COLOR_ID_OVERDUE], "orange");
  gdk_rgba_parse(&bar_colors[COLOR_ID_1_ACTIVE_DURING_BREAK], "red");
  gdk_rgba_parse(&bar_colors[COLOR_ID_2_ACTIVE_DURING_BREAK], "#e00000");
  gdk_rgba_parse(&bar_colors[COLOR_ID_INACTIVE_OVER_ACTIVE], "#00d4b2");
  gdk_rgba_parse(&bar_colors[COLOR_ID_INACTIVE_OVER_OVERDUE], "lightgreen");
  gdk_rgba_parse(&bar_colors[COLOR_ID_BG], "#eeeeee");
}

static void
workrave_timebar_init(WorkraveTimebar *self)
{
  WorkraveTimebarPrivate *priv = workrave_timebar_get_instance_private(self);

  priv->bar_color = COLOR_ID_INACTIVE_OVER_OVERDUE;
  priv->secondary_bar_color = COLOR_ID_2_ACTIVE_DURING_BREAK;

  gdk_rgba_parse(&priv->bar_text_color, "black");
  priv->bar_value = 20;
  priv->bar_max_value = 50;
  priv->secondary_bar_value = 100;
  priv->secondary_bar_max_value = 600;
  priv->bar_text = g_strdup("");
  priv->width = 0;
  priv->height = 0;
  priv->pango_context = NULL;
  priv->pango_layout = NULL;

  workrave_timebar_prepare(self);
}

static void
workrave_timebar_dispose(GObject *gobject)
{
  WorkraveTimebar *self = WORKRAVE_TIMEBAR(gobject);
  WorkraveTimebarPrivate *priv = workrave_timebar_get_instance_private(self);

  g_clear_pointer(&priv->bar_text, g_free);
  g_clear_pointer(&priv->pango_layout, g_object_unref);

  /* Chain up to the parent class */
  G_OBJECT_CLASS(workrave_timebar_parent_class)->dispose(gobject);
}

void
workrave_timebar_draw_bar(WorkraveTimebar *self, cairo_t *cr)
{
  WorkraveTimebarPrivate *priv = workrave_timebar_get_instance_private(self);

  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_rectangle(cr, 0, 0, priv->width, priv->height);
  cairo_clip(cr);

  // Frame
  workrave_timebar_draw_frame(self, cr, priv->width, priv->height);

  int bar_width = 0;
  int sbar_width = 0;
  int bar_height = 0;
  workrave_timebar_compute_bar_dimensions(self, &bar_width, &sbar_width, &bar_height);

  // g_debug("bar_width %d %d", bar_width, sbar_width);

  if (sbar_width > 0)
    {
      // Overlap

      // We should assert() because this is not supported
      // but there are some weird boundary cases
      // in which this still happens.. need to check
      // this out some time.
      // assert(secondary_bar_color == COLOR_ID_INACTIVE);
      int overlap_color;
      switch (priv->bar_color)
        {
        case COLOR_ID_ACTIVE:
          overlap_color = COLOR_ID_INACTIVE_OVER_ACTIVE;
          break;
        case COLOR_ID_OVERDUE:
          overlap_color = COLOR_ID_INACTIVE_OVER_OVERDUE;
          break;
        default:
          // We could abort() because this is not supported
          // but there are some weird boundary cases
          // in which this still happens.. need to check
          // this out some time.
          overlap_color = COLOR_ID_INACTIVE_OVER_ACTIVE;
        }

      if (sbar_width >= bar_width)
        {
          if (bar_width)
            {
              GdkRGBA color = bar_colors[overlap_color];
              set_color(cr, color);
              workrave_timebar_draw_filled_box(self, cr, BORDER_SIZE, BORDER_SIZE, bar_width, bar_height);
            }
          if (sbar_width > bar_width)
            {
              GdkRGBA color = bar_colors[priv->secondary_bar_color];
              set_color(cr, color);
              workrave_timebar_draw_filled_box(self,
                                               cr,
                                               BORDER_SIZE + bar_width,
                                               BORDER_SIZE,
                                               sbar_width - bar_width,
                                               bar_height);
            }
        }
      else
        {
          if (sbar_width)
            {
              GdkRGBA color = bar_colors[overlap_color];
              set_color(cr, color);
              workrave_timebar_draw_filled_box(self, cr, BORDER_SIZE, BORDER_SIZE, sbar_width, bar_height);
            }
          GdkRGBA color = bar_colors[priv->bar_color];
          set_color(cr, color);
          workrave_timebar_draw_filled_box(self, cr, BORDER_SIZE + sbar_width, BORDER_SIZE, bar_width - sbar_width, bar_height);
        }
    }
  else
    {
      // No overlap
      GdkRGBA color = bar_colors[priv->bar_color];
      set_color(cr, color);
      workrave_timebar_draw_filled_box(self, cr, BORDER_SIZE, BORDER_SIZE, bar_width, bar_height);
    }
}

void
workrave_timebar_draw_text(WorkraveTimebar *self, cairo_t *cr)
{
  WorkraveTimebarPrivate *priv = workrave_timebar_get_instance_private(self);

  // g_debug("bar_text %s", priv->bar_text);
  pango_layout_set_text(priv->pango_layout, priv->bar_text, -1);

  int text_width, text_height;
  pango_layout_get_pixel_size(priv->pango_layout, &text_width, &text_height);

  int text_x, text_y;
  text_x = priv->width - text_width - MARGINX;
  if (text_x < 0)
    {
      text_x = MARGINX;
    }
  text_y = (priv->height - text_height) / 2;

  cairo_move_to(cr, text_x, text_y);
  set_color(cr, priv->bar_text_color);
  pango_cairo_show_layout(cr, priv->pango_layout);
}

static PangoFontDescription *
workrave_timebar_get_font(void)
{
  PangoFontDescription *font_desc;

  if (gdk_screen_get_default())
    {
      GtkStyleContext *style = gtk_style_context_new();
      GtkWidgetPath *path = gtk_widget_path_new();

      gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
      gtk_style_context_set_path(style, path);
      gtk_widget_path_unref(path);

      gtk_style_context_get(style, GTK_STATE_FLAG_NORMAL, GTK_STYLE_PROPERTY_FONT, &font_desc, NULL);
      g_object_unref(style);
    }
  else
    {
      font_desc = pango_font_description_from_string("Sans 10");
    }

  return font_desc;
}

static void
workrave_timebar_prepare(WorkraveTimebar *self)
{
  WorkraveTimebarPrivate *priv = workrave_timebar_get_instance_private(self);

  if (priv->pango_layout == NULL)
    {
      PangoFontDescription *font_desc = workrave_timebar_get_font();

      cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 10, 10);
      cairo_t *cr = cairo_create(surface);

      priv->pango_layout = pango_cairo_create_layout(cr);
      priv->pango_context = pango_layout_get_context(priv->pango_layout);

      pango_context_set_language(priv->pango_context, gtk_get_default_language());
      pango_context_set_font_description(priv->pango_context, font_desc);

      pango_layout_set_text(priv->pango_layout, "-9:59:59", -1);
      pango_layout_get_pixel_size(priv->pango_layout, &priv->width, &priv->height);

      priv->width = MAX(priv->width + 2 * MARGINX, MIN_HORIZONTAL_BAR_WIDTH);
      priv->height = MAX(priv->height + 2 * MARGINY, MIN_HORIZONTAL_BAR_HEIGHT);

      cairo_surface_destroy(surface);
      cairo_destroy(cr);
      pango_font_description_free(font_desc);
    }
}

static void
workrave_timebar_draw_frame(WorkraveTimebar *self, cairo_t *cr, int width, int height)
{
  cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
  cairo_set_line_width(cr, 1);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_fill(cr);

  cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
  cairo_move_to(cr, 1.5, 1.5);
  cairo_line_to(cr, width - 1.5, 1.5);
  cairo_move_to(cr, 1.5, 1.5);
  cairo_line_to(cr, 1.5, height - 1.5);
  cairo_stroke(cr);

  cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
  cairo_move_to(cr, 1.5, height - 1.5);
  cairo_line_to(cr, width - 1.5, height - 1.5);
  cairo_move_to(cr, width - 1.5, 1.5);
  cairo_line_to(cr, width - 1.5, height - 1.5);
  cairo_stroke(cr);

  GdkRGBA color = bar_colors[COLOR_ID_BG];
  set_color(cr, color);
  cairo_rectangle(cr, 2, 2, width - 4, height - 4);
  cairo_fill(cr);
}

static void
workrave_timebar_draw_filled_box(WorkraveTimebar *self, cairo_t *cr, int x, int y, int width, int height)
{
  (void)self;
  cairo_rectangle(cr, x, y, width, height);
  cairo_fill(cr);
}

static void
workrave_timebar_compute_bar_dimensions(WorkraveTimebar *self, int *bar_width, int *sbar_width, int *bar_height)
{
  WorkraveTimebarPrivate *priv = workrave_timebar_get_instance_private(self);

  // Primary bar
  *bar_width = 0;
  if (priv->bar_max_value > 0)
    {
      *bar_width = (priv->bar_value * (priv->width - 2 * BORDER_SIZE)) / priv->bar_max_value;
    }

  // Secondary bar
  *sbar_width = 0;
  if (priv->secondary_bar_max_value > 0)
    {
      *sbar_width = (priv->secondary_bar_value * (priv->width - 2 * BORDER_SIZE)) / priv->secondary_bar_max_value;
    }

  *bar_height = priv->height - 2 * BORDER_SIZE;
}

void
workrave_timebar_set_progress(WorkraveTimebar *self, int value, int max_value, WorkraveColorId color)
{
  WorkraveTimebarPrivate *priv = workrave_timebar_get_instance_private(self);

  priv->bar_value = value <= max_value ? value : max_value;
  priv->bar_max_value = max_value;
  priv->bar_color = color;
}

void
workrave_timebar_set_secondary_progress(WorkraveTimebar *self, int value, int max_value, WorkraveColorId color)
{
  WorkraveTimebarPrivate *priv = workrave_timebar_get_instance_private(self);

  priv->secondary_bar_value = value <= max_value ? value : max_value;
  priv->secondary_bar_max_value = max_value;
  priv->secondary_bar_color = color;
}

void
workrave_timebar_set_text(WorkraveTimebar *self, const gchar *text)
{
  WorkraveTimebarPrivate *priv = workrave_timebar_get_instance_private(self);
  g_free(priv->bar_text);
  priv->bar_text = g_strdup(text);
}

void
workrave_timebar_draw(WorkraveTimebar *self, cairo_t *cr)
{
  workrave_timebar_draw_bar(self, cr);
  workrave_timebar_draw_text(self, cr);
}

void
workrave_timebar_get_dimensions(WorkraveTimebar *self, int *width, int *height)
{
  WorkraveTimebarPrivate *priv = workrave_timebar_get_instance_private(self);

  *width = priv->width;
  *height = priv->height;
}
