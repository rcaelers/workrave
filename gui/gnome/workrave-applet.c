// BreakControl.cc
//
// Copyright (C) 2002 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

static enum { IDLE, POSITION, SIZE } state;

#include "nls.h"

#include <gnome.h>
#include <panel-applet.h>

static int panel_size = 48;
static gboolean panel_vertical = FALSE;

static void
about(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  static const char *authors[] =
    { "Rob Caelers <robc@krandor.org",
      "Raymond Penners <raymond@dotsphinx.com",
      NULL
    };
  
  GtkWidget *about_box = NULL;
  
  about_box = gnome_about_new( _("Workrave Applet"), VERSION,
                               _("Copyright 2002 Rob Caelers, Raymond Penners"), 
                               _("Workrave Applet\n"),
                               authors,
                               NULL,
                               NULL,
                               NULL);
  
  gtk_widget_show(about_box);
}


static void
change_pixel_size(PanelApplet *applet, gint size, gpointer data)
{
  int x,y;
  GdkEventMotion m;
  
  gdk_window_get_pointer(NULL, &x, &y, NULL);
  m.x_root = x;
  m.y_root = y;
  
  panel_size = size;
}


static void
change_orient(PanelApplet *applet, PanelAppletOrient o, gpointer data)
{
  int x,y;
  GdkEventMotion m;
  gdk_window_get_pointer(NULL, &x, &y, NULL);
  m.x_root = x;
  m.y_root = y;

  if(o==PANEL_APPLET_ORIENT_UP || o==PANEL_APPLET_ORIENT_DOWN)
    panel_vertical = FALSE;
  else
    panel_vertical = TRUE;
}


static const BonoboUIVerb workrave_applet_verbs [] =
  {
    BONOBO_UI_VERB("About", about),
    BONOBO_UI_VERB_END
  };


static gboolean
workrave_applet_fill(PanelApplet *applet)
{
  GtkWidget *socket;
  
  socket = gtk_socket_new();
  gtk_container_add(GTK_CONTAINER(applet), socket);
  
  panel_size = panel_applet_get_size(applet);
  panel_applet_setup_menu_from_file(applet, NULL, "GNOME_WorkraveApplet.xml", NULL, workrave_applet_verbs, applet);
			   
  g_signal_connect(G_OBJECT(applet), "change_size", G_CALLBACK(change_pixel_size), NULL);
  g_signal_connect(G_OBJECT(applet), "change_orient", G_CALLBACK(change_orient), NULL);
		   
  gtk_widget_show(GTK_WIDGET(applet));
  
  return TRUE;
}


static gboolean
workrave_applet_factory(PanelApplet *applet, const gchar *iid, gpointer data)
{
  gboolean retval = FALSE;
    
  if (!strcmp(iid, "OAFIID:GNOME_WorkraveApplet"))
    {
      retval = workrave_applet_fill(applet); 
    }
  
  return retval;
}


PANEL_APPLET_BONOBO_FACTORY("OAFIID:GNOME_WorkraveApplet_Factory",
                            PANEL_TYPE_APPLET,
                            "Workrave Applet",
                            "0",
                            workrave_applet_factory,
                            NULL)
