#ifndef __WR_APPLET_CONTROL_H__
#define __WR_APPLET_CONTROL_H__

#include <glib-object.h>
#include <bonobo/bonobo-object.h>
#include <bonobo-activation/bonobo-activation.h>

#include "Workrave-Applet.h"

G_BEGIN_DECLS

#define WR_APPLET_CONTROL_TYPE         (workrave_applet_control_get_type())
#define WR_APPLET_CONTROL(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), WR_APPLET_CONTROL_TYPE, AppletControl))
#define WR_APPLET_CONTROL_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), WR_APPLET_CONTROL_TYPE, AppletControlClass))
#define WR_APPLET_CONTROL_IS_OBJECT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), WR_APPLET_CONTROL_TYPE))
#define WR_APPLET_CONTROL_IS_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), WR_APPLET_CONTROL_TYPE))
#define WR_APPLET_CONTROL_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), WR_APPLET_CONTROL_TYPE, AppletControlClass))

typedef struct _AppletControl       AppletControl;
typedef struct _AppletControlClass  AppletControlClass;

struct _AppletControl
{
  BonoboObject parent;

  GtkWidget *image;
  GtkWidget *socket;

  long size;
  long socket_id;
  gboolean vertical;
};

struct _AppletControlClass
{
  BonoboObjectClass parent_class;
  POA_GNOME_Workrave_AppletControl__epv epv;
};


static void 		workrave_applet_control_class_init(AppletControlClass *);
static void 		workrave_applet_control_init(AppletControl *);
static AppletControl*	workrave_applet_control_new(void);
static CORBA_long 	workrave_applet_control_get_socket_id(PortableServer_Servant, CORBA_Environment *);
static CORBA_long 	workrave_applet_control_get_size(PortableServer_Servant, CORBA_Environment *);
static CORBA_boolean 	workrave_applet_control_get_vertical(PortableServer_Servant, CORBA_Environment *);

G_END_DECLS

#endif /*__WR_APPLET_CONTROL_H__*/
