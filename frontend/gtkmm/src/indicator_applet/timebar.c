/*
 * workrave-timebar.c
 *
 * Copyright (C) 2011 Rob Caelers <robc@krandor.nl>
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
static void workrave_timebar_dispose(GObject *gobject);
static void workrave_timebar_finalize(GObject *gobject);
static void workrave_timebar_set_property(GObject *gobject, guint property_id, const GValue *value, GParamSpec *pspec);
static void workrave_timebar_get_property (GObject *gobject, guint property_id, GValue *value, GParamSpec *pspec);

static void workrave_timebar_draw_bar(WorkraveTimebar *self, cairo_t *cr, int x, int y, int width, int height, int winw, int winh);

G_DEFINE_TYPE(WorkraveTimebar, workrave_timebar, G_TYPE_OBJECT);

const int MARGINX = 4;
const int MARGINY = 2;
const int MIN_HORIZONTAL_BAR_WIDTH = 12;
const int MIN_HORIZONTAL_BAR_HEIGHT = 20; // stolen from gtk's progress bar

enum
{
  PROP_0,
  PROP_NAME
};

struct _WorkraveTimebarPrivate
{
  gchar *name;

  //! Color of the time-bar.
  ColorId bar_color;

  //! Color of the time-bar.
  ColorId secondary_bar_color;

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

  //! Text alignment
  int bar_text_align;

  //! Bar rotation (clockwise degrees)
  int rotation;

  WorkraveTimebar *timebar;
};


static GdkRGBA bar_colors[COLOR_ID_SIZEOF];

static void
workrave_timebar_class_init(WorkraveTimebarClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GParamSpec *pspec;

  gobject_class->dispose = workrave_timebar_dispose;
  gobject_class->finalize = workrave_timebar_finalize;
  gobject_class->set_property = workrave_timebar_set_property;
  gobject_class->get_property = workrave_timebar_get_property;

  pspec = g_param_spec_string("workrave",
                              "Workrave construct prop",
                              "Set workrave's name",
                              "no-name-set" /* default value */,
                              G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_property(gobject_class,
                                  PROP_NAME,
                                  pspec);

  g_type_class_add_private(klass, sizeof(WorkraveTimebarPrivate));

  gdk_rgba_parse(&bar_colors[COLOR_ID_ACTIVE], "lightblue");
  gdk_rgba_parse(&bar_colors[COLOR_ID_INACTIVE], "lightgreen");
  gdk_rgba_parse(&bar_colors[COLOR_ID_OVERDUE], "orange");
  gdk_rgba_parse(&bar_colors[COLOR_ID_1_ACTIVE_DURING_BREAK], "red");
  gdk_rgba_parse(&bar_colors[COLOR_ID_2_ACTIVE_DURING_BREAK], "#e00000");
  gdk_rgba_parse(&bar_colors[COLOR_ID_INACTIVE_OVER_ACTIVE], "#00d4b2");
  gdk_rgba_parse(&bar_colors[COLOR_ID_INACTIVE_OVER_OVERDUE], "lightgreen");
  gdk_rgba_parse(&bar_colors[COLOR_ID_BG], "#000000");
  
}


static void
workrave_timebar_init(WorkraveTimebar *self)
{
  self->priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self); 


  //gchar *name;

  self->priv->bar_color = COLOR_ID_INACTIVE_OVER_OVERDUE;
  self->priv->secondary_bar_color = COLOR_ID_2_ACTIVE_DURING_BREAK;

  gdk_rgba_parse(&self->priv->bar_text_color, "lightgreen");
  self->priv->bar_value = 20;
  self->priv->bar_max_value = 50;
  self->priv->secondary_bar_value = 100;
  self->priv->secondary_bar_max_value = 600;
  self->priv->bar_text = "foo";
  self->priv->bar_text_align = 1;
  self->priv->rotation = 0;
  
  /* initialize all public and private members to reasonable default values. */

  /* If you need specific construction properties to complete initialization,
   * delay initialization completion until the property is set. 
   */
}



