// WorkraveApplet.cc
//
// Copyright (C) 2002 Rob Caelers & Raymond Penners
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

#include "WorkraveAppletControl.h"

static void workrave_applet_class_init  (AppletControlClass *klass);
static void workrave_applet_init        (AppletControl *badmood);

static BonoboObjectClass *parent_class = NULL;

BONOBO_TYPE_FUNC_FULL(AppletControl,
                      GNOME_Workrave_AppletControl,
                      BONOBO_OBJECT_TYPE,
                      workrave_applet);


static void
workrave_applet_class_init(AppletControlClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  POA_GNOME_Workrave_AppletControl__epv *epv = &klass->epv;

  parent_class = g_type_class_peek_parent(klass);
  
  // object_class->dispose = pas_book_dispose;
          
  epv->get_socket_id = workrave_applet_get_socket_id;
}


static void
workrave_applet_init (AppletControl *badmood)
{
}


AppletControl*
workrave_applet_new (void)
{
  FILE *f = fopen("/home/robc/bad.txt", "a+");
  Bonobo_RegistrationResult result;

  AppletControl *obj = g_object_new (workrave_applet_get_type (), NULL);
  BonoboObject *object = BONOBO_OBJECT(obj);
    
  result = bonobo_activation_active_server_register("OAFIID:GNOME_Workrave_AppletControl",
                                                    bonobo_object_corba_objref(BONOBO_OBJECT(object)));

  switch (result) {
  case Bonobo_ACTIVATION_REG_SUCCESS:
    fprintf(f, "ok\n");
    break;
  case Bonobo_ACTIVATION_REG_NOT_LISTED:
    fprintf(f, "nl\n");
    break;
  case Bonobo_ACTIVATION_REG_ALREADY_ACTIVE:
    fprintf(f, "aa\n");
    break;
  case Bonobo_ACTIVATION_REG_ERROR:
  default:
    fprintf(f, "err\n");
    break;
  }
  
  fclose(f);
  return obj;
}


CORBA_long     
workrave_applet_get_socket_id(PortableServer_Servant servant, CORBA_Environment * ev)
{
  AppletControl *mood = WR_APPLET_CONTROL(bonobo_object_from_servant(servant));

  return workrave_applet_socket_id;
}

