#include <config.h>
#ifdef HAVE_X

#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include "gnome-winhints.h"

/* these are the X atoms for the hints we use */
static Atom _XA_WIN_PROTOCOLS;
static Atom _XA_WIN_WORKSPACE;
static Atom _XA_WIN_WORKSPACE_COUNT;
static Atom _XA_WIN_WORKSPACE_NAMES;
static Atom _XA_WIN_STATE;
static Atom _XA_WIN_HINTS;
static Atom _XA_WIN_PROTOCOLS;
static Atom _XA_WIN_LAYER;
static Atom _XA_WIN_ICONS;
static Atom _XA_WIN_CLIENT_LIST;
static Atom _XA_WIN_APP_STATE;
static Atom _XA_WIN_EXPANDED_SIZE;
static Atom _XA_WIN_CLIENT_MOVING;
static Atom _XA_WIN_SUPPORTING_WM_CHECK;

static int need_init = TRUE;

void
gnome_win_hints_init(void)
{
  /* Get the atoms we are working with, creating them if necessary.
   */
  g_return_if_fail(GDK_DISPLAY());
  _XA_WIN_PROTOCOLS = XInternAtom(GDK_DISPLAY(), XA_WIN_PROTOCOLS, False);
  _XA_WIN_STATE = XInternAtom(GDK_DISPLAY(), XA_WIN_STATE, False);
  _XA_WIN_WORKSPACE = XInternAtom(GDK_DISPLAY(), XA_WIN_WORKSPACE, False);
  _XA_WIN_WORKSPACE_COUNT = XInternAtom(GDK_DISPLAY(), XA_WIN_WORKSPACE_COUNT, False);
  _XA_WIN_WORKSPACE_NAMES = XInternAtom(GDK_DISPLAY(), XA_WIN_WORKSPACE_NAMES, False);
  _XA_WIN_LAYER = XInternAtom(GDK_DISPLAY(), XA_WIN_LAYER, False);
  _XA_WIN_PROTOCOLS = XInternAtom(GDK_DISPLAY(), XA_WIN_PROTOCOLS, False);
  _XA_WIN_HINTS = XInternAtom(GDK_DISPLAY(), XA_WIN_HINTS, False);
  _XA_WIN_ICONS = XInternAtom(GDK_DISPLAY(), XA_WIN_ICONS, False);
  _XA_WIN_CLIENT_LIST = XInternAtom(GDK_DISPLAY(), XA_WIN_CLIENT_LIST, False);
  _XA_WIN_APP_STATE = XInternAtom(GDK_DISPLAY(), XA_WIN_APP_STATE, False);
  _XA_WIN_EXPANDED_SIZE = XInternAtom(GDK_DISPLAY(), XA_WIN_EXPANDED_SIZE, False);
  _XA_WIN_CLIENT_MOVING = XInternAtom(GDK_DISPLAY(), XA_WIN_CLIENT_MOVING, False);
  _XA_WIN_SUPPORTING_WM_CHECK = XInternAtom(GDK_DISPLAY(), XA_WIN_SUPPORTING_WM_CHECK, False);
  need_init = FALSE;
}

void
gnome_win_hints_set_layer(GtkWidget *window, GnomeWinLayer layer)
{
  XEvent xev;
  Window xwindow = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);
  
  if (need_init)
    gnome_win_hints_init();
  
  if (GTK_WIDGET_MAPPED(window))
    {
      xev.type = ClientMessage;
      xev.xclient.type = ClientMessage;
      xev.xclient.window = xwindow;
      xev.xclient.message_type = _XA_WIN_LAYER;
      xev.xclient.format = 32;
      xev.xclient.data.l[0] = (long)layer;
      xev.xclient.data.l[1] = gdk_x11_get_server_time(GTK_WIDGET(window)->window);
      
      XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False,
		 SubstructureNotifyMask, (XEvent*) &xev);
    }
  else
    {
      long data[1];
      
      data[0] = layer;
      XChangeProperty(GDK_DISPLAY(), xwindow, _XA_WIN_LAYER,
		      XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data,
		      1);
    }
}

