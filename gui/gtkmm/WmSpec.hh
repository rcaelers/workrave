// WmSpec.hh 
//
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
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

#ifndef WMSPEC_HH
#define WMSPEC_HH

#include <gtk/gtkwidget.h>

class WmSpec
{
public:
  static bool supported();
  static void change_state(GtkWidget *window, bool add, const char *state);
  static void set_window_hint(GtkWidget *gtk_window, const char *type);
};

#endif // WINDOWHINTS_HH
