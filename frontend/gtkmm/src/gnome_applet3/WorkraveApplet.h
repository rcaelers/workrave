#ifndef __WORKRAVEAPPLET_H__
#define __WORKRAVEAPPLET_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define WORKRAVE_APPLET_TYPE         (workrave_applet_get_type())
#define WORKRAVE_APPLET(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), WORKRAVE_APPLET_TYPE, WorkraveApplet))
#define WORKRAVE_APPLET_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), WORKRAVE_APPLET_TYPE, WorkraveAppletClass))
#define WORKRAVE_APPLET_IS_OBJECT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), WORKRAVE_APPLET_TYPE))
#define WORKRAVE_APPLET_IS_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), WORKRAVE_APPLET_TYPE))
#define WORKRAVE_APPLET_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), WORKRAVE_APPLET_TYPE, WorkraveAppletClass))

typedef struct _WorkraveApplet       WorkraveApplet;
typedef struct _WorkraveAppletClass  WorkraveAppletClass;

struct _WorkraveApplet
{
  GObject base;

  GtkActionGroup *action_group;
  
  GtkWidget *event_box;
  GtkWidget *image;
  GtkWidget *socket;
  PanelApplet *applet;
  
  int size;
  int socket_id;
  int orientation;

  gboolean last_showlog_state;
  gboolean last_reading_mode_state;
  int last_mode;

  DBusGProxy *support;
  DBusGProxy *ui;
  DBusGProxy *core;
};

struct _WorkraveAppletClass
{
  GObjectClass base;
};


#define DBUS_SERVICE_APPLET "org.workrave.Workrave.GnomeApplet"
#define WORKRAVE_DBUS_ERROR g_quark_from_static_string ("workrave")

GType workrave_applet_get_type(void);
gboolean workrave_applet_get_socket_id(WorkraveApplet *, guint *, GError **);
gboolean workrave_applet_get_size(WorkraveApplet *, guint *, GError **);
gboolean workrave_applet_get_orientation(WorkraveApplet *, guint *, GError **);
gboolean workrave_applet_set_menu_status(WorkraveApplet *, const char *, gboolean, GError **);
gboolean workrave_applet_get_menu_status(WorkraveApplet *, const char *, gboolean *, GError **);
gboolean workrave_applet_set_menu_active(WorkraveApplet *, const char *, gboolean, GError **);
gboolean workrave_applet_get_menu_active(WorkraveApplet *, const char *, gboolean *, GError **);

G_END_DECLS

#endif /*__WORKRAVEAPPLET_H__*/
