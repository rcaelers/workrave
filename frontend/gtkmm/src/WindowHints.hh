// WindowHints.hh
//
// Copyright (C) 2001, 2002, 2003, 2007, 2008, 2011 Rob Caelers & Raymond Penners
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

#ifndef WINDOWHINTS_HH
#define WINDOWHINTS_HH

#include <gtk/gtk.h>

#ifdef PLATFORM_OS_WIN32
#include <windows.h>
#endif

namespace Gtk
{
  class Window;
}

class WindowHints
{
private:
public:
  typedef void *Grab;

  static void set_always_on_top(Gtk::Window *window, bool ontop);
  static Grab *grab(int num_windows, GdkWindow **window);
  static void ungrab(Grab *grab);
#if defined(PLATFORM_OS_WIN32)
  static void attach_thread_input(bool enabled);
#endif

#ifdef HAVE_GTK3
  static GdkDevice *keyboard, *pointer;  
#endif
};

#endif // WINDOWHINTS_HH
