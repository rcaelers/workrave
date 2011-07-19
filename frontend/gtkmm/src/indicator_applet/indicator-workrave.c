#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <locale.h>
#include <langinfo.h>
#include <string.h>
#include <time.h>

/* GStuff */
#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <glib/gi18n-lib.h>
#include <gio/gio.h>

/* Indicator Stuff */
#include <libindicator/indicator.h>
#include <libindicator/indicator-object.h>
#include <libindicator/indicator-service-manager.h>

/* DBusMenu */
#include <libdbusmenu-gtk/menu.h>
#include <libdbusmenu-gtk/menuitem.h>

#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "dbus-shared.h"

#include "timerbox.h"
#include "timebar.h"

#define INDICATOR_WORKRAVE_TYPE            (indicator_workrave_get_type())
#define INDICATOR_WORKRAVE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), INDICATOR_WORKRAVE_TYPE, IndicatorWorkrave))
#define INDICATOR_WORKRAVE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), INDICATOR_WORKRAVE_TYPE, IndicatorWorkraveClass))
#define IS_INDICATOR_WORKRAVE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), INDICATOR_WORKRAVE_TYPE))
#define IS_INDICATOR_WORKRAVE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), INDICATOR_WORKRAVE_TYPE))
#define INDICATOR_WORKRAVE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), INDICATOR_WORKRAVE_TYPE, IndicatorWorkraveClass))

typedef struct _IndicatorWorkrave         IndicatorWorkrave;
typedef struct _IndicatorWorkraveClass    IndicatorWorkraveClass;
typedef struct _IndicatorWorkravePrivate  IndicatorWorkravePrivate;

struct _IndicatorWorkraveClass {
	IndicatorObjectClass parent_class;
};

struct _IndicatorWorkrave {
	IndicatorObject parent;
	IndicatorWorkravePrivate *priv;
};

struct _IndicatorWorkravePrivate {
	GtkLabel *label;
  GtkImage *image;

	IndicatorServiceManager *sm;
	DbusmenuGtkMenu *menu;

	GCancellable *service_proxy_cancel;
	GCancellable *workrave_proxy_cancel;
	GDBusProxy *service_proxy;
	GDBusProxy *workrave_proxy;

  gboolean has_alpha;

  WorkraveTimerbox *timerbox;
};

#define INDICATOR_WORKRAVE_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE((o), INDICATOR_WORKRAVE_TYPE, IndicatorWorkravePrivate))

GType indicator_workrave_get_type(void);

static void indicator_workrave_class_init (IndicatorWorkraveClass *klass);
static void indicator_workrave_init       (IndicatorWorkrave *self);
static void indicator_workrave_dispose    (GObject *object);
static void indicator_workrave_finalize   (GObject *object);
static GtkLabel *get_label                (IndicatorObject *io);
static GtkImage *get_icon                 (IndicatorObject * io);
static GtkMenu *get_menu                  (IndicatorObject *io);
static const gchar *get_accessible_desc   (IndicatorObject *io);
static void receive_signal                (GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);
static void service_proxy_cb              (GObject *object, GAsyncResult *res, gpointer user_data);
static void workrave_proxy_cb              (GObject *object, GAsyncResult *res, gpointer user_data);

static void menu_visible_notfy_cb(GtkWidget * menu, G_GNUC_UNUSED GParamSpec *pspec, gpointer user_data);

/* Indicator Module Config */
INDICATOR_SET_VERSION
INDICATOR_SET_TYPE(INDICATOR_WORKRAVE_TYPE)

G_DEFINE_TYPE(IndicatorWorkrave, indicator_workrave, INDICATOR_OBJECT_TYPE);

static void
indicator_workrave_class_init(IndicatorWorkraveClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_debug("indicator-workrave: indicator_workrave_class_init");
  
	g_type_class_add_private(klass, sizeof(IndicatorWorkravePrivate));

	object_class->dispose = indicator_workrave_dispose;
	object_class->finalize = indicator_workrave_finalize;

	IndicatorObjectClass *io_class = INDICATOR_OBJECT_CLASS(klass);

	io_class->get_label = get_label;
	io_class->get_menu  = get_menu;
	io_class->get_image = get_icon;
	io_class->get_accessible_desc = get_accessible_desc;
}


