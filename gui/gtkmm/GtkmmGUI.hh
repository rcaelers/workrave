// GtkmmGUI.hh --- Gtk-- GUI Creator
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-08-11 16:36:00 robc>
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

#ifndef GTKMMGUI_HH
#define GTKMMGUI_HH

#include "GUIInterface.hh"

class ControlInterface;

class GtkmmGUI
{
public:
  static GUIInterface *create(ControlInterface *c, int argc, char **argv);
};

#endif // GTKMMGUI_HH