GnomeWinLayer
gnome_win_hints_get_layer(GtkWidget *window)
{
  Window xwindow = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);
  GnomeWinLayer mylayer;
  Atom r_type;
  int r_format;
  unsigned long count;
  unsigned long bytes_remain;
  unsigned char *prop;
  long layer;

  if (need_init)
    gnome_win_hints_init();
  
  if (XGetWindowProperty(GDK_DISPLAY(), xwindow, _XA_WIN_LAYER, 0, 1,
			 False, XA_CARDINAL, &r_type, &r_format,
			 &count, &bytes_remain, &prop) == Success)
    {
      if (r_type == XA_CARDINAL && r_format == 32 && count == 1)
	{
	  layer = ((long *)prop)[0];
	  mylayer = (GnomeWinLayer)layer;
	  XFree(prop);
	  return mylayer;
	}
      XFree(prop);
    }
  return WIN_LAYER_NORMAL;
}

void
gnome_win_hints_set_state(GtkWidget *window, GnomeWinState state)
{
  Window xwindow = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);
  XEvent xev;
  
  if (need_init)
    gnome_win_hints_init();
  
  if (GTK_WIDGET_MAPPED(window))
    {
      xev.type = ClientMessage;
      xev.xclient.type = ClientMessage;
      xev.xclient.window = xwindow;
      xev.xclient.message_type = _XA_WIN_STATE;
      xev.xclient.format = 32;
      xev.xclient.data.l[0] = (long)(WIN_STATE_STICKY |
				     WIN_STATE_MAXIMIZED_VERT |
				     WIN_STATE_MAXIMIZED_HORIZ |
				     WIN_STATE_HIDDEN |
				     WIN_STATE_SHADED |
				     WIN_STATE_HID_WORKSPACE |
				     WIN_STATE_HID_TRANSIENT |
				     WIN_STATE_FIXED_POSITION |
				     WIN_STATE_ARRANGE_IGNORE);
      xev.xclient.data.l[1] = (long)state;
      xev.xclient.data.l[2] = gdk_x11_get_server_time(GTK_WIDGET(window)->window);

      XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False, 
		 SubstructureNotifyMask, (XEvent*) &xev);
    }
  else
    {
      long data[1];
      
      data[0] = (long)state;
      XChangeProperty(GDK_DISPLAY(), xwindow, _XA_WIN_STATE,
		      XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data,
		      1);
    }
}

GnomeWinState
gnome_win_hints_get_state(GtkWidget *window)
{
  Window xwindow = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);
  GnomeWinState state;
  Atom r_type;
  int r_format;
  unsigned long count;
  unsigned long bytes_remain;
  unsigned char *prop;
  
  if (need_init)
    gnome_win_hints_init();
  

  if (XGetWindowProperty(GDK_DISPLAY(), xwindow, _XA_WIN_STATE, 0, 1,
			 False, XA_CARDINAL, &r_type, &r_format,
			 &count, &bytes_remain, &prop) == Success)
    {
      if (r_type == XA_CARDINAL && r_format == 32 && count == 2)
	{
	  state = (GnomeWinState)(((long *)prop)[0]) && 
	    (GnomeWinState)(((long *)prop)[1]);
	  XFree(prop);
	  return state;
	}
      else if (r_type == XA_CARDINAL && r_format == 32 && count == 1)
	{
	  state = (GnomeWinState)(((long *)prop)[0]);
	  XFree(prop);
	  return state;
	}
      XFree(prop);
    }
  return (GnomeWinState)0;
}

