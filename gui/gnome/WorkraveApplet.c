// WorkraveApplet.cc
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

#include <gnome.h>
#include <panel-applet.h>

#include "WorkraveAppletControl.h"

#include "nls.h"

/************************************************************************/

static int panel_size = 48;
static gboolean panel_vertical = FALSE;
static GtkWidget *panel_image = NULL;
static GtkWidget *panel_socket = NULL;

long workrave_applet_socket_id = 0;

/************************************************************************/

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

static const BonoboUIVerb
workrave_applet_verbs [] =
  {
    BONOBO_UI_VERB("About", about),
    BONOBO_UI_VERB_END
  };



static void
change_pixel_size(PanelApplet *applet, gint size, gpointer data)
{
  panel_size = size;
}


static void
change_orient(PanelApplet *applet, PanelAppletOrient o, gpointer data)
{
  if(o==PANEL_APPLET_ORIENT_UP || o==PANEL_APPLET_ORIENT_DOWN)
    panel_vertical = FALSE;
  else
    panel_vertical = TRUE;
}

static gboolean
plug_removed (GtkSocket *socket, void *manager)
{
  gtk_widget_show(GTK_WIDGET(panel_image));
  gtk_widget_hide(GTK_WIDGET(socket));
  return TRUE;
}


static gboolean
plug_added (GtkSocket *socket, void *manager)
{
  gtk_widget_hide(GTK_WIDGET(panel_image));
  gtk_widget_show(GTK_WIDGET(socket));
  return TRUE;
}


static gboolean
workrave_applet_fill(PanelApplet *applet)
{
  GdkPixbuf *pixbuf;
  GtkWidget *hbox;
  GdkNativeWindow wnd;
  
  //
  panel_size = panel_applet_get_size(applet);
  panel_applet_setup_menu_from_file(applet, NULL, "GNOME_WorkraveApplet.xml", NULL, workrave_applet_verbs, applet);
  
  // Socket.
  panel_socket = gtk_socket_new();
  
  // Image
  pixbuf = gdk_pixbuf_new_from_file(WORKRAVE_DATADIR "/images/workrave.png", NULL);  
  panel_image = gtk_image_new_from_pixbuf(pixbuf);
  gtk_widget_show(GTK_WIDGET(panel_image));

  // Signals.
  g_signal_connect(panel_socket, "plug_removed", G_CALLBACK(plug_removed), NULL);
  g_signal_connect(panel_socket, "plug_added", G_CALLBACK(plug_added), NULL);
  g_signal_connect(G_OBJECT(applet), "change_size", G_CALLBACK(change_pixel_size), NULL);
  g_signal_connect(G_OBJECT(applet), "change_orient", G_CALLBACK(change_orient), NULL);

  // Container.
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(applet), hbox);
  gtk_box_pack_end(GTK_BOX(hbox), panel_socket, FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(hbox), panel_image, FALSE, FALSE, 0);

  gtk_widget_show(GTK_WIDGET(hbox));
  gtk_widget_show(GTK_WIDGET(applet));

  wnd = gtk_socket_get_id(GTK_SOCKET(panel_socket));
  workrave_applet_socket_id = wnd;
  
  return TRUE;
}


static gboolean
workrave_applet_factory(PanelApplet *applet, const gchar *iid, gpointer data)
{
  gboolean retval = FALSE;
  AppletControl *ctrl = NULL;
  
  if (!strcmp(iid, "OAFIID:GNOME_WorkraveApplet"))
    {
      retval = workrave_applet_fill(applet); 
    }

  ctrl = workrave_applet_new();
  
  return retval;
}


PANEL_APPLET_BONOBO_FACTORY("OAFIID:GNOME_WorkraveApplet_Factory",
                            PANEL_TYPE_APPLET,
                            "Workrave Applet",
                            "0",
                            workrave_applet_factory,
                            NULL)
