// WmSpec.cc 
//
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_X

#include "debug.hh"

#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>

#include "WmSpec.hh"

#define XA_NET_SUPPORTING_WM_CHECK	"_NET_SUPPORTING_WM_CHECK"
#define XA_NET_WM_STATE			"_NET_WM_STATE"

#define _NET_WM_STATE_REMOVE   0
#define _NET_WM_STATE_ADD      1
#define _NET_WM_STATE_TOGGLE   2


bool
WmSpec::supported()
{
  TRACE_ENTER("WmSpec::supported");

  Atom r_type, support_check;
  int r_format, p;
  unsigned long count, bytes_remain;
  unsigned char *prop = NULL, *prop2 = NULL;
  bool ret = false;

  support_check = gdk_x11_get_xatom_by_name(XA_NET_SUPPORTING_WM_CHECK);

  p = XGetWindowProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(), support_check,
                         0, 1, False, XA_WINDOW, &r_type, &r_format,
                         &count, &bytes_remain, &prop);

  if (p == Success && prop && r_type == XA_WINDOW && r_format == 32 && count == 1)
    {
      Window n = *(Window *) prop;
      
      p = XGetWindowProperty(GDK_DISPLAY(), n, support_check, 0, 1,
                             False, XA_WINDOW, &r_type, &r_format,
                             &count, &bytes_remain, &prop2);
      
      if (p == Success && prop2 && *prop2 == *prop &&
          r_type == XA_WINDOW && r_format == 32 && count == 1)
        ret = true;
    }

  if (prop)
    XFree(prop);
  if (prop2)
    XFree(prop2);

  TRACE_EXIT();
  return ret;
}



void
WmSpec::change_state(GtkWidget *gtk_window, bool add, const char *state)
{
  TRACE_ENTER_MSG("WmSpec::change_state", state);

  GdkWindow *window = GTK_WIDGET(gtk_window)->window;
  g_return_if_fail (GDK_IS_WINDOW(window));
  
  if (GDK_WINDOW_DESTROYED(window))
    return;

  Atom wm_state_atom = gdk_x11_get_xatom_by_name(XA_NET_WM_STATE);
  Atom wm_value_atom =  gdk_x11_get_xatom_by_name(state);

  if (GTK_WIDGET_MAPPED(gtk_window))
    {
      XEvent xev;

      TRACE_MSG(wm_state_atom << " " << wm_value_atom);
  
      xev.type = ClientMessage;
      xev.xclient.type = ClientMessage;
      xev.xclient.serial = 0;
      xev.xclient.send_event = True;
      xev.xclient.display = GDK_DISPLAY();
      xev.xclient.window = GDK_WINDOW_XID(gtk_window->window);
      xev.xclient.message_type = wm_state_atom;
      xev.xclient.format = 32;
      xev.xclient.data.l[0] = add ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
      xev.xclient.data.l[1] = wm_value_atom;
      xev.xclient.data.l[2] = 0;
      
      XSendEvent (GDK_DISPLAY(), GDK_ROOT_WINDOW(), False,
                  SubstructureRedirectMask | SubstructureNotifyMask,
                  &xev);
    }
  else
    {
      Atom atoms[1];
      atoms[0] = wm_value_atom;
      XChangeProperty(GDK_DISPLAY(),
                      GDK_WINDOW_XID(gtk_window->window),
                      wm_state_atom,
                      XA_ATOM, 32, PropModeAppend,
                      (guchar*) atoms, 1);
    }
  TRACE_EXIT();
}


void
WmSpec::set_window_hint(GtkWidget *gtk_window, const char *type)
{
  g_return_if_fail (gtk_window != NULL);

  GdkWindow *window = GTK_WIDGET(gtk_window)->window;
  
  g_return_if_fail (GDK_IS_WINDOW (window));
  
  if (GDK_WINDOW_DESTROYED (window))
    return;

  Atom type_atom = gdk_x11_get_xatom_by_name(type);

  XChangeProperty (GDK_WINDOW_XDISPLAY (window),
		   GDK_WINDOW_XID (window),
		   gdk_x11_get_xatom_by_name ("_NET_WM_WINDOW_TYPE"),
		   XA_ATOM, 32, PropModeReplace,
		   (guchar *)&type_atom, 1);
}

#endif
