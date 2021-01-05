// Copyright (C) 2001 - 2008, 2011, 2013 Rob Caelers & Raymond Penners
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

#include "WindowHints.hh"

#include "debug.hh"
#include "utils/Platform.hh"

#ifdef PLATFORM_OS_WINDOWS_NATIVE
#  undef max
#endif

#include <gtkmm/window.h>

#ifdef PLATFORM_OS_WINDOWS
#  include <windows.h>
#  include <gtk/gtk.h>
#  include <gdk/gdkwin32.h>

#  include "W32Compat.hh"
#endif

void
WindowHints::set_always_on_top(Gtk::Window *window, bool on_top)
{
#if defined(PLATFORM_OS_WINDOWS)

  HWND hwnd = (HWND)GDK_WINDOW_HWND(gtk_widget_get_window(GTK_WIDGET(window->gobj())));
  W32Compat::SetWindowOnTop(hwnd, on_top);

#else

  window->set_keep_above(on_top);

#endif
}
