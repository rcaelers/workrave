#ifndef __WORKRAVE_TIMERBOX_COMPAT_H__
#define __WORKRAVE_TIMERBOX_COMPAT_H__

#include <gtk/gtk.h>

GdkPixbuf *gdk_pixbuf_get_from_surface(cairo_surface_t *surface, gint src_x, gint src_y, gint width, gint height);

#define GdkRGBA GdkColor
#define gdk_rgba_parse(x, y) gdk_color_parse((y), (x));

#endif /* __WORKRAVE_TIMERBOX_COMPAT_H__ */
