#include <gtk/gtk.h>

#include <string.h>

/* GStuff */
#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "timerbox.h"

static GtkImage *image;
static GCancellable *workrave_proxy_cancel;
static GDBusProxy *workrave_proxy;
static WorkraveTimerbox *timerbox;

static void receive_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    /* Change TRUE to FALSE and the main window will be destroyed with
     * a "delete-event". */

    return TRUE;
}

/* Another callback */
static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    gtk_main_quit ();
}

static void
workrave_proxy_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
	GError *error = NULL;

	g_debug("indicator-workrave: workrave_proxy_cb");
  
	GDBusProxy *proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

	if (workrave_proxy_cancel != NULL)
    {
      g_object_unref(workrave_proxy_cancel);
      workrave_proxy_cancel = NULL;
    }

	if (error != NULL)
    {
      g_warning("Could not grab DBus proxy: %s", error->message);
      g_error_free(error);
      return;
    }

	workrave_proxy = proxy;
	g_signal_connect(proxy, "g-signal", G_CALLBACK(receive_signal), NULL);
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

	g_debug("indicator-workrave: signal %s", signal_name);
  
	if (g_strcmp0(signal_name, "Update") == 0)
    {
    }
	else if (g_strcmp0(signal_name, "UpdateIndicator") == 0)
    {
      TimerData td[BREAK_ID_SIZEOF];

      memset(td, 0, sizeof(td));
      
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
          workrave_timerbox_set_slot(timerbox, td[i].slot, i);
        }
      
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          WorkraveTimebar *timebar = workrave_timerbox_get_time_bar(timerbox, i);
          if (timebar != NULL)
            {
              workrave_timebar_set_progress(timebar, td[i].bar_primary_val, td[i].bar_primary_max, td[i].bar_primary_color);
              workrave_timebar_set_secondary_progress(timebar, td[i].bar_secondary_val, td[i].bar_secondary_max, td[i].bar_secondary_color);
              workrave_timebar_set_text(timebar, td[i].bar_text);
            }
        }

      workrave_timerbox_update(timerbox, image);
    }
}

int main( int   argc,
          char *argv[] )
{
  GtkWidget *window;
    
  gtk_init(&argc, &argv);
    
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
  g_signal_connect(window, "delete-event", G_CALLBACK (delete_event), NULL);
  g_signal_connect(window, "destroy", G_CALLBACK (destroy), NULL);
    
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    
  image = gtk_image_new();
    
  gtk_container_add(GTK_CONTAINER(window), image);
    
  gtk_widget_show(image);
  gtk_widget_show(window);

  timerbox = g_object_new(WORKRAVE_TYPE_TIMERBOX, NULL);

	g_dbus_proxy_new_for_bus(G_BUS_TYPE_SESSION,
                           G_DBUS_PROXY_FLAGS_NONE,
                           NULL,
                           "org.workrave.Workrave",
                           "/org/workrave/Workrave/UI",
                           "org.workrave.UnityInterface",
                           workrave_proxy_cancel,
                           workrave_proxy_cb,
                           NULL);
  
  
  gtk_main();
    
  return 0;
}
