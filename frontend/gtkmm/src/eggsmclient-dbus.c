/*
 * Copyright (C) 2008 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include "eggsmclient.h"
#include "eggsmclient-private.h"

#include "eggdesktopfile.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gdk/gdk.h>
#include <dbus/dbus-glib.h>

#define GSM_DBUS_NAME      "org.gnome.SessionManager"
#define GSM_DBUS_PATH      "/org/gnome/SessionManager"
#define GSM_DBUS_INTERFACE "org.gnome.SessionManager"

#define GSM_CLIENT_PRIVATE_DBUS_INTERFACE "org.gnome.SessionManager.ClientPrivate"
#define GSM_CLIENT_DBUS_INTERFACE         "org.gnome.SessionManager.Client"

#define EGG_TYPE_SM_CLIENT_DBUS            (egg_sm_client_dbus_get_type ())
#define EGG_SM_CLIENT_DBUS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_SM_CLIENT_DBUS, EggSMClientDBus))
#define EGG_SM_CLIENT_DBUS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_SM_CLIENT_DBUS, EggSMClientDBusClass))
#define EGG_IS_SM_CLIENT_DBUS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_SM_CLIENT_DBUS))
#define EGG_IS_SM_CLIENT_DBUS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_SM_CLIENT_DBUS))
#define EGG_SM_CLIENT_DBUS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_SM_CLIENT_DBUS, EggSMClientDBusClass))

typedef struct _EggSMClientDBus        EggSMClientDBus;
typedef struct _EggSMClientDBusClass   EggSMClientDBusClass;

struct _EggSMClientDBus
{
  EggSMClient parent;

  DBusGConnection *conn;
  DBusGProxy *sm_proxy, *client_proxy;
  char *client_path;
};

struct _EggSMClientDBusClass
{
  EggSMClientClass parent_class;

};

static void     sm_client_dbus_startup     (EggSMClient *client,
					    const char  *client_id);
static void     sm_client_dbus_will_quit   (EggSMClient *client,
					    gboolean     will_quit);
static gboolean sm_client_dbus_end_session (EggSMClient *client,
					    EggSMClientEndStyle style,
					    gboolean     request_confirmation);

static void dbus_client_query_end_session  (DBusGProxy  *proxy,
					    guint        flags,
					    gpointer     smclient);
static void dbus_client_end_session        (DBusGProxy  *proxy,
					    guint        flags,
					    gpointer     smclient);
static void dbus_client_cancel_end_session (DBusGProxy  *proxy,
					    gpointer     smclient);
static void dbus_client_stop               (DBusGProxy  *proxy,
					    gpointer     smclient);

G_DEFINE_TYPE (EggSMClientDBus, egg_sm_client_dbus, EGG_TYPE_SM_CLIENT)

static void
egg_sm_client_dbus_init (EggSMClientDBus *dbus)
{
  ;
}

static void
egg_sm_client_dbus_class_init (EggSMClientDBusClass *klass)
{
  EggSMClientClass *sm_client_class = EGG_SM_CLIENT_CLASS (klass);

  sm_client_class->startup     = sm_client_dbus_startup;
  sm_client_class->will_quit   = sm_client_dbus_will_quit;
  sm_client_class->end_session = sm_client_dbus_end_session;
}

EggSMClient *
egg_sm_client_dbus_new (void)
{
  DBusGConnection *conn;
  DBusGProxy *proxy;
  EggSMClientDBus *dbus;

  conn = dbus_g_bus_get (DBUS_BUS_SESSION, NULL);
  if (!conn)
    return NULL;

  proxy = dbus_g_proxy_new_for_name (conn, GSM_DBUS_NAME, GSM_DBUS_PATH,
				     GSM_DBUS_INTERFACE);
  if (!proxy)
    {
      g_object_unref (conn);
      return NULL;
    }

  dbus = g_object_new (EGG_TYPE_SM_CLIENT_DBUS, NULL);
  dbus->conn = conn;
  dbus->sm_proxy = proxy;
  return (EggSMClient *)dbus;
}

static void
sm_client_dbus_startup (EggSMClient *client,
			const char  *client_id)
{
  EggSMClientDBus *dbus = (EggSMClientDBus *)client;
  GError *error = NULL;
  char *client_path, *ret_client_id;
  DBusGProxy *client_public;

  if (!dbus_g_proxy_call (dbus->sm_proxy, "RegisterClient", &error,
			  G_TYPE_STRING, g_get_prgname (),
			  G_TYPE_STRING, client_id,
			  G_TYPE_INVALID,
			  DBUS_TYPE_G_OBJECT_PATH, &client_path,
			  G_TYPE_INVALID))
    {
      g_warning ("Failed to register client: %s", error->message);
      g_error_free (error);
      return;
    }

  g_debug ("Client registered with session manager: %s", client_path);
  dbus->client_proxy = dbus_g_proxy_new_for_name (dbus->conn, GSM_DBUS_NAME,
                                                  client_path,
                                                  GSM_CLIENT_PRIVATE_DBUS_INTERFACE);
  dbus_g_proxy_add_signal (dbus->client_proxy, "QueryEndSession",
			   G_TYPE_UINT,
			   G_TYPE_INVALID);
  dbus_g_proxy_connect_signal (dbus->client_proxy, "QueryEndSession",
			       G_CALLBACK (dbus_client_query_end_session),
			       dbus, NULL);
  dbus_g_proxy_add_signal (dbus->client_proxy, "EndSession",
			   G_TYPE_UINT,
			   G_TYPE_INVALID);
  dbus_g_proxy_connect_signal (dbus->client_proxy, "EndSession",
			       G_CALLBACK (dbus_client_end_session),
			       dbus, NULL);
  dbus_g_proxy_add_signal (dbus->client_proxy, "CancelEndSession",
			   G_TYPE_UINT,
			   G_TYPE_INVALID);
  dbus_g_proxy_connect_signal (dbus->client_proxy, "CancelEndSession",
			       G_CALLBACK (dbus_client_cancel_end_session),
			       dbus, NULL);
  dbus_g_proxy_add_signal (dbus->client_proxy, "Stop",
			   G_TYPE_INVALID);
  dbus_g_proxy_connect_signal (dbus->client_proxy, "Stop",
			       G_CALLBACK (dbus_client_stop),
			       dbus, NULL);

  client_public = dbus_g_proxy_new_for_name (dbus->conn, GSM_DBUS_NAME,
					     client_path,
					     GSM_CLIENT_DBUS_INTERFACE);
  if (dbus_g_proxy_call (client_public, "GetStartupId", &error,
			 G_TYPE_INVALID,
			 G_TYPE_STRING, &ret_client_id,
			 G_TYPE_INVALID))
    {
      gdk_threads_enter ();
      gdk_set_sm_client_id (ret_client_id);
      gdk_threads_leave ();

      g_debug ("Got client ID \"%s\"", ret_client_id);
      g_free (ret_client_id);
    }
  else
    {
      g_warning ("Could not get client id: %s", error->message);
      g_error_free (error);
    }
  g_object_unref (client_public);
}

static void
sm_client_dbus_shutdown (EggSMClient *client)
{
  EggSMClientDBus *dbus = EGG_SM_CLIENT_DBUS (client);
  GError *error = NULL;

  if (!dbus_g_proxy_call (dbus->sm_proxy, "UnregisterClient", &error,
			  DBUS_TYPE_G_OBJECT_PATH, dbus->client_path,
			  G_TYPE_INVALID,
			  G_TYPE_INVALID))
    {
      g_warning ("Failed to unregister client: %s", error->message);
      g_error_free (error);
      return;
    }

  g_free (dbus->client_path);
  dbus->client_path = NULL;

  g_object_unref (dbus->client_proxy);
  dbus->client_proxy = NULL;
}

static void
dbus_client_query_end_session (DBusGProxy *proxy,
			       guint       flags,
			       gpointer    smclient)
{
  egg_sm_client_quit_requested (smclient);
}

static void
sm_client_dbus_will_quit (EggSMClient *client,
			  gboolean     will_quit)
{
  EggSMClientDBus *dbus = (EggSMClientDBus *)client;

  g_return_if_fail (dbus->client_proxy != NULL);

  dbus_g_proxy_call (dbus->client_proxy, "EndSessionResponse", NULL,
		     G_TYPE_BOOLEAN, will_quit,
		     G_TYPE_STRING, NULL,
		     G_TYPE_INVALID,
		     G_TYPE_INVALID);
}

static void
dbus_client_end_session (DBusGProxy *proxy,
			 guint       flags,
			 gpointer    smclient)
{
  sm_client_dbus_will_quit (smclient, TRUE);
  sm_client_dbus_shutdown (smclient);
  egg_sm_client_quit (smclient);
}

static void
dbus_client_cancel_end_session (DBusGProxy *proxy,
				gpointer    smclient)
{
  egg_sm_client_quit_cancelled (smclient);
}

static void
dbus_client_stop (DBusGProxy *proxy,
		  gpointer    smclient)
{
  sm_client_dbus_shutdown (smclient);
  egg_sm_client_quit (smclient);
}

static gboolean
sm_client_dbus_end_session (EggSMClient         *client,
			    EggSMClientEndStyle  style,
			    gboolean             request_confirmation)
{
  EggSMClientDBus *dbus = (EggSMClientDBus *)client;

  if (style == EGG_SM_CLIENT_END_SESSION_DEFAULT ||
      style == EGG_SM_CLIENT_LOGOUT)
    {
      return dbus_g_proxy_call (dbus->sm_proxy, "Logout", NULL,
				G_TYPE_UINT, request_confirmation ? 0 : 1,
				G_TYPE_INVALID,
				G_TYPE_INVALID);
    }
  else
    {
      return dbus_g_proxy_call (dbus->sm_proxy, "Shutdown", NULL,
				G_TYPE_INVALID,
				G_TYPE_INVALID);
    }
}
