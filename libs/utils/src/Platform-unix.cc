// Copyright (C) 2013 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "utils/Platform.hh"

#if defined(HAVE_GTK)
#  include <glib.h>
#  include <gdk/gdk.h>
#  if defined(PLATFORM_OS_UNIX)
#    include <gdk/gdkx.h>
#  endif
#  if defined(GDK_WINDOWING_WAYLAND)
#    include <gdk/gdkwayland.h>
#  endif
#endif

#if defined(HAVE_QT)
#  include <QtGui>
#  include <qapplication.h>

#  if defined(PLATFORM_OS_UNIX)
#    include <X11/Xlib.h>
#  endif
#endif

using namespace workrave::utils;

#if defined(PLATFORM_OS_UNIX)
void *
Platform::get_default_display()
{
  void *xdisplay = nullptr;

#  if defined(HAVE_GTK)
  GdkDisplay *display = gdk_display_get_default();
  if (display != nullptr && GDK_IS_X11_DISPLAY(display))
    {
      xdisplay = gdk_x11_display_get_xdisplay(display);
    }
#  elif defined(HAVE_QT)
  QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
  if (native != NULL)
    {
      xdisplay = native->nativeResourceForScreen("display", QGuiApplication::primaryScreen());
    }
#  else
#    error Platform unsupported
#  endif

  return xdisplay;
}

std::string
Platform::get_default_display_name()
{
  std::string ret;

#  if defined(HAVE_GTK)
  GdkDisplay *display = gdk_display_get_default();
  if (display != nullptr)
    {
      const gchar *name = gdk_display_get_name(display);
      if (name != nullptr)
        {
          ret = name;
        }
    }
#  elif defined(HAVE_QT)
  QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
  if (native != NULL)
    {
      void *xdisplay = native->nativeResourceForScreen("display", QGuiApplication::primaryScreen());
      char *name = XDisplayString(static_cast<Display *>(xdisplay));
      if (name != NULL)
        {
          ret = name;
        }
    }
#  else
#    error Platform unsupported
#  endif
  return ret;
}

unsigned long
Platform::get_default_root_window()
{
#  if defined(HAVE_GTK)
  return gdk_x11_get_default_root_xwindow();
#  elif defined(HAVE_QT)
  QDesktopWidget *desktop = QApplication::desktop();
  QWindow *window = desktop->windowHandle();
  return window->winId();
#  else
#    error Platform unsupported
#  endif
}
#endif

int
Platform::setenv(const char *name, const char *val, int overwrite)
{
  return ::setenv(name, val, overwrite);
}

int
Platform::unsetenv(const char *name)
{
  return ::unsetenv(name);
}

#if defined(PLATFORM_OS_UNIX)
bool
Platform::running_on_wayland()
{
#  if defined(GDK_WINDOWING_WAYLAND)
  GdkDisplay *display = gdk_display_manager_get_default_display(gdk_display_manager_get());
  return GDK_IS_WAYLAND_DISPLAY(display);
#  else
  return false;
#  endif
}
#endif

bool
Platform::can_position_windows()
{
#if defined(PLATFORM_OS_UNIX)
  return !running_on_wayland();
#elif defined(PLATFORM_OS_MACOS)
  return true;
#else
#  error Unknown platform
#endif
}
