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

#include "Workrave-Control.h"
#include "WorkraveApplet.h"
#include "nls.h"

static AppletControl *applet_control = NULL;
static GNOME_Workrave_AppletControl remote_control = NULL;

/************************************************************************/
/* GNOME::AppletControl                                                 */
/************************************************************************/

static BonoboObjectClass *parent_class = NULL;

BONOBO_TYPE_FUNC_FULL(AppletControl,
                      GNOME_Workrave_AppletControl,
                      BONOBO_OBJECT_TYPE,
                      workrave_applet_control);

static gboolean
workrave_applet_connect(gboolean start)
{
  CORBA_Environment ev;
  gboolean ok = TRUE;
  Bonobo_ActivationFlags flags = 0;

  if (remote_control != NULL)
    {
      // ping.
      
      CORBA_exception_init(&ev);
      Bonobo_Unknown_ref(remote_control, &ev);
      if (BONOBO_EX(&ev))
        {
          remote_control = NULL;
        }
      else
        {
          Bonobo_Unknown_unref(remote_control, &ev);
          if (BONOBO_EX(&ev))
            {
              remote_control = NULL;
            }
        }
      CORBA_exception_free(&ev);
    }

  if (remote_control == NULL)
    {
      bonobo_activate();

      CORBA_exception_init(&ev);
      
      if (start == FALSE)
        {
          flags = Bonobo_ACTIVATION_FLAG_EXISTING_ONLY;
        }
      
      remote_control = bonobo_activation_activate_from_id("OAFIID:GNOME_Workrave_WorkraveControl", flags,
                                                          NULL, &ev);
      
      if (remote_control == NULL || BONOBO_EX(&ev))
        {
          g_warning(_("Could not contact Workrave Panel"));
          ok = FALSE;
        }
      
      CORBA_exception_free(&ev);
    }
  
  return ok;
}


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
  epv->set_menu_status = workrave_applet_control_set_menu_status;
  epv->get_menu_status = workrave_applet_control_get_menu_status;
}


