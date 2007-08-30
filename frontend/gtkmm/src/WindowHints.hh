// WindowHints.hh
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef WINDOWHINTS_HH
#define WINDOWHINTS_HH

#include <gtk/gtkwidget.h>

#ifdef WIN32
#include <windows.h>
#endif

class WindowHints
{
private:
#if defined(HAVE_X)
  enum hint_type { HINTTYPE_NONE, HINTTYPE_WIN, HINTTYPE_NET };
  static hint_type type;
  static bool net_supported;
  static bool win_supported;
#elif defined(WIN32)

#endif

public:
  typedef void *Grab;

  static bool init();
  static bool set_always_on_top(GtkWidget *window, bool onTop);
  static bool set_skip_winlist(GtkWidget *window, bool skip);
  static bool set_tool_window(GtkWidget *window, bool istool);
  static Grab *grab(int num_windows, GdkWindow **window);
  static void ungrab(Grab *grab);
#if defined(WIN32)
  static void attach_thread_input(bool enabled);
#endif
};

#endif // WINDOWHINTS_HH
