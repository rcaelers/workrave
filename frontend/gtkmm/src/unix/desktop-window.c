#include "desktop-window.h"

#ifdef PLATFORM_OS_UNIX

#include <gdk/gdk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>

#ifdef HAVE_GTK3
#include <cairo.h>
#include <cairo-xlib.h>

// GTK3 compatibility
#undef GDK_DISPLAY
#define GDK_DISPLAY() GDK_DISPLAY_XDISPLAY(gdk_display_get_default())
#endif

#ifndef GDK_WINDOW_XWINDOW
#define GDK_WINDOW_XWINDOW(w) GDK_WINDOW_XID(w)
#endif


static Window
get_desktop_window (Window the_window)
{
  Atom prop, type, prop2;
  int format;
  unsigned long length, after;
  unsigned char *data;
  unsigned int nchildren;
  Window w, root, *children, parent;

  prop = XInternAtom(GDK_DISPLAY(), "_XROOTPMAP_ID", True);
  prop2 = XInternAtom(GDK_DISPLAY(), "_XROOTCOLOR_PIXEL", True);

  if (prop == None && prop2 == None)
    return None;

  for (w = the_window; w; w = parent) {
    if ((XQueryTree(GDK_DISPLAY(), w, &root, &parent, &children, &nchildren)) == False)
      return None;

    if (nchildren)
      XFree(children);

    if (prop != None) {
      XGetWindowProperty(GDK_DISPLAY(), w, prop, 0L, 1L, False, AnyPropertyType,
       &type, &format, &length, &after, &data);
    } else if (prop2 != None) {
      XGetWindowProperty(GDK_DISPLAY(), w, prop2, 0L, 1L, False, AnyPropertyType,
       &type, &format, &length, &after, &data);
    } else  {
      continue;
    }

    if (type != None) {
      return w;
    }
  }
  return None;
}

static Pixmap
get_pixmap_prop (Window the_window, char *prop_id)
{
  Atom prop, type;
  int format;
  unsigned long length, after;
  unsigned char *data;

  Window desktop_window = get_desktop_window(the_window);

  if(desktop_window == None)
    desktop_window = GDK_ROOT_WINDOW();

  prop = XInternAtom(GDK_DISPLAY(), prop_id, True);

  if (prop == None)
    return None;

  XGetWindowProperty(GDK_DISPLAY(), desktop_window, prop, 0L, 1L, False,
         AnyPropertyType, &type, &format, &length, &after,
         &data);

  if (type == XA_PIXMAP)
    return *((Pixmap *)data);

  return None;
}



void
set_desktop_background(GdkWindow *window)
{
  Pixmap xpm = get_pixmap_prop(GDK_WINDOW_XWINDOW(window), "_XROOTPMAP_ID");

  if (xpm != None)
    {
#ifdef HAVE_GTK3
      GdkScreen *screen = gdk_window_get_screen(window);
      Window root_return;
      int x, y;
      unsigned int width, height, bw, depth_ret;
      cairo_surface_t *surface = NULL;
      
      gdk_error_trap_push();
      if (XGetGeometry(GDK_SCREEN_XDISPLAY(screen),
                       xpm,
                       &root_return,
                       &x, &y, &width, &height, &bw, &depth_ret))
        {
          surface = cairo_xlib_surface_create(GDK_SCREEN_XDISPLAY (screen),
                                              xpm,
                                              GDK_VISUAL_XVISUAL(gdk_screen_get_system_visual(screen)),
                                              width, height);
        }
      gdk_error_trap_pop_ignored ();
      
      cairo_pattern_t *pattern = cairo_pattern_create_for_surface(surface);
      gdk_window_set_background_pattern(window, pattern);

      cairo_surface_destroy(surface);
      // cairo_pattern_destroy      ???
#else
      GdkPixmap *gpm = gdk_pixmap_foreign_new(xpm);
      gdk_window_set_back_pixmap (window, gpm, FALSE);
      g_object_unref (gpm);
#endif      
    }
}

#endif
