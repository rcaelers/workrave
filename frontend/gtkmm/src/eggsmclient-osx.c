/*
 * Copyright (C) 2007 Novell, Inc.
 * Copyright (C) 2008 Red Hat, Inc.
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

/* EggSMClientOSX
 *
 * For details on the OS X logout process, see:
 * http://developer.apple.com/documentation/MacOSX/Conceptual/BPSystemStartup/Articles/BootProcess.html#//apple_ref/doc/uid/20002130-114618
 *
 * EggSMClientOSX registers for the kAEQuitApplication AppleEvent; the
 * handler we register (quit_requested()) will be invoked from inside
 * the quartz event-handling code (specifically, from inside
 * [NSApplication nextEventMatchingMask]) when an AppleEvent arrives.
 * We use AESuspendTheCurrentEvent() and AEResumeTheCurrentEvent() to
 * allow asynchronous / non-main-loop-reentering processing of the
 * quit request. (These are part of the Carbon framework; it doesn't
 * seem to be possible to handle AppleEvents asynchronously from
 * Cocoa.)
 */

#include "config.h"

#include "eggsmclient-private.h"
#include <glib.h>
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>

#define EGG_TYPE_SM_CLIENT_OSX            (egg_sm_client_osx_get_type ())
#define EGG_SM_CLIENT_OSX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_SM_CLIENT_OSX, EggSMClientOSX))
#define EGG_SM_CLIENT_OSX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_SM_CLIENT_OSX, EggSMClientOSXClass))
#define EGG_IS_SM_CLIENT_OSX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_SM_CLIENT_OSX))
#define EGG_IS_SM_CLIENT_OSX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_SM_CLIENT_OSX))
#define EGG_SM_CLIENT_OSX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_SM_CLIENT_OSX, EggSMClientOSXClass))

typedef struct _EggSMClientOSX        EggSMClientOSX;
typedef struct _EggSMClientOSXClass   EggSMClientOSXClass;

struct _EggSMClientOSX {
  EggSMClient parent;

  AppleEvent quit_event, quit_reply;
  gboolean quit_requested, quitting;
};

struct _EggSMClientOSXClass
{
  EggSMClientClass parent_class;

};

static void     sm_client_osx_startup (EggSMClient *client,
				       const char  *client_id);
static void     sm_client_osx_will_quit (EggSMClient *client,
					 gboolean     will_quit);
static gboolean sm_client_osx_end_session (EggSMClient         *client,
					   EggSMClientEndStyle  style,
					   gboolean  request_confirmation);

static pascal OSErr quit_requested (const AppleEvent *, AppleEvent *, long);

G_DEFINE_TYPE (EggSMClientOSX, egg_sm_client_osx, EGG_TYPE_SM_CLIENT)

static void
egg_sm_client_osx_init (EggSMClientOSX *osx)
{
  ;
}

static void
egg_sm_client_osx_class_init (EggSMClientOSXClass *klass)
{
  EggSMClientClass *sm_client_class = EGG_SM_CLIENT_CLASS (klass);

  sm_client_class->startup             = sm_client_osx_startup;
  sm_client_class->will_quit           = sm_client_osx_will_quit;
  sm_client_class->end_session         = sm_client_osx_end_session;
}

EggSMClient *
egg_sm_client_osx_new (void)
{
  return g_object_new (EGG_TYPE_SM_CLIENT_OSX, NULL);
}

static void
sm_client_osx_startup (EggSMClient *client,
		       const char  *client_id)
{
  AEInstallEventHandler (kCoreEventClass, kAEQuitApplication,
			 NewAEEventHandlerUPP (quit_requested),
			 (long)GPOINTER_TO_SIZE (client), false);
}

static gboolean
idle_quit_requested (gpointer client)
{
  egg_sm_client_quit_requested (client);
  return FALSE;
}

