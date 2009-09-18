/*
 * Copyright (C) 2007 Novell, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* EggSMClientWin32
 *
 * For details on the Windows XP logout process, see:
 * http://msdn.microsoft.com/en-us/library/aa376876.aspx.
 *
 * Vista adds some new APIs which EggSMClient does not make use of; see
 * http://msdn.microsoft.com/en-us/library/ms700677(VS.85).aspx
 *
 * When shutting down, Windows sends every top-level window a
 * WM_QUERYENDSESSION event, which the application must respond to
 * synchronously, saying whether or not it will quit. To avoid main
 * loop re-entrancy problems (and to avoid having to muck about too
 * much with the guts of the gdk-win32 main loop), we watch for this
 * event in a separate thread, which then signals the main thread and
 * waits for the main thread to handle the event. Since we don't want
 * to require g_thread_init() to be called, we do this all using
 * Windows-specific thread methods.
 *
 * After the application handles the WM_QUERYENDSESSION event,
 * Windows then sends it a WM_ENDSESSION event with a TRUE or FALSE
 * parameter indicating whether the session is or is not actually
 * going to end now. We handle this from the other thread as well.
 *
 * As mentioned above, Vista introduces several additional new APIs
 * that don't fit into the (current) EggSMClient API. Windows also has
 * an entirely separate shutdown-notification scheme for non-GUI apps,
 * which we also don't handle here.
 */

#include "config.h"

#include "eggsmclient-private.h"
#include <gdk/gdk.h>

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <process.h>

#define EGG_TYPE_SM_CLIENT_WIN32            (egg_sm_client_win32_get_type ())
#define EGG_SM_CLIENT_WIN32(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_SM_CLIENT_WIN32, EggSMClientWin32))
#define EGG_SM_CLIENT_WIN32_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_SM_CLIENT_WIN32, EggSMClientWin32Class))
#define EGG_IS_SM_CLIENT_WIN32(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_SM_CLIENT_WIN32))
#define EGG_IS_SM_CLIENT_WIN32_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_SM_CLIENT_WIN32))
#define EGG_SM_CLIENT_WIN32_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_SM_CLIENT_WIN32, EggSMClientWin32Class))

typedef struct _EggSMClientWin32        EggSMClientWin32;
typedef struct _EggSMClientWin32Class   EggSMClientWin32Class;

struct _EggSMClientWin32 {
  EggSMClient parent;

  HANDLE message_event, response_event;

  volatile GSourceFunc event;
  volatile gboolean will_quit;
};

struct _EggSMClientWin32Class
{
  EggSMClientClass parent_class;

};

static void     sm_client_win32_startup (EggSMClient *client,
					 const char  *client_id);
static void     sm_client_win32_will_quit (EggSMClient *client,
					   gboolean     will_quit);
static gboolean sm_client_win32_end_session (EggSMClient         *client,
					     EggSMClientEndStyle  style,
					     gboolean  request_confirmation);

static GSource *g_win32_handle_source_add (HANDLE handle, GSourceFunc callback,
					gpointer user_data);
static gboolean got_message (gpointer user_data);
static void sm_client_thread (gpointer data);

G_DEFINE_TYPE (EggSMClientWin32, egg_sm_client_win32, EGG_TYPE_SM_CLIENT)

static void
egg_sm_client_win32_init (EggSMClientWin32 *win32)
{
  ;
}

static void
egg_sm_client_win32_class_init (EggSMClientWin32Class *klass)
{
  EggSMClientClass *sm_client_class = EGG_SM_CLIENT_CLASS (klass);

  sm_client_class->startup             = sm_client_win32_startup;
  sm_client_class->will_quit           = sm_client_win32_will_quit;
  sm_client_class->end_session         = sm_client_win32_end_session;
}

EggSMClient *
egg_sm_client_win32_new (void)
{
  return g_object_new (EGG_TYPE_SM_CLIENT_WIN32, NULL);
}

static void
sm_client_win32_startup (EggSMClient *client,
			 const char  *client_id)
{
  EggSMClientWin32 *win32 = (EggSMClientWin32 *)client;

  win32->message_event = CreateEvent (NULL, FALSE, FALSE, NULL);
  win32->response_event = CreateEvent (NULL, FALSE, FALSE, NULL);
  g_win32_handle_source_add (win32->message_event, got_message, win32);  
  _beginthread (sm_client_thread, 0, client);
}

static void
sm_client_win32_will_quit (EggSMClient *client,
			   gboolean     will_quit)
{
  EggSMClientWin32 *win32 = (EggSMClientWin32 *)client;

  win32->will_quit = will_quit;
  SetEvent (win32->response_event);
}