void
gnome_win_hints_set_hints(GtkWidget *window,  GnomeWinHints hints)
{
  Window xwindow = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);
  XEvent xev;
  
  if (need_init)
    gnome_win_hints_init();
  
  if (GTK_WIDGET_MAPPED(window))
    {
      xev.type = ClientMessage;
      xev.xclient.type = ClientMessage;
      xev.xclient.window = xwindow;
      xev.xclient.message_type = _XA_WIN_HINTS;
      xev.xclient.format = 32;
      xev.xclient.data.l[0] = (long)(WIN_HINTS_SKIP_FOCUS | 
				     WIN_HINTS_SKIP_WINLIST | 
				     WIN_HINTS_SKIP_TASKBAR | 
				     WIN_HINTS_GROUP_TRANSIENT |
				     WIN_HINTS_FOCUS_ON_CLICK |
				     WIN_HINTS_DO_NOT_COVER);
      xev.xclient.data.l[1] = (long)hints;
      xev.xclient.data.l[2] = gdk_x11_get_server_time(GTK_WIDGET(window)->window);
      
      XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False, 
		 SubstructureNotifyMask, (XEvent*) &xev);
    }
  else
    {
      long data[1];
      
      data[0] = (long)hints;
      XChangeProperty(GDK_DISPLAY(), xwindow, _XA_WIN_HINTS,
		      XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data,
		      1);
    }
}

GnomeWinHints
gnome_win_hints_get_hints(GtkWidget *window)
{
  Window xwindow = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);
  GnomeWinHints hints;
  Atom r_type;
  int r_format;
  unsigned long count;
  unsigned long bytes_remain;
  unsigned char *prop;
    
  if (need_init)
    gnome_win_hints_init();
  
  if (XGetWindowProperty(GDK_DISPLAY(), xwindow, _XA_WIN_HINTS, 0, 1,
			 False, XA_CARDINAL, &r_type, &r_format,
			 &count, &bytes_remain, &prop) == Success)
    {
      if (r_type == XA_CARDINAL && r_format == 32 && count == 2)
	{
	  hints = (GnomeWinState)(((long *)prop)[0]) && 
	    (GnomeWinState)(((long *)prop)[1]);
	  XFree(prop);
	  return hints;
	}
      else if (r_type == XA_CARDINAL && r_format == 32 && count == 1)
	{
	  hints = (GnomeWinState)(((long *)prop)[0]);
	  XFree(prop);
	  return hints;
	}

      XFree(prop);
    }
  return (GnomeWinState)0;
}

void
gnome_win_hints_set_workspace(GtkWidget *window, gint workspace)
{
  Window xwindow = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);
  long data[1];
  
  if (need_init)
    gnome_win_hints_init();
  
  data[0] = (long)workspace;
  XChangeProperty(GDK_DISPLAY(), xwindow, _XA_WIN_WORKSPACE,
		  XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data,
		  1);
}

gint
gnome_win_hints_get_workspace(GtkWidget *window)
{
  Window xwindow = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);
  gint ws;
  Atom r_type;
  int r_format;
  unsigned long count;
  unsigned long bytes_remain;
  unsigned char *prop;
  
  if (need_init)
    gnome_win_hints_init();
  
  if (XGetWindowProperty(GDK_DISPLAY(), xwindow, _XA_WIN_WORKSPACE, 0, 1,
			 False, XA_CARDINAL, &r_type, &r_format,
			 &count, &bytes_remain, &prop) == Success)
    {
      if (r_type == XA_CARDINAL && r_format == 32 && count == 1)
	{
	  ws = (gint)(((long *)prop)[0]); 
	  XFree(prop);
	  return ws;
	}
      XFree(prop);
    }
  return 0;
}

void
gnome_win_hints_set_current_workspace(gint workspace)
{
  XEvent xev;
  
  if (need_init)
    gnome_win_hints_init();
  
  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.window = GDK_ROOT_WINDOW();
  xev.xclient.message_type = _XA_WIN_WORKSPACE;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = workspace;
  xev.xclient.data.l[1] = 0; //FIXME: gdk_x11_get_server_time(GDK_ROOT_WINDOW());
  
  XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False,
	     SubstructureNotifyMask, (XEvent*) &xev);
}

