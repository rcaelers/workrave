// WorkraveApplet.cc
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007 Rob Caelers & Raymond Penners
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

#include "credits.h"

#include <gnome.h>
#include <bonobo.h>
#include <bonobo/bonobo-object.h>
#include <panel-applet.h>

#include <gtk/gtkaboutdialog.h>
#include <gdk/gdkx.h>

#include "Workrave-Control.h"
#include "WorkraveApplet.h"
#include "nls.h"

static AppletControl *applet_control = NULL;
static GNOME_Workrave_AppletControl remote_control = NULL;

static void
workrave_applet_hide_menus(gboolean hide);
static void
workrave_applet_set_hidden(gchar *name, gboolean hidden);

/************************************************************************/
/* GNOME::AppletControl                                                 */
/************************************************************************/

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
          ok = FALSE;
        }
      else
        {
          Bonobo_Unknown_unref(remote_control, &ev);
          if (BONOBO_EX(&ev))
            {
              remote_control = NULL;
              ok = FALSE;
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
      
      remote_control = bonobo_activation_activate_from_id("OAFIID:GNOME_Workrave_WorkraveControl",
                                                          flags, NULL, &ev);
      
      if (remote_control == NULL || BONOBO_EX(&ev))
        {
          ok = FALSE;
        }
      
      CORBA_exception_free(&ev);
    }

  //workrave_applet_hide_menus(!ok);
  
  return ok;
}


static void
workrave_applet_control_class_init(AppletControlClass *klass)
{
  POA_GNOME_Workrave_AppletControl__epv *epv = &klass->epv;

  epv->get_socket_id = workrave_applet_control_get_socket_id;
  epv->get_size = workrave_applet_control_get_size;
  epv->get_vertical = workrave_applet_control_get_vertical;
  epv->set_menu_status = workrave_applet_control_set_menu_status;
  epv->get_menu_status = workrave_applet_control_get_menu_status;
  epv->set_menu_active = workrave_applet_control_set_menu_active;
  epv->get_menu_active = workrave_applet_control_get_menu_active;
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
 
      if ((status && !set) || (!status && set))
        {
          bonobo_ui_component_set_prop(ui, name, "state", status ? "1" : "0", NULL);
        }
    }
}


static CORBA_boolean
workrave_applet_control_get_menu_status(PortableServer_Servant servant, const CORBA_char *name,
                                        CORBA_Environment *ev)
{
  AppletControl *applet_control = WR_APPLET_CONTROL(bonobo_object_from_servant(servant));
  
  BonoboUIComponent *ui = NULL;
  PanelApplet *applet = NULL;
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


void
workrave_applet_control_set_menu_active(PortableServer_Servant servant, const CORBA_char *name,
                                        const CORBA_boolean status, CORBA_Environment *ev)
{
  workrave_applet_set_hidden((char *) name, !status); 
}


static CORBA_boolean
workrave_applet_control_get_menu_active(PortableServer_Servant servant, const CORBA_char *name,
                                        CORBA_Environment *ev)
{
  AppletControl *applet_control = WR_APPLET_CONTROL(bonobo_object_from_servant(servant));
  
  BonoboUIComponent *ui = NULL;
  PanelApplet *applet = NULL;
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
      const char *s = bonobo_ui_component_get_prop(ui, name, "hidden", NULL);

      ret = (s != NULL && atoi(s) != 0);
    }

  return ret;
}


/* static CORBA_boolean */
/* workrave_applet_control_register_control(PortableServer_Servant servant, */
/*                                          const Bonobo_Unknown control, */
/*                                          CORBA_Environment *ev) */
/* { */
/*   //AppletControl *applet_control = WR_APPLET_CONTROL(bonobo_object_from_servant(servant)); */
/*   return TRUE; */
/* } */


/* static CORBA_boolean */
/* workrave_applet_control_unregister_control(PortableServer_Servant servant, */
/*                                            const Bonobo_Unknown control, */
/*                                            CORBA_Environment *ev) */
/* { */
/*   //AppletControl *applet_control = WR_APPLET_CONTROL(bonobo_object_from_servant(servant)); */
/*   return TRUE; */
/* }  */


