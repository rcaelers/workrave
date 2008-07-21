#ifndef DESKTOP_WINDOW_H
#define DESKTOP_WINDOW_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef PLATFORM_OS_UNIX

#include <gdk/gdkwindow.h>

#ifdef __cplusplus
extern "C" {
#endif

  void set_desktop_background(GdkWindow *window);

#ifdef __cplusplus
}
#endif

#endif

#endif /* DESKTOP_WINDOW_H */
