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

#include "RemoteControl.hh"

RemoteControl   *RemoteControl::instance = NULL;
WorkraveControl *RemoteControl::workrave_control = NULL;

#include "nls.h"

/************************************************************************/
/* GNOME::RemoteControl                                                 */
/************************************************************************/

RemoteControl::RemoteControl()
{
}


RemoteControl::~RemoteControl()
{
}


WR_METHOD_NOARGS_IMPL(void, fire)
{
}


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
  parent_class = (BonoboObjectClass *)g_type_class_peek_parent(klass);

  WR_METHOD_REGISTER(fire);
}

static void
workrave_control_init(WorkraveControl *applet)
{
  RemoteControl *control = RemoteControl::get_instance();
  applet->_this = control;
}


WorkraveControl*
workrave_control_new(void)
{
  Bonobo_RegistrationResult result;

  WorkraveControl *control = (WorkraveControl *)g_object_new(workrave_control_get_type(), NULL);
  BonoboObject *object = BONOBO_OBJECT(control);
    
  result = bonobo_activation_active_server_register("OAFIID:GNOME_Workrave_WorkraveControl",
                                                    bonobo_object_corba_objref(BONOBO_OBJECT(object)));
  
  return control;
}


extern "C" BonoboObject*
workrave_component_factory(BonoboGenericFactory *factory, const char *iid, void *data)
{
  WorkraveControl *workrave_control = workrave_control_new();
  return BONOBO_OBJECT(workrave_control);
}