static pascal OSErr
quit_requested (const AppleEvent *aevt, AppleEvent *reply, long refcon)
{
  EggSMClient *client = GSIZE_TO_POINTER ((gsize)refcon);
  EggSMClientOSX *osx = GSIZE_TO_POINTER ((gsize)refcon);

  g_return_val_if_fail (!osx->quit_requested, userCanceledErr);
    
  /* FIXME AEInteractWithUser? */

  osx->quit_requested = TRUE;
  AEDuplicateDesc (aevt, &osx->quit_event);
  AEDuplicateDesc (reply, &osx->quit_reply);
  AESuspendTheCurrentEvent (aevt);

  /* Don't emit the "quit_requested" signal immediately, since we're
   * called from a weird point in the guts of gdkeventloop-quartz.c
   */
  g_idle_add (idle_quit_requested, client);
  return noErr;
}

static pascal OSErr
quit_requested_resumed (const AppleEvent *aevt, AppleEvent *reply, long refcon)
{
  EggSMClientOSX *osx = GSIZE_TO_POINTER ((gsize)refcon);

  osx->quit_requested = FALSE;
  return osx->quitting ? noErr : userCanceledErr;
}

static gboolean
idle_will_quit (gpointer client)
{
  EggSMClientOSX *osx = (EggSMClientOSX *)client;

  /* Resume the event with a new handler that will return a value to
   * the system.
   */
  AEResumeTheCurrentEvent (&osx->quit_event, &osx->quit_reply,
			   NewAEEventHandlerUPP (quit_requested_resumed),
			   (long)GPOINTER_TO_SIZE (client));
  AEDisposeDesc (&osx->quit_event);
  AEDisposeDesc (&osx->quit_reply);

  if (osx->quitting)
    egg_sm_client_quit (client);
  return FALSE;
}

static void
sm_client_osx_will_quit (EggSMClient *client,
			 gboolean     will_quit)
{
  EggSMClientOSX *osx = (EggSMClientOSX *)client;

  g_return_if_fail (osx->quit_requested);

  osx->quitting = will_quit;

  /* Finish in an idle handler since the caller might have called
   * egg_sm_client_will_quit() from inside the "quit_requested" signal
   * handler, but may not expect the "quit" signal to arrive during
   * the _will_quit() call.
   */
  g_idle_add (idle_will_quit, client);
}

static gboolean
sm_client_osx_end_session (EggSMClient         *client,
			   EggSMClientEndStyle  style,
			   gboolean             request_confirmation)
{
  static const ProcessSerialNumber loginwindow_psn = { 0, kSystemProcess };
  AppleEvent event = { typeNull, NULL }, reply = { typeNull, NULL };
  AEAddressDesc target;
  AEEventID id;
  OSErr err;

  switch (style)
    {
    case EGG_SM_CLIENT_END_SESSION_DEFAULT:
    case EGG_SM_CLIENT_LOGOUT:
      id = request_confirmation ? kAELogOut : kAEReallyLogOut;
      break;
    case EGG_SM_CLIENT_REBOOT:
      id = request_confirmation ? kAEShowRestartDialog : kAERestart;
      break;
    case EGG_SM_CLIENT_SHUTDOWN:
      id = request_confirmation ? kAEShowShutdownDialog : kAEShutDown;
      break;
    }

  err = AECreateDesc (typeProcessSerialNumber, &loginwindow_psn, 
		      sizeof (loginwindow_psn), &target);
  if (err != noErr)
    {
      g_warning ("Could not create descriptor for loginwindow: %d", err);
      return FALSE;
    }

  err = AECreateAppleEvent (kCoreEventClass, id, &target,
			    kAutoGenerateReturnID, kAnyTransactionID,
			    &event);
  AEDisposeDesc (&target);
  if (err != noErr)
    {
      g_warning ("Could not create logout AppleEvent: %d", err);
      return FALSE;
    }

  err = AESend (&event, &reply, kAENoReply, kAENormalPriority,
		kAEDefaultTimeout, NULL, NULL);
  AEDisposeDesc (&event);
  if (err == noErr)
    AEDisposeDesc (&reply);

  return err == noErr;
}
