#ifndef DESKTOP_WINDOW_H
#define DESKTOP_WINDOW_H

#if defined(PLATFORM_OS_UNIX)

#  include <gdk/gdk.h>

#  ifdef __cplusplus
extern "C"
{
#  endif

  void set_desktop_background(GdkWindow *window);

#  ifdef __cplusplus
}
#  endif

#endif

#endif /* DESKTOP_WINDOW_H */