static void
workrave_timebar_dispose(GObject *gobject)
{
  //WorkraveTimebar *self = WORKRAVE_TIMEBAR(gobject);

  /* 
   * In dispose, you are supposed to free all types referenced from this
   * object which might themselves hold a reference to self. Generally,
   * the most simple solution is to unref all members on which you own a 
   * reference.
   */

  /* dispose might be called multiple times, so we must guard against
   * calling g_object_unref() on an invalid GObject.
   */

  /* Chain up to the parent class */
  G_OBJECT_CLASS(workrave_timebar_parent_class)->dispose(gobject);
}


static void
workrave_timebar_finalize(GObject *gobject)
{
  //  WorkraveTimebar *self = WORKRAVE_TIMEBAR(gobject);

  /* Chain up to the parent class */
  G_OBJECT_CLASS(workrave_timebar_parent_class)->finalize(gobject);
}


void
workrave_timebar_do_action(WorkraveTimebar *self, int param)
{
  g_return_if_fail(WORKRAVE_IS_TIMEBAR(self));

  /* do stuff here. */
}


static void
workrave_timebar_set_property(GObject *gobject, guint property_id, const GValue *value, GParamSpec *pspec)
{
  WorkraveTimebar *self = WORKRAVE_TIMEBAR(gobject);
  //GObject *obj;

  switch (property_id)
    {
    case PROP_NAME:
      g_free (self->priv->name);
      self->priv->name = g_value_dup_string(value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, property_id, pspec);
      break;
    }
}

static void
workrave_timebar_get_property (GObject *gobject, guint property_id, GValue *value, GParamSpec *pspec)
{
  WorkraveTimebar *self = WORKRAVE_TIMEBAR(gobject);

  switch (property_id)
    {
    case PROP_NAME:
      g_value_set_string(value, self->priv->name);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, property_id, pspec);
      break;
    }
}



/* static void */
/* workrave_timebar_get_preferred_size(int &width, int &height) const */
/* { */
/*   // Not sure why create_pango_layout is not const... */
/*   pango_layout pl = const_cast<TimeBar *>(this)->create_pango_layout(bar_text); */

/*   string min_string = Text::time_to_string(-(59+59*60+9*60*60));; */
/*   pango_layout plmin = const_cast<TimeBar *>(this)->create_pango_layout(min_string); */

/*   Glib::RefPtr<Pango::Context> pcl = pl->get_context(); */
/*   Glib::RefPtr<Pango::Context> pcmin = plmin->get_context(); */
/*   Pango::Matrix matrix = PANGO_MATRIX_INIT; */

/*   pango_matrix_rotate(&matrix, 360 - rotation); */

/*   pcl->set_matrix(matrix); */
/*   pcmin->set_matrix(matrix); */

/*   pl->get_pixel_size(width, height); */

/*   int mwidth, mheight; */
/*   plmin->get_pixel_size(mwidth, mheight); */
/*   if (mwidth > width) */
/*     width = mwidth; */
/*   if (mheight > height) */
/*     height = mheight; */

/*   width = width + 2 * MARGINX; */
/*   height = max(height + 2 * MARGINY, MIN_HORIZONTAL_BAR_HEIGHT); */
/* } */

