#ifndef __WORKRAVE_CONTROL_H__
#define __WORKRAVE_CONTROL_H__

#include <bonobo/bonobo-object.h>
#include <glib-object.h>
#include <bonobo-activation/bonobo-activation.h>

#include "Workrave-Control.h"

G_BEGIN_DECLS

#define WORKRAVE_CONTROL_TYPE         (workrave_control_get_type())
#define WORKRAVE_CONTROL(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), WORKRAVE_CONTROL_TYPE, WorkraveControl))
#define WORKRAVE_CONTROL_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), WORKRAVE_CONTROL_TYPE, WorkraveControlClass))
#define WORKRAVE_CONTROL_IS_OBJECT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), WORKRAVE_CONTROL_TYPE))
#define WORKRAVE_CONTROL_IS_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), WORKRAVE_CONTROL_TYPE))
#define WORKRAVE_CONTROL_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), WORKRAVE_CONTROL_TYPE, WorkraveControlClass))

typedef struct _WorkraveControl       WorkraveControl;
typedef struct _WorkraveControlClass  WorkraveControlClass;

struct _WorkraveControl
{
  BonoboObject parent;
};

struct _WorkraveControlClass
{
  BonoboObjectClass parent_class;
  POA_GNOME_Workrave_WorkraveControl__epv epv;
};


static void 		workrave_control_class_init(WorkraveControlClass *);
static void 		workrave_control_init(WorkraveControl *);
static WorkraveControl*	workrave_control_new(void);
static void 		workrave_control_fire(PortableServer_Servant, CORBA_Environment *);

G_END_DECLS

#endif /*__WORKRAVE_CONTROL_H__*/