gboolean
workrave_applet_fire_workrave()
{
  gboolean ok = FALSE;
  workrave_applet_connect(TRUE);
      
  if (remote_control != NULL)
    {
      CORBA_Environment ev;
      CORBA_exception_init(&ev);
      
      GNOME_Workrave_WorkraveControl_set_applet(remote_control,
                                                bonobo_object_corba_objref(BONOBO_OBJECT(applet_control)),
                                                &ev);

      if (BONOBO_EX(&ev))
        {
          char *err = (char *) bonobo_exception_get_text(&ev);
          g_warning (_("An exception occured '%s'"), err);
          g_free(err);
        }
      else
        {
          ok = TRUE;
        }
      
      CORBA_exception_free(&ev);
    }

  return ok;
}


/************************************************************************/
/* GNOME::Applet                                                        */
/************************************************************************/


static void
verb_about(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(WORKRAVE_DATADIR "/images/workrave.png", NULL);  
  GtkAboutDialog *about = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
  
  gtk_container_set_border_width(GTK_CONTAINER(about), 5);


  gtk_show_about_dialog (NULL,
			 "name", "Workrave",
			 "version", VERSION,
			 "copyright", workrave_copyright,
			 "website", "http://www.workrave.org",
			 "website_label", "www.workrave.org",
			 "comments", _("This program assists in the prevention and recovery"
                                  " of Repetitive Strain Injury (RSI)."),
                         "translator-credits", workrave_translators,
			 "authors", workrave_authors,
			 "logo", pixbuf,
			 NULL);
  g_object_unref(pixbuf);

  
  
/*   gtk_widget_show (gnome_about_new */
/*                    ("Workrave Applet", VERSION, */
/*                     "Copyright 2001-2006 Rob Caelers & Raymond Penners", */
/*                     _("This program assists in the prevention and recovery" */
/*                       " of Repetitive Strain Injury (RSI)."), */
/*                     (const gchar **) authors, */
/*                     (const gchar **) NULL, */
/*                     translators, */
/*                     pixbuf)); */
}