gint
gnome_win_hints_get_current_workspace(void)
{
  Atom r_type;
  int r_format;
  unsigned long count;
  unsigned long bytes_remain;
  unsigned char *prop;
  long ws = 0;
  
  if (need_init)
    gnome_win_hints_init();
  
  if (XGetWindowProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(),
			 _XA_WIN_WORKSPACE, 0, 1, False, XA_CARDINAL,
			 &r_type, &r_format,
			 &count, &bytes_remain, &prop) == Success)
    {
      if (r_type == XA_CARDINAL && r_format == 32 && count == 1)
        {
	  long n = *(long *)prop;
	  
	  ws = (gint)n;
        }
      XFree(prop);
    }       
  return ws;
}

GList*
gnome_win_hints_get_workspace_names(void)
{
  GList *tmp_list;
  XTextProperty tp;
  char **list;
  int count, i;
  
  if (need_init)
    gnome_win_hints_init();
  
  XGetTextProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(),
		   &tp, _XA_WIN_WORKSPACE_NAMES);  
  XTextPropertyToStringList(&tp, &list, &count);
  
  if (tp.value==NULL) 
    {
      return NULL; /* current wm does not support this! */
    }
  
  tmp_list = NULL;
  for(i=0; i<count; i++)
    {
      tmp_list = g_list_append(tmp_list, g_strdup(list[i]));
    }  
  return tmp_list;
}

gint
gnome_win_hints_get_workspace_count(void)
{
  gint wscount;
  Atom r_type;
  int r_format;
  unsigned long count;
  unsigned long bytes_remain;
  unsigned char *prop;
  
  if (need_init)
    gnome_win_hints_init();
  
  wscount = 1;
  if (XGetWindowProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(),
			 _XA_WIN_WORKSPACE_COUNT, 0, 1, False, XA_CARDINAL,
			 &r_type, &r_format,
			 &count, &bytes_remain, &prop) == Success && prop)
    {
      if (r_type == XA_CARDINAL && r_format == 32 && count == 1)
        {
	  long n = *(long *)prop;
	  wscount = (gint)n;
        }
      XFree(prop);
    }       
  return wscount;
}

void
gnome_win_hints_set_expanded_size(GtkWidget *window, gint x, gint y, gint width, gint height)
{
  Window xwindow = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);

  long data[4];
  
  if (need_init)
    gnome_win_hints_init();
  
  data[0] = (long)x;
  data[1] = (long)y;
  data[2] = (long)width;
  data[3] = (long)height;
  XChangeProperty(GDK_DISPLAY(), xwindow, _XA_WIN_APP_STATE,
		  XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data,
		  4);
}

gboolean
gnome_win_hints_get_expanded_size(GtkWidget *window, gint *x, gint *y, gint *width, gint *height)
{
  Window xwindow = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);
  Atom r_type;
  int r_format;
  unsigned long count;
  unsigned long bytes_remain;
  unsigned char *prop;
  
  if (need_init)
    gnome_win_hints_init();
  
  if (XGetWindowProperty(GDK_DISPLAY(), xwindow, _XA_WIN_APP_STATE, 0, 1,
			 False, XA_CARDINAL, &r_type, &r_format,
			 &count, &bytes_remain, &prop) == Success)
    {
      if (r_type == XA_CARDINAL && r_format == 32 && count == 4)
	{
	  if (x)
	    *x = (gint)(((long *)prop)[0]);
	  if (y)
	    *y = (gint)(((long *)prop)[1]);
	  if (width)
	    *width = (gint)(((long *)prop)[2]);
	  if (height)
	    *height = (gint)(((long *)prop)[3]);
	  XFree(prop);
	  return TRUE;
	}
      XFree(prop);
    }
  return FALSE;
}

