// GUIFactory.cc
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2003-04-11 22:54:50 robc>
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

#include "GUIFactory.hh"

#ifdef HAVE_GUI_GTK
#include "gtkmm/GtkmmGUI.hh"
#endif

#ifdef HAVE_GUI_TEXT
#include "text/TextGUI.hh"
#endif


GUIInterface *
GUIFactory::create_gui(string type, ControlInterface *c, int argc, char **argv)
{
  GUIInterface *gi =  NULL;
  (void) type;
  
#ifdef HAVE_GUI_GTK
  // if (type == "gtkmm")
    {
      gi = GtkmmGUI::create(c, argc, argv);
    }
#endif 
#ifdef HAVE_GUI_TEXT
  //  if (type == "text")
    {
      gi = TextGUI::create(c, argc, argv);
    }
#endif
 
  return gi;
}

  
