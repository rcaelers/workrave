#ifndef __WORKRAVE_INTERFACE_H__
#define __WORKRAVE_INTERFACE_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define WORKRAVE_INTERFACE_TYPE            (workrave_interface_get_type ())
#define WORKRAVE_INTERFACE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), WORKRAVE_INTERFACE_TYPE, WorkraveInterface))
#define WORKRAVE_INTERFACE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), WORKRAVE_INTERFACE_TYPE, WorkraveInterfaceClass))
#define IS_WORKRAVE_INTERFACE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WORKRAVE_INTERFACE_TYPE))
#define IS_WORKRAVE_INTERFACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WORKRAVE_INTERFACE_TYPE))
#define WORKRAVE_INTERFACE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), WORKRAVE_INTERFACE_TYPE, WorkraveInterfaceClass))

typedef struct _WorkraveInterface        WorkraveInterface;
typedef struct _WorkraveInterfacePrivate WorkraveInterfacePrivate;
typedef struct _WorkraveInterfaceClass   WorkraveInterfaceClass;

struct _WorkraveInterfaceClass {
	GObjectClass parent_class;

	void (*update) (void);
};

struct _WorkraveInterface {
	GObject parent;
	WorkraveInterfacePrivate * priv;
};

GType              workrave_interface_get_type       (void);
void               workrave_interface_update         (WorkraveInterface *self);

G_END_DECLS

#endif