static void
verb_open(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  workrave_applet_connect(TRUE);
      
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
verb_exercises(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  workrave_applet_connect(FALSE);
      
  if (remote_control != NULL)
    {
      CORBA_Environment ev;
      CORBA_exception_init(&ev);
     
      GNOME_Workrave_WorkraveControl_open_exercises(remote_control, &ev);

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
verb_statistics(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  workrave_applet_connect(FALSE);
      
  if (remote_control != NULL)
    {
      CORBA_Environment ev;
      CORBA_exception_init(&ev);
     
      GNOME_Workrave_WorkraveControl_open_statistics(remote_control, &ev);

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
    BONOBO_UI_VERB("Exercises", verb_exercises),
    BONOBO_UI_VERB("Preferences", verb_preferences),
    BONOBO_UI_VERB("Restbreak", verb_restbreak),
    BONOBO_UI_VERB("Connect", verb_connect),
    BONOBO_UI_VERB("Disconnect", verb_disconnect),
    BONOBO_UI_VERB("Reconnect", verb_reconnect),
    BONOBO_UI_VERB("Statistics", verb_statistics),
    BONOBO_UI_VERB("Quit", verb_quit),
    BONOBO_UI_VERB_END
  };


static void
change_pixel_size(PanelApplet *applet, gint size, gpointer data)
{
  applet_control->size = size;

  workrave_applet_connect(FALSE);
  
  if (remote_control != NULL)
    {
      CORBA_Environment ev;
      CORBA_exception_init(&ev);
      
      GNOME_Workrave_WorkraveControl_set_applet_size(remote_control, size, &ev);
      
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
change_orient(PanelApplet *applet, PanelAppletOrient o, gpointer data)
{
  if(o==PANEL_APPLET_ORIENT_UP || o==PANEL_APPLET_ORIENT_DOWN)
    applet_control->vertical = FALSE;
  else
    applet_control->vertical = TRUE;

  workrave_applet_connect(FALSE);
  
  if (remote_control != NULL)
    {
      CORBA_Environment ev;
      CORBA_exception_init(&ev);
      
      GNOME_Workrave_WorkraveControl_set_applet_vertical(remote_control, applet_control->vertical, &ev);
      
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
change_background (PanelApplet * widget, 
                   PanelAppletBackgroundType type,
                   GdkColor * color,
                   GdkPixmap * pixmap,
                   void *data)
{
  static GdkPixmap *keep = NULL;
  long xid = 0;
  GNOME_Workrave_WorkraveControl_Color c;
  
  workrave_applet_connect(FALSE);
  
  if (remote_control != NULL)
    {
      CORBA_Environment ev;
      CORBA_exception_init(&ev);

      if (type == PANEL_COLOR_BACKGROUND && color != NULL)
        {
          c.pixel = color->pixel;
          c.red = color->red;
          c.green = color->green;
          c.blue = color->blue;
        }
      
      if (type == PANEL_PIXMAP_BACKGROUND)
        {
          if (keep != NULL)
            {
              gdk_pixmap_unref(keep);
              keep = pixmap;
            }
          if (pixmap != NULL)
            {
              gdk_pixmap_ref(pixmap);
            }
          
          xid = GDK_PIXMAP_XID(pixmap);
        }

      
      GNOME_Workrave_WorkraveControl_set_applet_background(remote_control,
                                                           type,
                                                           &c,
                                                           (CORBA_long) xid,
                                                           &ev);
      if (BONOBO_EX(&ev))
        {
          char *err = (char *) bonobo_exception_get_text(&ev);
          g_warning (_("An exception occured '%s'"), err);
          g_free(err);
        }
      
      CORBA_exception_free(&ev);
    }
}

static gboolean
plug_removed(GtkSocket *socket, void *manager)
{
  gtk_widget_show(GTK_WIDGET(applet_control->image));
  gtk_widget_hide(GTK_WIDGET(applet_control->socket));
  workrave_applet_hide_menus(TRUE);
  return TRUE;
}


static gboolean
plug_added(GtkSocket *socket, void *manager)
{
  gtk_widget_hide(GTK_WIDGET(applet_control->image));
  gtk_widget_show(GTK_WIDGET(applet_control->socket));
  workrave_applet_hide_menus(FALSE);
  return TRUE;
}


static gboolean
button_pressed(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  gboolean ret = FALSE;

  if (event->button == 1)
    {
      workrave_applet_connect(FALSE);
      
      if (remote_control != NULL)
        {
          CORBA_Environment ev;
          CORBA_exception_init(&ev);
      
          GNOME_Workrave_WorkraveControl_button_clicked(remote_control, event->button, &ev);

          if (BONOBO_EX(&ev))
            {
              char *err = (char *) bonobo_exception_get_text(&ev);
              g_warning (_("An exception occured '%s'"), err);
              g_free(err);
            }
          
          CORBA_exception_free(&ev);
        }
      
      ret = TRUE;
    }
  
  return ret;
}

static void
showlog_callback(BonoboUIComponent *ui, const char *path, Bonobo_UIComponent_EventType type,
                 const char *state, gpointer user_data)
{
  gboolean new_state;
  workrave_applet_connect(FALSE);

  if (state == NULL || strcmp(state, "") == 0)
    {
      /* State goes blank when component is removed; ignore this. */
      return;
    }

  new_state = strcmp (state, "0") != 0;

  if (1) /* FIXME: applet_control->last_showlog_state != new_state) */
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
  workrave_applet_connect(FALSE);

  if (state == NULL || strcmp(state, "") == 0)
    {
      /* State goes blank when component is removed; ignore this. */
      return;
    }

  new_state = strcmp (state, "0") != 0;
      
  if (remote_control != NULL && path != NULL && new_state)
    {
      CORBA_Environment ev;
      GNOME_Workrave_WorkraveControl_Mode mode = GNOME_Workrave_WorkraveControl_MODE_INVALID;
        
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

      if (mode != GNOME_Workrave_WorkraveControl_MODE_INVALID /* FIXME: &&
                                                                 mode !=
                                                                 applet_control->last_mode*/
          ) 
        {
          applet_control->last_mode = mode;

          CORBA_exception_init(&ev);
          GNOME_Workrave_WorkraveControl_set_mode(remote_control, mode, &ev);

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
workrave_applet_set_hidden(gchar *name, gboolean hidden)
{
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
      const char *s = bonobo_ui_component_get_prop(ui, name, "hidden", NULL);

      set = (s != NULL && atoi(s) != 0);

      if ((hidden && !set) || (!hidden && set))
      {
        bonobo_ui_component_set_prop(ui, name, "hidden", hidden ? "1" : "0", NULL);
      }
    }
}


static void
workrave_applet_hide_menus(gboolean hide)
{
  workrave_applet_set_hidden("/commands/Preferences", hide);
  workrave_applet_set_hidden("/commands/Restbreak", hide);
  workrave_applet_set_hidden("/commands/Network", hide);
  workrave_applet_set_hidden("/commands/Normal", hide);
  workrave_applet_set_hidden("/commands/Suspended", hide);
  workrave_applet_set_hidden("/commands/Quiet", hide);
  workrave_applet_set_hidden("/commands/Mode", hide);
  workrave_applet_set_hidden("/commands/Statistics", hide);
  workrave_applet_set_hidden("/commands/Exercises", hide);
  workrave_applet_set_hidden("/commands/Quit", hide);
}


/* stolen from clock applet :) */
static inline void
force_no_focus_padding (GtkWidget *widget)
{
  static gboolean first = TRUE;

  if (first)
    {
      gtk_rc_parse_string ("\n"
                           "   style \"hdate-applet-button-style\"\n"
                           "   {\n"
                           "      GtkWidget::focus-line-width=0\n"
                           "      GtkWidget::focus-padding=0\n"
                           "   }\n"
                           "\n"
                           "    widget \"*.hdate-applet-button\" style \"hdate-applet-button-style\"\n"
                           "\n");
      first = FALSE;
    }

  gtk_widget_set_name (widget, "hdate-applet-button");
}

static gboolean
workrave_applet_fill(PanelApplet *applet)
{
  GdkPixbuf *pixbuf = NULL;
  GtkWidget *hbox = NULL;
  BonoboUIComponent *ui = NULL;
  
  // Create menus.
  panel_applet_setup_menu_from_file(applet, WORKRAVE_UIDATADIR, "GNOME_WorkraveApplet.xml", NULL,
                                    workrave_applet_verbs, applet);

  // Add listeners for menu toggle-items.
  ui = panel_applet_get_popup_component(applet);
  bonobo_ui_component_add_listener(ui, "ShowLog", showlog_callback, NULL);
  bonobo_ui_component_add_listener(ui, "Normal", mode_callback, NULL);
  bonobo_ui_component_add_listener(ui, "Suspended", mode_callback, NULL);
  bonobo_ui_component_add_listener(ui, "Quiet", mode_callback, NULL);

  gtk_container_set_border_width(GTK_CONTAINER(applet), 0);
  panel_applet_set_background_widget(applet, GTK_WIDGET(applet));
  gtk_widget_set_events(GTK_WIDGET(applet),
                        gtk_widget_get_events(GTK_WIDGET(applet)) | GDK_BUTTON_PRESS_MASK);
  

  g_signal_connect(G_OBJECT(applet), "button_press_event", G_CALLBACK(button_pressed),
                   applet_control);
  
  // Socket.
  applet_control->socket = gtk_socket_new();
  gtk_container_set_border_width(GTK_CONTAINER(applet_control->socket), 0);
  
  // Image
  pixbuf = gdk_pixbuf_new_from_file(WORKRAVE_DATADIR "/images/workrave-icon-medium.png", NULL);  
  applet_control->image = gtk_image_new_from_pixbuf(pixbuf);

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

  gtk_container_set_border_width(GTK_CONTAINER(hbox), 0);
  
  applet_control->socket_id = gtk_socket_get_id(GTK_SOCKET(applet_control->socket));
  applet_control->size = panel_applet_get_size(applet);

  workrave_applet_hide_menus(TRUE);

  force_no_focus_padding(GTK_WIDGET(applet));
  force_no_focus_padding(GTK_WIDGET(applet_control->socket));
  force_no_focus_padding(GTK_WIDGET(applet_control->image));
  force_no_focus_padding(GTK_WIDGET(hbox));

  gtk_widget_show(GTK_WIDGET(applet_control->image));
  gtk_widget_show(GTK_WIDGET(applet_control->socket));
  gtk_widget_show(GTK_WIDGET(hbox));
  gtk_widget_show(GTK_WIDGET(applet));

  g_signal_connect(G_OBJECT(applet), "change_background", G_CALLBACK(change_background), NULL);
  
  return TRUE;
}


static gboolean
workrave_applet_factory(PanelApplet *applet, const gchar *iid, gpointer data)
{
  gboolean retval = FALSE;
  
  if (!strcmp(iid, "OAFIID:GNOME_WorkraveApplet"))
    {
      applet_control = workrave_applet_control_new();
      applet_control->applet = applet;

      retval = workrave_applet_fill(applet); 

      workrave_applet_fire_workrave();
    }

  return retval;
}


PANEL_APPLET_BONOBO_FACTORY("OAFIID:GNOME_WorkraveApplet_Factory",
                            PANEL_TYPE_APPLET,
                            "Workrave Applet",
                            "0",
                            workrave_applet_factory,
                            NULL)
