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
#include <bonobo.h>
#include <bonobo/bonobo-object.h>
#include <panel-applet.h>

#include "WorkraveApplet.h"
#include "nls.h"

static AppletControl *applet_control = NULL;

/************************************************************************/
/* GNOME::AppletControl                                                 */
/************************************************************************/

static BonoboObjectClass *parent_class = NULL;

BONOBO_TYPE_FUNC_FULL(AppletControl,
                      GNOME_Workrave_AppletControl,
                      BONOBO_OBJECT_TYPE,
                      workrave_applet_control);


static void
workrave_applet_control_class_init(AppletControlClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  POA_GNOME_Workrave_AppletControl__epv *epv = &klass->epv;

  parent_class = g_type_class_peek_parent(klass);
  
  // object_class->dispose = pas_book_dispose;
          
  epv->get_socket_id = workrave_applet_control_get_socket_id;
  epv->get_size = workrave_applet_control_get_size;
  epv->get_vertical = workrave_applet_control_get_vertical;
}

static void
workrave_applet_control_init(AppletControl *applet)
{
  applet->image = NULL;
  applet->socket = NULL;

  applet->size = 48;
  applet->socket_id = 0;
  applet->vertical = FALSE;
}


AppletControl*
workrave_applet_control_new(void)
{
  Bonobo_RegistrationResult result;

  AppletControl *applet_control = g_object_new(workrave_applet_control_get_type(), NULL);
  BonoboObject *object = BONOBO_OBJECT(applet_control);
    
  result = bonobo_activation_active_server_register("OAFIID:GNOME_Workrave_AppletControl",
                                                    bonobo_object_corba_objref(BONOBO_OBJECT(object)));
  
  return applet_control;
}


CORBA_long     
workrave_applet_control_get_socket_id(PortableServer_Servant servant, CORBA_Environment *ev)
{
  AppletControl *applet_control = WR_APPLET_CONTROL(bonobo_object_from_servant(servant));

  return applet_control->socket_id;
}

CORBA_long     
workrave_applet_control_get_size(PortableServer_Servant servant, CORBA_Environment *ev)
{
  AppletControl *applet_control = WR_APPLET_CONTROL(bonobo_object_from_servant(servant));

  return applet_control->size;
}

CORBA_boolean
workrave_applet_control_get_vertical(PortableServer_Servant servant, CORBA_Environment *ev)
{
  AppletControl *applet_control = WR_APPLET_CONTROL(bonobo_object_from_servant(servant));

  return applet_control->vertical;
}

gboolean
workrave_applet_fire_workrave()
{
  GNOME_Workrave_AppletControl ctrl;
  CORBA_Environment ev;
  gboolean ok = TRUE;
  
  bonobo_activate();

  CORBA_exception_init(&ev);
  ctrl = bonobo_activation_activate_from_id("OAFIID:GNOME_Workrave_WorkraveControl", 0, NULL, &ev);
  
  if (ctrl == NULL || BONOBO_EX (&ev))
    {
      g_warning(_("Could not contact Workrave Panel"));
      ok = FALSE;
    }
  

  if (ok)
    {
      GNOME_Workrave_WorkraveControl_fire(ctrl, &ev);

      if (BONOBO_EX (&ev))
        {
          char *err = (char *) bonobo_exception_get_text(&ev);
          g_warning (_("An exception occured '%s'"), err);
          g_free(err);
          ok = FALSE;
        }
    }

  CORBA_exception_free(&ev);
  return ok;
}


/************************************************************************/
/* GNOME::Applet                                                        */
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
  applet_control->size = size;
}


static void
change_orient(PanelApplet *applet, PanelAppletOrient o, gpointer data)
{
  if(o==PANEL_APPLET_ORIENT_UP || o==PANEL_APPLET_ORIENT_DOWN)
    applet_control->vertical = FALSE;
  else
    applet_control->vertical = TRUE;
}

static gboolean
plug_removed(GtkSocket *socket, void *manager)
{
  gtk_widget_show(GTK_WIDGET(applet_control->image));
  gtk_widget_hide(GTK_WIDGET(applet_control->socket));
  return TRUE;
}


static gboolean
plug_added(GtkSocket *socket, void *manager)
{
  gtk_widget_hide(GTK_WIDGET(applet_control->image));
  gtk_widget_show(GTK_WIDGET(applet_control->socket));
  return TRUE;
}


static gboolean
workrave_applet_fill(PanelApplet *applet)
{
  GdkPixbuf *pixbuf;
  GtkWidget *hbox;
  GdkNativeWindow wnd;
  
  //
  applet_control->size = panel_applet_get_size(applet);
  panel_applet_setup_menu_from_file(applet, NULL, "GNOME_WorkraveApplet.xml", NULL, workrave_applet_verbs, applet);
  
  // Socket.
  applet_control->socket = gtk_socket_new();
  
  // Image
  pixbuf = gdk_pixbuf_new_from_file(WORKRAVE_DATADIR "/images/workrave.png", NULL);  
  applet_control->image = gtk_image_new_from_pixbuf(pixbuf);
  gtk_widget_show(GTK_WIDGET(applet_control->image));

  // Signals.
  g_signal_connect(applet_control->socket, "plug_removed", G_CALLBACK(plug_removed), NULL);
  g_signal_connect(applet_control->socket, "plug_added", G_CALLBACK(plug_added), NULL);
  g_signal_connect(G_OBJECT(applet), "change_size", G_CALLBACK(change_pixel_size), NULL);
  g_signal_connect(G_OBJECT(applet), "change_orient", G_CALLBACK(change_orient), NULL);

  // Container.
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(applet), hbox);
  gtk_box_pack_end(GTK_BOX(hbox), applet_control->socket, FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(hbox), applet_control->image, FALSE, FALSE, 0);

  gtk_widget_show(GTK_WIDGET(hbox));
  gtk_widget_show(GTK_WIDGET(applet));

  applet_control->socket_id = gtk_socket_get_id(GTK_SOCKET(applet_control->socket));
  
  return TRUE;
}


static gboolean
workrave_applet_factory(PanelApplet *applet, const gchar *iid, gpointer data)
{
  gboolean retval = FALSE;
  
  applet_control = workrave_applet_control_new();

  if (!strcmp(iid, "OAFIID:GNOME_WorkraveApplet"))
    {
      retval = workrave_applet_fill(applet); 
    }

  workrave_applet_fire_workrave();
  
  return retval;
}


PANEL_APPLET_BONOBO_FACTORY("OAFIID:GNOME_WorkraveApplet_Factory",
                            PANEL_TYPE_APPLET,
                            "Workrave Applet",
                            "0",
                            workrave_applet_factory,
                            NULL)