static void
indicator_workrave_init(IndicatorWorkrave *self)
{
	g_debug("indicator-workrave: indicator_workrave_init");
	self->priv = INDICATOR_WORKRAVE_GET_PRIVATE(self);

	self->priv->label = NULL;
	self->priv->image = NULL;
	self->priv->service_proxy = NULL;
	self->priv->workrave_proxy = NULL;
	self->priv->sm = NULL;
	self->priv->menu = NULL;

  self->priv->timerbox = g_object_new(WORKRAVE_TYPE_TIMERBOX, NULL);
  
	self->priv->sm = indicator_service_manager_new_version(SERVICE_NAME, SERVICE_VERSION);
	self->priv->menu = dbusmenu_gtkmenu_new(SERVICE_NAME, MENU_OBJ);
  
	g_signal_connect(self->priv->menu, "notify::visible", G_CALLBACK(menu_visible_notfy_cb), self);
 
	// DbusmenuGtkClient *client = dbusmenu_gtkmenu_get_client(self->priv->menu);
	// dbusmenu_client_add_type_handler_full(DBUSMENU_CLIENT(client), DBUSMENU_WORKRAVE_MENUITEM_TYPE, new_menu_item, self, NULL);
  
	self->priv->service_proxy_cancel = g_cancellable_new();
	self->priv->workrave_proxy_cancel = g_cancellable_new();

	g_dbus_proxy_new_for_bus(G_BUS_TYPE_SESSION,
                           G_DBUS_PROXY_FLAGS_NONE,
                           NULL,
                           SERVICE_NAME,
                           SERVICE_OBJ,
                           SERVICE_IFACE,
                           self->priv->service_proxy_cancel,
                           service_proxy_cb,
                           self);
  
	g_dbus_proxy_new_for_bus(G_BUS_TYPE_SESSION,
                           G_DBUS_PROXY_FLAGS_NONE,
                           NULL,
                           "org.workrave.Workrave",
                           "/org/workrave/Workrave/UI",
                           "org.workrave.UnityInterface",
                           self->priv->workrave_proxy_cancel,
                           workrave_proxy_cb,
                           self);
  
}

static void
service_proxy_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
	GError *error = NULL;

	g_debug("indicator-workrave: service_proxy_cb");
  
	IndicatorWorkrave *self = INDICATOR_WORKRAVE(user_data);
	g_return_if_fail(self != NULL);

	GDBusProxy *proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

	IndicatorWorkravePrivate *priv = INDICATOR_WORKRAVE_GET_PRIVATE(self);

	if (priv->service_proxy_cancel != NULL)
    {
      g_object_unref(priv->service_proxy_cancel);
      priv->service_proxy_cancel = NULL;
    }

	if (error != NULL)
    {
      g_warning("Could not grab DBus proxy for %s: %s", SERVICE_NAME, error->message);
      g_error_free(error);
      return;
    }

	priv->service_proxy = proxy;

	g_signal_connect(proxy, "g-signal", G_CALLBACK(receive_signal), self);

	return;
}


static void
workrave_proxy_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
	GError *error = NULL;

	g_debug("indicator-workrave: workrave_proxy_cb");
  
	IndicatorWorkrave *self = INDICATOR_WORKRAVE(user_data);
	g_return_if_fail(self != NULL);

	GDBusProxy *proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

	IndicatorWorkravePrivate *priv = INDICATOR_WORKRAVE_GET_PRIVATE(self);

	if (priv->workrave_proxy_cancel != NULL)
    {
      g_object_unref(priv->workrave_proxy_cancel);
      priv->workrave_proxy_cancel = NULL;
    }

	if (error != NULL)
    {
      g_warning("Could not grab DBus proxy for %s: %s", SERVICE_NAME, error->message);
      g_error_free(error);
      return;
    }

	priv->workrave_proxy = proxy;
	g_signal_connect(proxy, "g-signal", G_CALLBACK(receive_signal), self);

	return;
}

static void
indicator_workrave_dispose(GObject *object)
{
	IndicatorWorkrave *self = INDICATOR_WORKRAVE(object);

	if (self->priv->label != NULL)
    {
      g_object_unref(self->priv->label);
      self->priv->label = NULL;
    }

	if (self->priv->image != NULL)
    {
      g_object_unref(self->priv->image);
      self->priv->image = NULL;
    }

	if (self->priv->menu != NULL)
    {
      g_object_unref(G_OBJECT(self->priv->menu));
      self->priv->menu = NULL;
    }

	if (self->priv->sm != NULL)
    {
      g_object_unref(G_OBJECT(self->priv->sm));
      self->priv->sm = NULL;
    }

	if (self->priv->service_proxy != NULL)
    {
      g_object_unref(self->priv->service_proxy);
      self->priv->service_proxy = NULL;
    }

	G_OBJECT_CLASS(indicator_workrave_parent_class)->dispose(object);
	return;
}

static void
indicator_workrave_finalize(GObject *object)
{
	// IndicatorWorkrave *self = INDICATOR_WORKRAVE(object);
	G_OBJECT_CLASS(indicator_workrave_parent_class)->finalize(object);
	return;
}


typedef struct TimerData
{
  char *bar_text;
  int slot;
  int bar_secondary_color;
  int bar_secondary_val;
  int bar_secondary_max;
  int bar_primary_color;
  int bar_primary_val;
  int bar_primary_max;
} TimerData;