void
workrave_timebar_draw(WorkraveTimebar *self, cairo_t *cr, int x, int y, int win_w, int win_h)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self); 

  const int border_size = 2;

  GtkStyleContext *context = gtk_style_context_new();
  GtkWidgetPath *path = gtk_widget_path_new();

  gtk_widget_path_append_type (path, GTK_TYPE_FRAME);
  gtk_style_context_set_path(context, path);
  
  // Logical width/height
  // width = direction of bar
  int win_lw, win_lh;
  if (priv->rotation == 0 || priv->rotation == 180)
    {
      win_lw = win_w;
      win_lh = win_h;
    }
  else
    {
      win_lw = win_h;
      win_lh = win_w;
    }

  // Draw background
  gtk_style_context_set_state(context, GTK_STATE_FLAG_ACTIVE);
  GdkRGBA back_color;
  gtk_style_context_get_background_color(context, GTK_STATE_FLAG_ACTIVE, &back_color);
  cairo_set_source_rgb(cr, back_color.red, back_color.green, back_color.blue);

  // clip to the area indicated by the expose event so that we only redraw
  // the portion of the window that needs to be redrawn
  cairo_rectangle(cr, 0, 0, win_w, win_h);
  cairo_clip(cr);

  gtk_style_context_save(context);
  gtk_style_context_set_state(context, (GtkStateFlags)GTK_STATE_FLAG_ACTIVE);
  gtk_render_frame(context, cr, 0, 0, win_w - 1, win_h -1);
  gtk_style_context_restore(context);
  
  cairo_set_source_rgb(cr, back_color.red, back_color.green, back_color.blue);
  cairo_rectangle(cr, border_size, border_size, win_w - 2*border_size, win_h - 2*border_size);
  cairo_fill(cr);
  
  // Bar
  int bar_width = 0;
  if (priv->bar_max_value > 0)
    {
      bar_width = (priv->bar_value * (win_lw - 2 * border_size)) / priv->bar_max_value;
    }

  // Secondary bar
  int sbar_width = 0;
  if (priv->secondary_bar_max_value >  0)
    {
      sbar_width = (priv->secondary_bar_value * (win_lw - 2 * border_size)) / priv->secondary_bar_max_value;
    }

  int bar_height = win_lh - 2 * border_size;

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
              cairo_set_source_rgb(cr, color.red, color.green, color.blue);
              
              workrave_timebar_draw_bar(self, cr,
                       border_size, border_size,
                       bar_width, bar_height,
                       win_lw, win_lh);
            }
          if (sbar_width > bar_width)
            {
              GdkRGBA color = bar_colors[priv->secondary_bar_color];
              cairo_set_source_rgb(cr, color.red, color.green, color.blue);
              workrave_timebar_draw_bar(self, cr,
                       border_size + bar_width, border_size,
                       sbar_width - bar_width, bar_height,
                       win_lw, win_lh);
            }
        }
      else
        {
          if (sbar_width)
            {
              GdkRGBA color = bar_colors[overlap_color];
              cairo_set_source_rgb(cr, color.red, color.green, color.blue);
              workrave_timebar_draw_bar(self, cr,
                       border_size, border_size,
                       sbar_width, bar_height,
                       win_lw, win_lh);
            }
          GdkRGBA color = bar_colors[priv->bar_color];
          cairo_set_source_rgb(cr, color.red, color.green, color.blue);
          workrave_timebar_draw_bar(self, cr,
                   border_size + sbar_width, border_size,
                   bar_width - sbar_width, bar_height,
                   win_lw, win_lh);
        }
    }
  else
    {
      // No overlap
      GdkRGBA color = bar_colors[priv->bar_color];
      cairo_set_source_rgb(cr, color.red, color.green, color.blue);
      workrave_timebar_draw_bar(self, cr,
               border_size, border_size,
               bar_width, bar_height, win_lw, win_lh);
    }


  // Text
  PangoMatrix matrix = PANGO_MATRIX_INIT;
  pango_matrix_rotate(&matrix, 360 - priv->rotation);

  PangoContext *pc = pango_context_new();
  PangoLayout *layout = pango_layout_new(pc);

  pango_layout_set_text(layout, priv->bar_text, -1);
  pango_context_set_matrix(pc, &matrix);

  int text_width = 50, text_height = 16;
  //pango_layout_get_pixel_size(layout, &text_width, &text_height);

  int text_x, text_y;

  GdkRectangle rect1, rect2;

  if (priv->rotation == 0 || priv->rotation == 180)
    {
      if (win_w - text_width - MARGINX > 0)
        {
          if (priv->bar_text_align > 0)
            text_x = (win_w - text_width - MARGINX);
          else if (priv->bar_text_align < 0)
            text_x = MARGINX;
          else
            text_x = (win_w - text_width) / 2;
        }
      else
        {
          text_x = MARGINX;
        }
      text_y = (win_h - text_height) / 2;

      int left_width = (bar_width > sbar_width) ? bar_width : sbar_width;
      left_width += border_size;

      GdkRectangle left_rect = { 0, 0, left_width, win_h };
      GdkRectangle right_rect = { left_width, 0, win_w - left_width, win_h };

      rect1 = left_rect;
      rect2 = right_rect;
    }
  else
    {
      if (win_h - text_width - MARGINY > 0)
        {
          int a = priv->bar_text_align;
          if (priv->rotation == 270)
            {
              a *= -1;
            }
          if (a > 0)
            text_y = (win_h - text_width - MARGINY);
          else if (a < 0)
            text_y = MARGINY;
          else
            text_y = (win_h - text_width) / 2;
        }
      else
        {
          text_y = MARGINY;
        }

      text_x = (win_w - text_height) / 2;

      int left_width = (bar_width > sbar_width) ? bar_width : sbar_width;
      left_width += border_size;

      GdkRectangle up_rect = { 0, 0, win_w, left_width };
      GdkRectangle down_rect = { 0, left_width, win_w, win_h - left_width };

      rect1 = up_rect;
      rect2 = down_rect;
    }

  cairo_reset_clip(cr);
  cairo_rectangle(cr, rect1.x, rect1.y, rect1.width, rect1.height);
  cairo_clip(cr);

  /* cairo_move_to(cr, text_x, text_y); */
  /* GdkRGBA color = priv->bar_text_color; */
  /* cairo_set_source_rgb(cr, color.red, color.green, color.blue); */
  /* pango_cairo_show_layout(cr, layout); */
 
  /* GdkRGBA front_color; */
  /* gtk_style_context_get_color(context, GTK_STATE_FLAG_ACTIVE, &front_color); */
  /* cairo_reset_clip(cr); */
  /* cairo_rectangle(cr, rect2.x, rect2.y, rect2.width, rect2.height); */
  /* cairo_clip(cr); */
  /* cairo_move_to(cr, text_x, text_y); */
  /* cairo_set_source_rgb(cr, front_color.red, front_color.green, front_color.blue); */
  /* pango_cairo_show_layout(cr, layout); */

  //gtk_style_context_restore(context);

  gtk_widget_path_free (path);
}

void
workrave_timebar_draw_bar(WorkraveTimebar *self, cairo_t *cr,
                          int x, int y, int width, int height,
                          int winw, int winh)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self); 
  (void) winh;

  if (priv->rotation == 0 || priv->rotation == 180)
    {
      cairo_rectangle(cr, x, y, width, height);
      cairo_fill(cr);
    }
  else
    {
      cairo_rectangle(cr, y, winw - x- width, height, width);
      cairo_fill(cr);
    }
}

void workrave_timebar_set_progress(WorkraveTimebar *self, int value, int max_value, ColorId color)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self); 

  priv->bar_value = value;
  priv->bar_max_value = max_value;
  priv->bar_color = color;
}

void workrave_timebar_set_secondary_progress(WorkraveTimebar *self, int value, int max_value, ColorId color)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self); 

  priv->secondary_bar_value = value;
  priv->secondary_bar_max_value = max_value;
  priv->secondary_bar_color = color;
}

void workrave_timebar_set_text(WorkraveTimebar *self, const char *text)
{
  WorkraveTimebarPrivate *priv = WORKRAVE_TIMEBAR_GET_PRIVATE(self); 
  priv->bar_text = text;
}
