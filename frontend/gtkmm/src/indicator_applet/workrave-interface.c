#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gio/gio.h>

#include "workrave-interface.h"
#include "gen-workrave-service.xml.h"
#include "dbus-shared.h"

/**
	WorkraveInterfacePrivate:
	@dbus_registration: The handle for this object being registered
		on dbus.

	Structure to define the memory for the private area
	of the workrave interface instance.
*/
struct _WorkraveInterfacePrivate {
	GDBusConnection * bus;
	GCancellable * bus_cancel;
	guint dbus_registration;
};

#define WORKRAVE_INTERFACE_GET_PRIVATE(o) (WORKRAVE_INTERFACE(o)->priv)

/* GDBus Stuff */
static GDBusNodeInfo *      node_info = NULL;
static GDBusInterfaceInfo * interface_info = NULL;

static void workrave_interface_class_init (WorkraveInterfaceClass *klass);
static void workrave_interface_init       (WorkraveInterface *self);
static void workrave_interface_dispose    (GObject *object);
static void workrave_interface_finalize   (GObject *object);
static void bus_get_cb (GObject * object, GAsyncResult * res, gpointer user_data);

G_DEFINE_TYPE (WorkraveInterface, workrave_interface, G_TYPE_OBJECT);

static void
workrave_interface_class_init (WorkraveInterfaceClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (WorkraveInterfacePrivate));

	object_class->dispose = workrave_interface_dispose;
	object_class->finalize = workrave_interface_finalize;

	/* Setting up the DBus interfaces */
	if (node_info == NULL) {
		GError * error = NULL;

		node_info = g_dbus_node_info_new_for_xml(_workrave_service, &error);
		if (error != NULL) {
			g_error("Unable to parse Workrave Service Interface description: %s", error->message);
			g_error_free(error);
		}
	}

	if (interface_info == NULL) {
		interface_info = g_dbus_node_info_lookup_interface(node_info, SERVICE_IFACE);

		if (interface_info == NULL) {
			g_error("Unable to find interface '" SERVICE_IFACE "'");
		}
	}

	return;
}

static void
workrave_interface_init (WorkraveInterface *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, WORKRAVE_INTERFACE_TYPE, WorkraveInterfacePrivate);

	self->priv->bus = NULL;
	self->priv->bus_cancel = NULL;
	self->priv->dbus_registration = 0;

	self->priv->bus_cancel = g_cancellable_new();
	g_bus_get(G_BUS_TYPE_SESSION,
	          self->priv->bus_cancel,
	          bus_get_cb,
	          self);

	return;
}

static void
bus_get_cb (GObject * object, GAsyncResult * res, gpointer user_data)
{
	GError * error = NULL;
	GDBusConnection * connection = g_bus_get_finish(res, &error);

	if (error != NULL) {
		g_error("OMG! Unable to get a connection to DBus: %s", error->message);
		g_error_free(error);
		return;
	}

	WorkraveInterfacePrivate * priv = WORKRAVE_INTERFACE_GET_PRIVATE(user_data);

	g_warn_if_fail(priv->bus == NULL);
	priv->bus = connection;

	if (priv->bus_cancel != NULL) {
		g_object_unref(priv->bus_cancel);
		priv->bus_cancel = NULL;
	}

	/* Now register our object on our new connection */
	priv->dbus_registration = g_dbus_connection_register_object(priv->bus,
	                                                            SERVICE_OBJ,
	                                                            interface_info,
	                                                            NULL,
	                                                            user_data,
	                                                            NULL,
	                                                            &error);

	if (error != NULL) {
		g_error("Unable to register the object to DBus: %s", error->message);
		g_error_free(error);
		return;
	}

	return;	
}

static void
workrave_interface_dispose (GObject *object)
{
	WorkraveInterfacePrivate * priv = WORKRAVE_INTERFACE_GET_PRIVATE(object);

	if (priv->dbus_registration != 0) {
		g_dbus_connection_unregister_object(priv->bus, priv->dbus_registration);
		/* Don't care if it fails, there's nothing we can do */
		priv->dbus_registration = 0;
	}

	if (priv->bus != NULL) {
		g_object_unref(priv->bus);
		priv->bus = NULL;
	}

	if (priv->bus_cancel != NULL) {
		g_cancellable_cancel(priv->bus_cancel);
		g_object_unref(priv->bus_cancel);
		priv->bus_cancel = NULL;
	}

	G_OBJECT_CLASS (workrave_interface_parent_class)->dispose (object);
	return;
}

static void
workrave_interface_finalize (GObject *object)
{

	G_OBJECT_CLASS (workrave_interface_parent_class)->finalize (object);
	return;
}

void
workrave_interface_update (WorkraveInterface *self)
{
	g_return_if_fail(IS_WORKRAVE_INTERFACE(self));

	WorkraveInterfacePrivate * priv = WORKRAVE_INTERFACE_GET_PRIVATE(self);
	GError * error = NULL;

	g_dbus_connection_emit_signal (priv->bus,
	                               NULL,
	                               SERVICE_OBJ,
	                               SERVICE_IFACE,
	                               "Update",
	                               NULL,
	                               &error);

	if (error != NULL) {
		g_error("Unable to send UpdateTime signal: %s", error->message);
		g_error_free(error);
		return;
	}

	return;
}
