#ifndef __WR_APPLET_CONTROL_H__
#define __WR_APPLET_CONTROL_H__

#include <bonobo/bonobo-object.h>
#include <glib-object.h>
#include <bonobo-activation/bonobo-activation.h>

#include "Workrave-Applet.h"

G_BEGIN_DECLS

extern long workrave_applet_socket_id;

#define WR_APPLET_CONTROL_TYPE             (workrave_applet_get_type ())
#define WR_APPLET_CONTROL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), WR_APPLET_CONTROL_TYPE, AppletControl))
#define WR_APPLET_CONTROL_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), WR_APPLET_CONTROL_TYPE, AppletControlClass))
#define WR_APPLET_CONTROL_IS_OBJECT(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WR_APPLET_CONTROL_TYPE))
#define WR_APPLET_CONTROL_IS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), WR_APPLET_CONTROL_TYPE))
#define WR_APPLET_CONTROL_GET_CLASS(o) 	   (G_TYPE_INSTANCE_GET_CLASS ((o), WR_APPLET_CONTROL_TYPE, AppletControlClass))

typedef struct _AppletControl       AppletControl;
typedef struct _AppletControlClass  AppletControlClass;

struct _AppletControl {
	BonoboObject parent;

	/* TODO: add member vars */
};

struct _AppletControlClass {
	BonoboObjectClass parent_class;
	POA_GNOME_Workrave_AppletControl__epv epv;

	/* TODO: add other class vars, signals here */
};


GType           workrave_applet_get_type   (void);
AppletControl*  workrave_applet_new        (void);
CORBA_long      workrave_applet_get_socket_id (PortableServer_Servant _servant, CORBA_Environment * ev);   


/* TODO: add custom function declarations here */


G_END_DECLS

#endif /*__WR_APPLET_CONTROL_H__*/