static void
workrave_applet_control_init(AppletControl *applet)
{
  applet->image = NULL;
  applet->socket = NULL;

  applet->size = 48;
  applet->socket_id = 0;
  applet->vertical = FALSE;

  applet->last_showlog_state = FALSE;
  applet->last_mode = GNOME_Workrave_WorkraveControl_MODE_INVALID;
  
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


void
workrave_applet_control_set_menu_status(PortableServer_Servant servant, const CORBA_char *name,
                                        const CORBA_boolean status, CORBA_Environment *ev)
{
  AppletControl *applet_control = WR_APPLET_CONTROL(bonobo_object_from_servant(servant));
  
  BonoboUIComponent *ui = NULL;
  PanelApplet *applet = NULL;
  gboolean set = FALSE;
  
  if (applet_control != NULL)
    {
      applet = applet_control->applet;
    }

  if (applet != NULL)
    {
      ui = panel_applet_get_popup_component(applet);
    }
  
  if (ui != NULL)
    {
      const char *s = bonobo_ui_component_get_prop(ui, name, "state", NULL);

      set = (s != NULL && atoi(s) != 0);
    }

  if ((status && !set) || (!status && set))
    {
      bonobo_ui_component_set_prop(ui, name, "state", status ? "1" : "0", NULL);
    }
}

static CORBA_boolean
workrave_applet_control_get_menu_status(PortableServer_Servant servant, const CORBA_char *name,
                                        CORBA_Environment *ev)
{
  AppletControl *applet_control = WR_APPLET_CONTROL(bonobo_object_from_servant(servant));
  
  BonoboUIComponent *ui = NULL;
  PanelApplet *applet = NULL;
  gboolean b;
  CORBA_boolean ret = FALSE;
    
  if (applet_control != NULL)
    {
      applet = applet_control->applet;
    }

  if (applet != NULL)
    {
      ui = panel_applet_get_popup_component(applet);
    }
  
  if (ui != NULL)
    {
      const char *s = bonobo_ui_component_get_prop(ui, name, "state", NULL);

      ret = (s != NULL && atoi(s) != 0);
    }

  return ret;
}




gboolean
workrave_applet_fire_workrave()
{
  CORBA_Environment ev;
  gboolean ok = TRUE;
  
  bonobo_activate();

  CORBA_exception_init(&ev);
  remote_control = bonobo_activation_activate_from_id("OAFIID:GNOME_Workrave_WorkraveControl", 0, NULL, &ev);
  
  if (remote_control == NULL || BONOBO_EX(&ev))
    {
      g_warning(_("Could not contact Workrave Panel"));
      ok = FALSE;
    }
  

  if (ok)
    {
      GNOME_Workrave_WorkraveControl_fire(remote_control, &ev);

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
verb_about(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
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
verb_open(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  workrave_applet_connect(FALSE);
      
  if (remote_control != NULL)
    {
      CORBA_Environment ev;
      CORBA_exception_init(&ev);
      
      GNOME_Workrave_WorkraveControl_open_main(remote_control, &ev);

      if (BONOBO_EX(&ev))
        {
          char *err = (char *) bonobo_exception_get_text(&ev);
          g_warning (_("An exception occured '%s'"), err);
          g_free(err);
        }

      CORBA_exception_free(&ev);
    }
}


static void
verb_preferences(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  workrave_applet_connect(FALSE);
      
  if (remote_control != NULL)
    {
      CORBA_Environment ev;
      CORBA_exception_init(&ev);
     
      GNOME_Workrave_WorkraveControl_open_preferences(remote_control, &ev);

      if (BONOBO_EX(&ev))
        {
          char *err = (char *) bonobo_exception_get_text(&ev);
          g_warning (_("An exception occured '%s'"), err);
          g_free(err);
        }
      CORBA_exception_free(&ev);
    }

}


static void
verb_restbreak(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  workrave_applet_connect(FALSE);
      
  if (remote_control != NULL)
    {
      CORBA_Environment ev;
      CORBA_exception_init(&ev);
      
      GNOME_Workrave_WorkraveControl_restbreak(remote_control, &ev);

      if (BONOBO_EX(&ev))
        {
          char *err = (char *) bonobo_exception_get_text(&ev);
          g_warning (_("An exception occured '%s'"), err);
          g_free(err);
        }
      CORBA_exception_free(&ev);
    }
}



static void
verb_connect(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  workrave_applet_connect(FALSE);
      
  if (remote_control != NULL)
    {
      CORBA_Environment ev;
      CORBA_exception_init(&ev);
      
      GNOME_Workrave_WorkraveControl_open_network_connect(remote_control, &ev);

      if (BONOBO_EX(&ev))
        {
          char *err = (char *) bonobo_exception_get_text(&ev);
          g_warning (_("An exception occured '%s'"), err);
          g_free(err);
        }
      CORBA_exception_free(&ev);
    }
}


static void
verb_disconnect(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  workrave_applet_connect(FALSE);
      
  if (remote_control != NULL)
    {
      CORBA_Environment ev;
      CORBA_exception_init(&ev);
      
      GNOME_Workrave_WorkraveControl_disconnect_all(remote_control, &ev);

      if (BONOBO_EX(&ev))
        {
          char *err = (char *) bonobo_exception_get_text(&ev);
          g_warning (_("An exception occured '%s'"), err);
          g_free(err);
        }
      CORBA_exception_free(&ev);
    }
}

static void
verb_reconnect(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  workrave_applet_connect(FALSE);
      
  if (remote_control != NULL)
    {
      CORBA_Environment ev;
      CORBA_exception_init(&ev);
      
      GNOME_Workrave_WorkraveControl_reconnect_all(remote_control, &ev);

      if (BONOBO_EX(&ev))
        {
          char *err = (char *) bonobo_exception_get_text(&ev);
          g_warning (_("An exception occured '%s'"), err);
          g_free(err);
        }
      CORBA_exception_free(&ev);
    }
}



static void
verb_quit(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  workrave_applet_connect(FALSE);
      
  if (remote_control != NULL)
    {
      CORBA_Environment ev;
      CORBA_exception_init(&ev);
      
      GNOME_Workrave_WorkraveControl_quit(remote_control, &ev);

      if (BONOBO_EX(&ev))
        {
          char *err = (char *) bonobo_exception_get_text(&ev);
          g_warning (_("An exception occured '%s'"), err);
          g_free(err);
        }
      CORBA_exception_free(&ev);
    }
}

static const BonoboUIVerb
workrave_applet_verbs [] =
  {
    BONOBO_UI_VERB("About", verb_about),
    BONOBO_UI_VERB("Open", verb_open),
    BONOBO_UI_VERB("Prefereces", verb_preferences),
    BONOBO_UI_VERB("Restbreak", verb_restbreak),
    BONOBO_UI_VERB("Connect", verb_connect),
    BONOBO_UI_VERB("Disconnect", verb_disconnect),
    BONOBO_UI_VERB("Reconnect", verb_reconnect),
    BONOBO_UI_VERB("Quit", verb_quit),
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


static void
showlog_callback(BonoboUIComponent *ui, const char *path, Bonobo_UIComponent_EventType type,
                 const char *state, gpointer user_data)
{
  gboolean new_state;
  FILE *file = fopen("/home/robc/wr.txt", "a+");
  workrave_applet_connect(FALSE);


  fprintf(file, "%s %s\n", path, state);
  fclose(file);
  
  if (state == NULL || strcmp(state, "") == 0)
    {
      /* State goes blank when component is removed; ignore this. */
      return;
    }

  new_state = strcmp (state, "0") != 0;

  if (applet_control->last_showlog_state != new_state)
    {
      applet_control->last_showlog_state = new_state;
      
      if (remote_control != NULL)
        {
          CORBA_Environment ev;
          CORBA_exception_init(&ev);
          
          GNOME_Workrave_WorkraveControl_open_network_log(remote_control, new_state, &ev);
          
          if (BONOBO_EX(&ev))
            {
              char *err = (char *) bonobo_exception_get_text(&ev);
              g_warning (_("An exception occured '%s'"), err);
              g_free(err);
            }
          
          CORBA_exception_free(&ev);
        }
    }
}



static void
mode_callback(BonoboUIComponent *ui, const char *path, Bonobo_UIComponent_EventType type,
                 const char *state, gpointer user_data)
{
  gboolean new_state;
  FILE *file = fopen("/home/robc/wr.txt", "a+");

  fprintf(file, "%s %s\n", path, state);
  fclose(file);
  
  workrave_applet_connect(FALSE);

  if (state == NULL || strcmp(state, "") == 0)
    {
      /* State goes blank when component is removed; ignore this. */
      return;
    }

  new_state = strcmp (state, "0") != 0;
  
  workrave_applet_connect(FALSE);
      
  if (remote_control != NULL && path != NULL && new_state)
    {
      CORBA_Environment ev;
      GNOME_Workrave_WorkraveControl_Mode mode = GNOME_Workrave_WorkraveControl_MODE_INVALID;

      CORBA_exception_init(&ev);
        
      if (g_ascii_strcasecmp(path, "Normal") == 0)
        {
          mode = GNOME_Workrave_WorkraveControl_MODE_NORMAL;
        }
      else if (g_ascii_strcasecmp(path, "Suspended") == 0)
        {
          mode = GNOME_Workrave_WorkraveControl_MODE_SUSPENDED;
        }
      else if (g_ascii_strcasecmp(path, "Quiet") == 0)
        {
          mode = GNOME_Workrave_WorkraveControl_MODE_QUIET;
        }

      if (mode != GNOME_Workrave_WorkraveControl_MODE_INVALID &&
          mode != applet_control->last_mode)
        {
          applet_control->last_mode = mode;
          GNOME_Workrave_WorkraveControl_set_mode(remote_control, mode, &ev);
          
          if (BONOBO_EX(&ev))
            {
              char *err = (char *) bonobo_exception_get_text(&ev);
              g_warning (_("An exception occured '%s'"), err);
              g_free(err);
            }
        }
      
      CORBA_exception_free(&ev);
    }
}


static gboolean
workrave_applet_fill(PanelApplet *applet)
{
  GdkPixbuf *pixbuf;
  GtkWidget *hbox;
  GdkNativeWindow wnd;
  BonoboUIComponent *ui = NULL;
  
  //
  applet_control->size = panel_applet_get_size(applet);
  panel_applet_setup_menu_from_file(applet, NULL, "GNOME_WorkraveApplet.xml", NULL, workrave_applet_verbs, applet);


  ui = panel_applet_get_popup_component(applet);
  bonobo_ui_component_add_listener(ui, "ShowLog", showlog_callback, NULL);
  bonobo_ui_component_add_listener(ui, "Normal", mode_callback, NULL);
  bonobo_ui_component_add_listener(ui, "Suspended", mode_callback, NULL);
  bonobo_ui_component_add_listener(ui, "Quiet", mode_callback, NULL);

  
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
  applet_control->applet = applet;

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
