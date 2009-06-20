/* gtktrayicon.h
 * Copyright (C) 2002 Anders Carlsson <andersca@gnu.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GTK_TRAY_ICON_H__
#define __GTK_TRAY_ICON_H__

#include "gtk/gtk.h"

G_BEGIN_DECLS

#define GTK_TYPE_TRAY_ICON		(wrgtk_tray_icon_get_type ())
#define GTK_TRAY_ICON(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_TRAY_ICON, WRGtkTrayIcon))
#define GTK_TRAY_ICON_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_TRAY_ICON, WRGtkTrayIconClass))
#define GTK_IS_TRAY_ICON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_TRAY_ICON))
#define GTK_IS_TRAY_ICON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TRAY_ICON))
#define GTK_TRAY_ICON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_TRAY_ICON, WRGtkTrayIconClass))
	
typedef struct _WRGtkTrayIcon	   WRGtkTrayIcon;
typedef struct _WRGtkTrayIconPrivate WRGtkTrayIconPrivate;
typedef struct _WRGtkTrayIconClass   WRGtkTrayIconClass;

struct _WRGtkTrayIcon
{
  GtkPlug parent_instance;

  WRGtkTrayIconPrivate *priv;
};

struct _WRGtkTrayIconClass
{
  GtkPlugClass parent_class;

  void (*__gtk_reserved1);
  void (*__gtk_reserved2);
  void (*__gtk_reserved3);
  void (*__gtk_reserved4);
  void (*__gtk_reserved5);
  void (*__gtk_reserved6);
};

GType          wrgtk_tray_icon_get_type         (void) G_GNUC_CONST;

WRGtkTrayIcon   *wrgtk_tray_icon_new_for_screen  (GdkScreen   *screen,
					       const gchar *name);

WRGtkTrayIcon   *wrgtk_tray_icon_new             (const gchar *name);

guint          _wrgtk_tray_icon_send_message    (WRGtkTrayIcon *icon,
					       gint         timeout,
					       const gchar *message,
					       gint         len);
void           _wrgtk_tray_icon_cancel_message  (WRGtkTrayIcon *icon,
					       guint        id);

GtkOrientation wrgtk_tray_icon_get_orientation (WRGtkTrayIcon *icon);
					    
G_END_DECLS

#endif /* __GTK_TRAY_ICON_H__ */
