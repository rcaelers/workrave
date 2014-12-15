/*
 * workrave-timebar.c
 *
 * Copyright (C) 2011, 2013 Rob Caelers <robc@krandor.nl>
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

#include "timebar.h"

#include <cairo.h>
#include <gtk/gtk.h>

#define WORKRAVE_TIMEBAR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), WORKRAVE_TYPE_TIMEBAR, WorkraveTimebarPrivate))

static void workrave_timebar_class_init(WorkraveTimebarClass *klass);
static void workrave_timebar_init(WorkraveTimebar *self);

static void workrave_timebar_init_ui(WorkraveTimebar *self);
static void workrave_timebar_draw_filled_box(WorkraveTimebar *self, cairo_t *cr, int x, int y, int width, int height);
static void workrave_timebar_draw_frame(WorkraveTimebar *self, cairo_t *cr, int width, int height);
static void workrave_timebar_compute_bar_dimensions(WorkraveTimebar *self, int *bar_width, int *sbar_width, int *bar_height);

G_DEFINE_TYPE(WorkraveTimebar, workrave_timebar, G_TYPE_OBJECT);

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

#ifdef USE_GTK2
#include "compat.h"
#endif

struct _WorkraveTimebarPrivate
{
  gchar *name;

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

#ifndef USE_GTK2  
  GtkStyleContext *style_context;
#endif  
  PangoContext *pango_context;
  PangoLayout *pango_layout;

  GdkRGBA front_color;
};


static GdkRGBA bar_colors[COLOR_ID_SIZEOF];

#ifdef USE_GTK2
static void set_color(cairo_t *cr, GdkColor color)
{
  cairo_set_source_rgb(cr, color.red / 65535.0, color.green / 65535.0, color.blue / 65535.0);
}
#else
static void set_color(cairo_t *cr, GdkRGBA color)
{
  cairo_set_source_rgb(cr, color.red, color.green, color.blue);
}
#endif

static void
workrave_timebar_class_init(WorkraveTimebarClass *klass)
{
  g_type_class_add_private(klass, sizeof(WorkraveTimebarPrivate));

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
  self->priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self);

  self->priv->bar_color = COLOR_ID_INACTIVE_OVER_OVERDUE;
  self->priv->secondary_bar_color = COLOR_ID_2_ACTIVE_DURING_BREAK;

  gdk_rgba_parse(&self->priv->bar_text_color, "black");
  self->priv->bar_value = 20;
  self->priv->bar_max_value = 50;
  self->priv->secondary_bar_value = 100;
  self->priv->secondary_bar_max_value = 600;
  self->priv->bar_text = g_strdup("");
  gdk_rgba_parse(&self->priv->front_color, "#000000");

  workrave_timebar_init_ui(self);
}


void
workrave_timebar_draw_bar(WorkraveTimebar *self, cairo_t *cr)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self);

  // clip to the area indicated by the expose event so that we only redraw
  // the portion of the window that needs to be redrawn
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
          // We could abort() becaue this is not supported
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

              workrave_timebar_draw_filled_box(self, cr,
                                               BORDER_SIZE, BORDER_SIZE,
                                               bar_width, bar_height);
            }
          if (sbar_width > bar_width)
            {
              GdkRGBA color = bar_colors[priv->secondary_bar_color];
              set_color(cr, color);
              workrave_timebar_draw_filled_box(self, cr,
                                               BORDER_SIZE + bar_width, BORDER_SIZE,
                                               sbar_width - bar_width, bar_height);
            }
        }
      else
        {
          if (sbar_width)
            {
              GdkRGBA color = bar_colors[overlap_color];
              set_color(cr, color);
              workrave_timebar_draw_filled_box(self, cr,
                                               BORDER_SIZE, BORDER_SIZE,
                                               sbar_width, bar_height);
            }
          GdkRGBA color = bar_colors[priv->bar_color];
          set_color(cr, color);
          workrave_timebar_draw_filled_box(self, cr,
                                           BORDER_SIZE + sbar_width, BORDER_SIZE,
                                           bar_width - sbar_width, bar_height);
        }
    }
  else
    {
      // No overlap
      GdkRGBA color = bar_colors[priv->bar_color];
      set_color(cr, color);
      workrave_timebar_draw_filled_box(self, cr,
                                       BORDER_SIZE, BORDER_SIZE,
                                       bar_width, bar_height);
    }
}

void
workrave_timebar_draw_text(WorkraveTimebar *self, cairo_t *cr)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self);

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

  int bar_width = 0;
  int sbar_width = 0;
  int bar_height = 0;
  workrave_timebar_compute_bar_dimensions(self, &bar_width, &sbar_width, &bar_height);

  int left_width = (bar_width > sbar_width) ? bar_width : sbar_width;
  left_width += BORDER_SIZE;

  GdkRectangle left_rect = { 0, 0, left_width, priv->height };
  GdkRectangle right_rect = { left_width, 0, priv->width - left_width, priv->height };

  cairo_reset_clip(cr);
  cairo_rectangle(cr, left_rect.x, left_rect.y, left_rect.width, left_rect.height);
  cairo_clip(cr);

  cairo_move_to(cr, text_x, text_y);
  set_color(cr, priv->bar_text_color);
  pango_cairo_show_layout(cr, priv->pango_layout);

  cairo_reset_clip(cr);
  cairo_rectangle(cr, right_rect.x, right_rect.y, right_rect.width, right_rect.height);
  cairo_clip(cr);

  cairo_move_to(cr, text_x, text_y);
  set_color(cr, priv->front_color);
  pango_cairo_show_layout(cr, priv->pango_layout);
}


#ifndef USE_GTK2  
static void
workrave_timebar_init_ui(WorkraveTimebar *self)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self);

  priv->style_context = gtk_style_context_new();
  GtkWidgetPath *path = gtk_widget_path_new();
  gtk_widget_path_append_type(path, GTK_TYPE_BUTTON);
  gtk_style_context_set_path(priv->style_context, path);
  gtk_style_context_add_class(priv->style_context, GTK_STYLE_CLASS_TROUGH);
  
  GdkScreen *screen = gdk_screen_get_default();
  priv->pango_context = gdk_pango_context_get_for_screen(screen);

  const PangoFontDescription *font_desc = gtk_style_context_get_font(priv->style_context, GTK_STATE_FLAG_ACTIVE);

  pango_context_set_language(priv->pango_context, gtk_get_default_language());
  pango_context_set_font_description(priv->pango_context, font_desc);

  priv->pango_layout = pango_layout_new(priv->pango_context);
  pango_layout_set_text(priv->pango_layout, "-9:59:59", -1);

  pango_layout_get_pixel_size(priv->pango_layout, &priv->width, &priv->height);

  priv->width = MAX(priv->width + 2 * MARGINX, MIN_HORIZONTAL_BAR_WIDTH);
  priv->height = MAX(priv->height + 2 * MARGINY, MIN_HORIZONTAL_BAR_HEIGHT);

  gtk_widget_path_free(path);
  gtk_style_context_set_state(priv->style_context, GTK_STATE_FLAG_ACTIVE);
  gtk_style_context_get_color(priv->style_context, GTK_STATE_FLAG_NORMAL, &priv->front_color);
}

static void
workrave_timebar_draw_frame(WorkraveTimebar *self, cairo_t *cr,
                            int width, int height)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self);

  gtk_style_context_save(priv->style_context);
  gtk_style_context_set_state(priv->style_context, (GtkStateFlags)GTK_STATE_FLAG_ACTIVE);
  gtk_render_background(priv->style_context, cr, 0, 0, width -1, height -1);
  gtk_render_frame(priv->style_context, cr, 0, 0, width -1, height -1);
  gtk_style_context_restore(priv->style_context);
}

#else

static void
workrave_timebar_init_ui(WorkraveTimebar *self)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self);

  GdkScreen *screen = gdk_screen_get_default();
  priv->pango_context = gdk_pango_context_get_for_screen(screen);

  const PangoFontDescription *font_desc = pango_font_description_from_string("Sans 10");

  pango_context_set_language(priv->pango_context, gtk_get_default_language());
  pango_context_set_font_description(priv->pango_context, font_desc);

  priv->pango_layout = pango_layout_new(priv->pango_context);
  pango_layout_set_text(priv->pango_layout, "-9:59:59", -1);

  pango_layout_get_pixel_size(priv->pango_layout, &priv->width, &priv->height);

  priv->width = MAX(priv->width + 2 * MARGINX, MIN_HORIZONTAL_BAR_WIDTH);
  priv->height = MAX(priv->height + 2 * MARGINY, MIN_HORIZONTAL_BAR_HEIGHT);
}

static void
workrave_timebar_draw_frame(WorkraveTimebar *self, cairo_t *cr,
                            int x, int y, int width, int height)
{
  cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
  cairo_rectangle(cr, x, y, width, height);
  cairo_fill(cr);

  GdkRGBA color = bar_colors[COLOR_ID_BG];
  set_color(cr, color);
  cairo_rectangle(cr, x + 2, y + 2, width - 4 , height - 4);
  cairo_fill(cr);

}
#endif 

static void
workrave_timebar_draw_filled_box(WorkraveTimebar *self, cairo_t *cr,
                                 int x, int y, int width, int height)
{
  (void) self;
  cairo_rectangle(cr, x, y, width, height);
  cairo_fill(cr);
}

static void
workrave_timebar_compute_bar_dimensions(WorkraveTimebar *self, int *bar_width, int *sbar_width, int *bar_height)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self);

  // Primary bar
  *bar_width = 0;
  if (priv->bar_max_value > 0)
    {
      *bar_width = (priv->bar_value * (priv->width - 2 * BORDER_SIZE) -1) / priv->bar_max_value;
    }

  // Secondary bar
  *sbar_width = 0;
  if (priv->secondary_bar_max_value >  0)
    {
      *sbar_width = (priv->secondary_bar_value * (priv->width - 2 * BORDER_SIZE) -1) / priv->secondary_bar_max_value;
    }

  *bar_height = priv->height - 2 * BORDER_SIZE - 1;
}


void
workrave_timebar_set_progress(WorkraveTimebar *self, int value, int max_value, WorkraveColorId color)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self);

  priv->bar_value = value <= max_value ? value : max_value;
  priv->bar_max_value = max_value;
  priv->bar_color = color;
}

void
workrave_timebar_set_secondary_progress(WorkraveTimebar *self, int value, int max_value, WorkraveColorId color)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self);

  priv->secondary_bar_value = value <= max_value ? value : max_value;
  priv->secondary_bar_max_value = max_value;
  priv->secondary_bar_color = color;
}

void
workrave_timebar_set_text(WorkraveTimebar *self, const gchar *text)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self);
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
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self);

  *width = priv->width;
  *height = priv->height;
}
