#ifndef __WORKRAVE_CONTROL_H__
#define __WORKRAVE_CONTROL_H__

#include <gnome.h>
#include <glib-object.h>
#include <bonobo/bonobo-generic-factory.h>
#include <bonobo/bonobo-object.h>
#include <bonobo-activation/bonobo-activation.h>

#define WR_C_CLASS RemoteControl
#define WR_CLASS WorkraveControl
#define WR_CAST WORKRAVE_CONTROL
#define WR_PREFIX workrave_control

#include "macros.h"

#include "Workrave-Control.h"

typedef struct _WorkraveControl       WorkraveControl;
typedef struct _WorkraveControlClass  WorkraveControlClass;

class RemoteControl
{
public:
  //! Constructor.
  RemoteControl();

  //! Destructor
  ~RemoteControl();

  static RemoteControl *get_instance();
  
  WR_METHOD_NOARGS(void, fire);

  WR_METHOD_NOARGS(CORBA_boolean, open_main);
  WR_METHOD_NOARGS(CORBA_boolean, open_preferences);
  WR_METHOD_NOARGS(CORBA_boolean, open_network_connect);
  WR_METHOD_NOARGS(CORBA_boolean, open_network_log);
  
  WR_METHOD_NOARGS(CORBA_boolean, restbreak);
  WR_METHOD       (CORBA_boolean, set_mode, GNOME_Workrave_WorkraveControl_Mode mode);
  WR_METHOD_NOARGS(CORBA_boolean, disconnect_all);
  WR_METHOD_NOARGS(CORBA_boolean, reconnect_all);
  WR_METHOD_NOARGS(CORBA_boolean, quit);
  
private:  
  //! The one and only instance
  static RemoteControl *instance;

  //!
  static WorkraveControl *workrave_control;
};


inline RemoteControl *
RemoteControl::get_instance()
{
  if (instance == NULL)
    {
      instance = new RemoteControl();
    }
       
  return instance;
}

G_BEGIN_DECLS

#define WORKRAVE_CONTROL_TYPE         (workrave_control_get_type())
#define WORKRAVE_CONTROL(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), WORKRAVE_CONTROL_TYPE, WorkraveControl))
#define WORKRAVE_CONTROL_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), WORKRAVE_CONTROL_TYPE, WorkraveControlClass))
#define WORKRAVE_CONTROL_IS_OBJECT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), WORKRAVE_CONTROL_TYPE))
#define WORKRAVE_CONTROL_IS_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), WORKRAVE_CONTROL_TYPE))
#define WORKRAVE_CONTROL_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), WORKRAVE_CONTROL_TYPE, WorkraveControlClass))

struct _WorkraveControl
{
  BonoboObject parent;

  RemoteControl *_this;
};

struct _WorkraveControlClass
{
  BonoboObjectClass parent_class;
  POA_GNOME_Workrave_WorkraveControl__epv epv;
};

WR_INIT()

G_END_DECLS

#endif /*__WORKRAVE_CONTROL_H__*/