static void
receive_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data)
{
	IndicatorWorkrave *self = INDICATOR_WORKRAVE(user_data);

	g_debug("indicator-workrave: signal %s", signal_name);
  
	if (g_strcmp0(signal_name, "Update") == 0)
    {
    }
	else if (g_strcmp0(signal_name, "UpdateIndicator") == 0)
    {
      TimerData td[BREAK_ID_SIZEOF];
      
      g_variant_get(parameters, "((suuuuuuu)(suuuuuuu)(suuuuuuu))",
                    &td[BREAK_ID_MICRO_BREAK].bar_text,
                    &td[BREAK_ID_MICRO_BREAK].slot,
                    &td[BREAK_ID_MICRO_BREAK].bar_secondary_color,
                    &td[BREAK_ID_MICRO_BREAK].bar_secondary_val,
                    &td[BREAK_ID_MICRO_BREAK].bar_secondary_max,
                    &td[BREAK_ID_MICRO_BREAK].bar_primary_color,
                    &td[BREAK_ID_MICRO_BREAK].bar_primary_val,
                    &td[BREAK_ID_MICRO_BREAK].bar_primary_max,
                    &td[BREAK_ID_REST_BREAK].bar_text,
                    &td[BREAK_ID_REST_BREAK].slot,
                    &td[BREAK_ID_REST_BREAK].bar_secondary_color,
                    &td[BREAK_ID_REST_BREAK].bar_secondary_val,
                    &td[BREAK_ID_REST_BREAK].bar_secondary_max,
                    &td[BREAK_ID_REST_BREAK].bar_primary_color,
                    &td[BREAK_ID_REST_BREAK].bar_primary_val,
                    &td[BREAK_ID_REST_BREAK].bar_primary_max,
                    &td[BREAK_ID_DAILY_LIMIT].bar_text,
                    &td[BREAK_ID_DAILY_LIMIT].slot,
                    &td[BREAK_ID_DAILY_LIMIT].bar_secondary_color,
                    &td[BREAK_ID_DAILY_LIMIT].bar_secondary_val,
                    &td[BREAK_ID_DAILY_LIMIT].bar_secondary_max,
                    &td[BREAK_ID_DAILY_LIMIT].bar_primary_color,
                    &td[BREAK_ID_DAILY_LIMIT].bar_primary_val,
                    &td[BREAK_ID_DAILY_LIMIT].bar_primary_max
                    );

      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          WorkraveTimebar *timebar = workrave_timerbox_get_time_bar(self->priv->timerbox, i);
          if (timebar != NULL)
            {
              workrave_timebar_set_progress(timebar, td[i].bar_primary_val, td[i].bar_primary_max, td[i].bar_primary_color);
              workrave_timebar_set_secondary_progress(timebar, td[i].bar_secondary_val, td[i].bar_secondary_max, td[i].bar_secondary_color);
              workrave_timebar_set_text(timebar, td[i].bar_text);
            }
        }

      workrave_timerbox_update(self->priv->timerbox, self->priv->image);
    }
}
static void
menu_visible_notfy_cb(GtkWidget * menu, G_GNUC_UNUSED GParamSpec *pspec, gpointer user_data)
{
	// IndicatorWorkrave *self = INDICATOR_WORKRAVE(user_data);
	g_debug("indicator-workrave: menu visible notify");
	
	gboolean visible;
	g_object_get(G_OBJECT(menu), "visible", &visible, NULL);
	if (visible) return;
}


static GtkLabel *
get_label(IndicatorObject *io)
{
	IndicatorWorkrave *self = INDICATOR_WORKRAVE(io);

	if (self->priv->label == NULL)
    {
      self->priv->label = GTK_LABEL(gtk_label_new("Workrave"));
      gtk_label_set_justify(GTK_LABEL(self->priv->label), GTK_JUSTIFY_CENTER);
      gtk_widget_show(GTK_WIDGET(self->priv->label));
      g_object_ref(G_OBJECT(self->priv->label));
    }

	return self->priv->label;
}

static GtkImage *
get_icon(IndicatorObject *io)
{
	IndicatorWorkrave *self = INDICATOR_WORKRAVE(io);

	if (self->priv->image == NULL)
    {
      self->priv->image = GTK_IMAGE(gtk_image_new());
      gtk_widget_show(GTK_WIDGET(self->priv->image));
    }
	return self->priv->image;
}

static GtkMenu *
get_menu(IndicatorObject *io)
{
	IndicatorWorkrave *self = INDICATOR_WORKRAVE(io);
	return GTK_MENU(self->priv->menu);
}

static const gchar *
get_accessible_desc(IndicatorObject *io)
{
	IndicatorWorkrave *self = INDICATOR_WORKRAVE(io);
	const gchar *name =  NULL;

	if (self->priv->label != NULL)
    {
      name = gtk_label_get_text(self->priv->label);
    }
	return name;
}
