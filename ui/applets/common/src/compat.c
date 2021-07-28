#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "compat.h"

/* Copy functions from GTK 3.0 to be used if GTK 2.x is used */

static cairo_format_t
gdk_cairo_format_for_content(cairo_content_t content)
{
  switch (content)
    {
    case CAIRO_CONTENT_COLOR:
      return CAIRO_FORMAT_RGB24;
    case CAIRO_CONTENT_ALPHA:
      return CAIRO_FORMAT_A8;
    case CAIRO_CONTENT_COLOR_ALPHA:
    default:
      return CAIRO_FORMAT_ARGB32;
    }
}

static cairo_surface_t *
gdk_cairo_surface_coerce_to_image(cairo_surface_t *surface, cairo_content_t content, int src_x, int src_y, int width, int height)
{
  cairo_surface_t *copy;
  cairo_t *cr;

  copy = cairo_image_surface_create(gdk_cairo_format_for_content(content), width, height);

  cr = cairo_create(copy);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_surface(cr, surface, -src_x, -src_y);
  cairo_paint(cr);
  cairo_destroy(cr);

  return copy;
}

static void
convert_alpha(guchar *dest_data, int dest_stride, guchar *src_data, int src_stride, int src_x, int src_y, int width, int height)
{
  int x, y;

  src_data += src_stride * src_y + src_x * 4;

  for (y = 0; y < height; y++)
    {
      guint32 *src = (guint32 *)src_data;

      for (x = 0; x < width; x++)
        {
          guint alpha = src[x] >> 24;

          if (alpha == 0)
            {
              dest_data[x * 4 + 0] = 0;
              dest_data[x * 4 + 1] = 0;
              dest_data[x * 4 + 2] = 0;
            }
          else
            {
              dest_data[x * 4 + 0] = (((src[x] & 0xff0000) >> 16) * 255 + alpha / 2) / alpha;
              dest_data[x * 4 + 1] = (((src[x] & 0x00ff00) >> 8) * 255 + alpha / 2) / alpha;
              dest_data[x * 4 + 2] = (((src[x] & 0x0000ff) >> 0) * 255 + alpha / 2) / alpha;
            }
          dest_data[x * 4 + 3] = alpha;
        }

      src_data += src_stride;
      dest_data += dest_stride;
    }
}

static void
convert_no_alpha(guchar *dest_data, int dest_stride, guchar *src_data, int src_stride, int src_x, int src_y, int width, int height)
{
  int x, y;

  src_data += src_stride * src_y + src_x * 4;

  for (y = 0; y < height; y++)
    {
      guint32 *src = (guint32 *)src_data;

      for (x = 0; x < width; x++)
        {
          dest_data[x * 3 + 0] = src[x] >> 16;
          dest_data[x * 3 + 1] = src[x] >> 8;
          dest_data[x * 3 + 2] = src[x];
        }

      src_data += src_stride;
      dest_data += dest_stride;
    }
}

/**
 * gdk_pixbuf_get_from_surface:
 * @surface: surface to copy from
 * @src_x: Source X coordinate within @surface
 * @src_y: Source Y coordinate within @surface
 * @width: Width in pixels of region to get
 * @height: Height in pixels of region to get
 *
 * Transfers image data from a #cairo_surface_t and converts it to an RGB(A)
 * representation inside a #GdkPixbuf. This allows you to efficiently read
 * individual pixels from cairo surfaces. For #GdkWindows, use
 * gdk_pixbuf_get_from_window() instead.
 *
 * This function will create an RGB pixbuf with 8 bits per channel.
 * The pixbuf will contain an alpha channel if the @surface contains one.
 *
 * Return value: (transfer full): A newly-created pixbuf with a reference
 *     count of 1, or %NULL on error
 */
GdkPixbuf *
gdk_pixbuf_get_from_surface(cairo_surface_t *surface, gint src_x, gint src_y, gint width, gint height)
{
  cairo_content_t content;
  GdkPixbuf *dest;

  /* General sanity checks */
  g_return_val_if_fail(surface != NULL, NULL);
  g_return_val_if_fail(width > 0 && height > 0, NULL);

  content = (cairo_content_t)(cairo_surface_get_content(surface) | CAIRO_CONTENT_COLOR);
  dest = gdk_pixbuf_new(GDK_COLORSPACE_RGB, !!(content & CAIRO_CONTENT_ALPHA), 8, width, height);

  surface = gdk_cairo_surface_coerce_to_image(surface, content, src_x, src_y, width, height);
  cairo_surface_flush(surface);
  if (cairo_surface_status(surface) || dest == NULL)
    {
      cairo_surface_destroy(surface);
      return NULL;
    }

  if (gdk_pixbuf_get_has_alpha(dest))
    convert_alpha(gdk_pixbuf_get_pixels(dest),
                  gdk_pixbuf_get_rowstride(dest),
                  cairo_image_surface_get_data(surface),
                  cairo_image_surface_get_stride(surface),
                  0,
                  0,
                  width,
                  height);
  else
    convert_no_alpha(gdk_pixbuf_get_pixels(dest),
                     gdk_pixbuf_get_rowstride(dest),
                     cairo_image_surface_get_data(surface),
                     cairo_image_surface_get_stride(surface),
                     0,
                     0,
                     width,
                     height);

  cairo_surface_destroy(surface);
  return dest;
}
