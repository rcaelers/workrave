// Platform.hh --- Platform class
//
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
#include "config.h"
#endif

#include "utils/Platform.hh"

#include <stdlib.h>

#ifdef HAVE_GTK
#include <glib.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#endif

#ifdef HAVE_QT5
#include <QtGui>
#include <qdesktopwidget.h>
#include <qapplication.h>
#include <qpa/qplatformnativeinterface.h>
#endif

#ifdef PLATFORM_OS_UNIX

using namespace workrave::utils;

void *
Platform::get_default_display()
{
  void *xdisplay = NULL;

#if defined(HAVE_GTK)
  GdkDisplay *display = gdk_display_get_default();
  if (display != NULL)
    {
      xdisplay = gdk_x11_display_get_xdisplay(display);
    }
#elif defined(HAVE_QT5)
  QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
  if (native != NULL)
    {
      xdisplay = native->nativeResourceForScreen("display", QGuiApplication::primaryScreen());
    }
#endif

    return xdisplay;
}

unsigned long 
Platform::get_default_root_window()
{
#if defined(HAVE_GTK)
  return gdk_x11_get_default_root_xwindow();
#elif defined(HAVE_QT5)
    QDesktopWidget *desktop = QApplication::desktop();
    QWindow *window = desktop->windowHandle();
    return window->winId();
#endif
}
          
#endif
