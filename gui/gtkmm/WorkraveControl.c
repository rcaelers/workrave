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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <gnome.h>
#include <bonobo/bonobo-generic-factory.h>

//#include <panel-applet.h>

#include "WorkraveControl.h"
#include "nls.h"

/************************************************************************/
/* GNOME::WorkraveControl                                               */
/************************************************************************/

static BonoboObjectClass *parent_class = NULL;

BONOBO_TYPE_FUNC_FULL(WorkraveControl,
                      GNOME_Workrave_WorkraveControl,
                      BONOBO_OBJECT_TYPE,
                      workrave_control);


static void
workrave_control_class_init(WorkraveControlClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  POA_GNOME_Workrave_WorkraveControl__epv *epv = &klass->epv;
  FILE *file = fopen("/home/robc/wr.txt", "a+");

  parent_class = g_type_class_peek_parent(klass);

  fprintf(file,"init class !\n");
  fclose(file);
  
  // object_class->dispose = pas_book_dispose;
          
  epv->fire = workrave_control_fire;
}

static void
workrave_control_init(WorkraveControl *applet)
{
  FILE *file = fopen("/home/robc/wr.txt", "a+");
  fprintf(file,"init class !\n");
  fclose(file);
}


WorkraveControl*
workrave_control_new(void)
{
  Bonobo_RegistrationResult result;

  WorkraveControl *control = g_object_new(workrave_control_get_type(), NULL);
  BonoboObject *object = BONOBO_OBJECT(control);
    
  result = bonobo_activation_active_server_register("OAFIID:GNOME_Workrave_WorkraveControl",
                                                    bonobo_object_corba_objref(BONOBO_OBJECT(object)));
  
  return control;
}

extern int run(int argc, char **argv);

void     
workrave_control_fire(PortableServer_Servant servant, CORBA_Environment *ev)
{
  WorkraveControl *control = WORKRAVE_CONTROL(bonobo_object_from_servant(servant));

  char *argv[] = { "workrave" };

  FILE *file = fopen("/home/robc/wr.txt", "a+");
  fprintf(file, "fire!\n");
  fclose(file);
  
  run(sizeof(argv)/sizeof(argv[0]), argv);
}



BonoboObject*
workrave_component_factory(BonoboGenericFactory *factory, const char *iid, void *data)
{
  WorkraveControl *workrave_control = workrave_control_new();

  FILE *file = fopen("/home/robc/wr.txt", "a+");
  fprintf(file,"factory !\n");
  fclose(file);
  
  return BONOBO_OBJECT(workrave_control);
}