static gboolean
sm_client_win32_end_session (EggSMClient         *client,
			     EggSMClientEndStyle  style,
			     gboolean             request_confirmation)
{
  UINT uFlags = EWX_LOGOFF;

  switch (style)
    {
    case EGG_SM_CLIENT_END_SESSION_DEFAULT:
    case EGG_SM_CLIENT_LOGOUT:
      uFlags = EWX_LOGOFF;
      break;
    case EGG_SM_CLIENT_REBOOT:
      uFlags = EWX_REBOOT;
      break;
    case EGG_SM_CLIENT_SHUTDOWN:
      uFlags = EWX_POWEROFF;
      break;
    }

  /* There's no way to make ExitWindowsEx() show a logout dialog, so
   * we ignore @request_confirmation.
   */

#ifdef SHTDN_REASON_FLAG_PLANNED
  ExitWindowsEx (uFlags, SHTDN_REASON_FLAG_PLANNED);
#else
  ExitWindowsEx (uFlags, 0);
#endif

  return TRUE;
}


/* callbacks from logout-listener thread */

static gboolean
emit_quit_requested (gpointer smclient)
{
  gdk_threads_enter ();
  egg_sm_client_quit_requested (smclient);
  gdk_threads_leave ();

  return FALSE;
}

static gboolean
emit_quit (gpointer smclient)
{
  EggSMClientWin32 *win32 = smclient;

  gdk_threads_enter ();
  egg_sm_client_quit (smclient);
  gdk_threads_leave ();

  SetEvent (win32->response_event);
  return FALSE;
}

static gboolean
emit_quit_cancelled (gpointer smclient)
{
  EggSMClientWin32 *win32 = smclient;

  gdk_threads_enter ();
  egg_sm_client_quit_cancelled (smclient);
  gdk_threads_leave ();

  SetEvent (win32->response_event);
  return FALSE;
}

static gboolean
got_message (gpointer smclient)
{
  EggSMClientWin32 *win32 = smclient;

  win32->event (win32);
  return TRUE;
}

/* Windows HANDLE GSource */

typedef struct {
  GSource source;
  GPollFD pollfd;
} GWin32HandleSource;

static gboolean
g_win32_handle_source_prepare (GSource *source, gint *timeout)
{
  *timeout = -1;
  return FALSE;
}

static gboolean
g_win32_handle_source_check (GSource *source)
{
  GWin32HandleSource *hsource = (GWin32HandleSource *)source;

  return hsource->pollfd.revents;
}

static gboolean
g_win32_handle_source_dispatch (GSource *source, GSourceFunc callback, gpointer user_data)
{
  return (*callback) (user_data);
}

static void
g_win32_handle_source_finalize (GSource *source)
{
  ;
}

GSourceFuncs g_win32_handle_source_funcs = {
  g_win32_handle_source_prepare,
  g_win32_handle_source_check,
  g_win32_handle_source_dispatch,
  g_win32_handle_source_finalize
};

static GSource *
g_win32_handle_source_add (HANDLE handle, GSourceFunc callback, gpointer user_data)
{
  GWin32HandleSource *hsource;
  GSource *source;

  source = g_source_new (&g_win32_handle_source_funcs, sizeof (GWin32HandleSource));
  hsource = (GWin32HandleSource *)source;
  hsource->pollfd.fd = (int)handle;
  hsource->pollfd.events = G_IO_IN;
  hsource->pollfd.revents = 0;
  g_source_add_poll (source, &hsource->pollfd);

  g_source_set_callback (source, callback, user_data, NULL);
  g_source_attach (source, NULL);
  return source;
}

/* logout-listener thread */

LRESULT CALLBACK
sm_client_win32_window_procedure (HWND   hwnd,
				  UINT   message,
				  WPARAM wParam,
				  LPARAM lParam)
{
  EggSMClientWin32 *win32 =
    (EggSMClientWin32 *)GetWindowLongPtr (hwnd, GWLP_USERDATA);

  switch (message)
    {
    case WM_QUERYENDSESSION:
      win32->event = emit_quit_requested;
      SetEvent (win32->message_event);

      WaitForSingleObject (win32->response_event, INFINITE);
      return win32->will_quit;

    case WM_ENDSESSION:
      if (wParam)
	{
	  /* The session is ending */
	  win32->event = emit_quit;
	}
      else
	{
	  /* Nope, the session *isn't* ending */
	  win32->event = emit_quit_cancelled;
	}

      SetEvent (win32->message_event);
      WaitForSingleObject (win32->response_event, INFINITE);

      return 0;

    default:
      return DefWindowProc (hwnd, message, wParam, lParam);
    }
}

static void
sm_client_thread (gpointer smclient)
{
  HINSTANCE instance;
  WNDCLASSEXW wcl; 
  ATOM klass;
  HWND window;
  MSG msg;

  instance = GetModuleHandle (NULL);

  memset (&wcl, 0, sizeof (WNDCLASSEX));
  wcl.cbSize = sizeof (WNDCLASSEX);
  wcl.lpfnWndProc = sm_client_win32_window_procedure;
  wcl.hInstance = instance;
  wcl.lpszClassName = L"EggSmClientWindow";
  klass = RegisterClassEx (&wcl);

  window = CreateWindowEx (0, MAKEINTRESOURCE (klass),
			   L"EggSmClientWindow", 0,
			   10, 10, 50, 50, GetDesktopWindow (),
			   NULL, instance, NULL);
  SetWindowLongPtr (window, GWLP_USERDATA, (LONG_PTR)smclient);

  /* main loop */
  while (GetMessage (&msg, NULL, 0, 0))
    DispatchMessage (&msg);
}