void
gnome_win_hints_set_app_state(GtkWidget *window,  GnomeWinAppState state)
{
  Window xwindow = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);
  long data[1];
  
  if (need_init)
    gnome_win_hints_init();
  
  data[0] = (long)state;
  XChangeProperty(GDK_DISPLAY(), xwindow, _XA_WIN_APP_STATE,
		  XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data,
		  1);
}

GnomeWinAppState
gnome_win_hints_get_app_state(GtkWidget *window)
{
  Window xwindow = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);
  GnomeWinAppState state;
  Atom r_type;
  int r_format;
  unsigned long count;
  unsigned long bytes_remain;
  unsigned char *prop;
  
  if (need_init)
    gnome_win_hints_init();
  
  if (XGetWindowProperty(GDK_DISPLAY(), xwindow, _XA_WIN_APP_STATE, 0, 1,
			 False, XA_CARDINAL, &r_type, &r_format,
			 &count, &bytes_remain, &prop) == Success)
    {
      if (r_type == XA_CARDINAL && r_format == 32 && count == 2)
	{
	  state = (GnomeWinAppState)(((long *)prop)[0]);
	  XFree(prop);
	  return state;
	}
      XFree(prop);
    }

  return WIN_APP_STATE_NONE;
}

void
gnome_win_hints_set_moving(GtkWidget *window, gboolean moving)
{
  Window xwindow = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);
  long data[1];
  
  if (need_init)
    gnome_win_hints_init();
  
  if (moving)
    data[0] = 1;
  else
    data[0] = 0;
  XChangeProperty(GDK_DISPLAY(), xwindow, _XA_WIN_CLIENT_MOVING,
		  XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data,
		  1);
}

gboolean
gnome_win_hints_wm_exists(void)
{
  Atom r_type;
  int r_format;
  unsigned long count;
  unsigned long bytes_remain;
  unsigned char *prop, *prop2;
  
  if (need_init)
    gnome_win_hints_init();
  
  if (XGetWindowProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(),
			 _XA_WIN_SUPPORTING_WM_CHECK, 0, 1, False, XA_CARDINAL,
			 &r_type, &r_format,
			 &count, &bytes_remain, &prop) == Success && prop)
    {
      if (r_type == XA_CARDINAL && r_format == 32 && count == 1)
        {
	  Window n = *(long *)prop;
	  if (XGetWindowProperty(GDK_DISPLAY(), n,
				 _XA_WIN_SUPPORTING_WM_CHECK, 0, 1, False, 
				 XA_CARDINAL,
				 &r_type, &r_format, &count, &bytes_remain, 
				 &prop2) == Success && prop)
	    {
	      if (r_type == XA_CARDINAL && r_format == 32 && count == 1)
		{
		  XFree(prop);
		  XFree(prop2);
		  return TRUE;
		}
	      XFree(prop2);
	    }       
        }
      XFree(prop);
    }       
  return FALSE;
}

GList*
gnome_win_hints_get_client_window_ids(void)
{
  GList *tmp_list;
  Window *wlist;
  unsigned int i;
  Atom r_type;
  int r_format;
  unsigned long count;
  unsigned long bytes_remain;
  unsigned char *prop;
  
  if (need_init)
    gnome_win_hints_init();
  
  if (XGetWindowProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(), 
			 _XA_WIN_CLIENT_LIST, 0, 1,
			 False, XA_CARDINAL, &r_type, &r_format,
			 &count, &bytes_remain, &prop) == Success)
    {
      if (r_type == XA_CARDINAL && r_format == 32)
	{
	  tmp_list = NULL;
	  
	  wlist = (Window *)prop;
	  for(i=0; i<count; i++)
	    {
	      tmp_list = g_list_append(tmp_list, (gpointer)wlist[i]);
	    }  
	  XFree(prop);
	  return tmp_list;
	}
      XFree(prop);
    }
  return NULL;
}

#endif
