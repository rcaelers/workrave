// GtkmmGUI.cc --- Gtk-- GUI Creator
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-09-16 13:36:04 pennersr>
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GNOME
#include <gnome.h>
#endif
#include "GtkmmGUI.hh"
#include "GUI.hh"

GUIInterface *
GtkmmGUI::create(ControlInterface *c, int argc, char **argv)
{
#ifdef HAVE_GNOME
  // FIXME: check return type
  gnome_init("Workrave", VERSION, argc, argv);
#endif  
  return new GUI(c, argc, argv);
}
