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

#include "debug.hh"

#include "RemoteControl.hh"

#include "GUI.hh"
#include "AppletWindow.hh"

RemoteControl   *RemoteControl::instance = NULL;
WorkraveControl *RemoteControl::workrave_control = NULL;

#include "nls.h"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#include "NetworkJoinDialog.hh"
#include "NetworkLogDialog.hh"
#endif

/************************************************************************/
/* GNOME::RemoteControl                                                 */
/************************************************************************/

RemoteControl::RemoteControl()
{
}


RemoteControl::~RemoteControl()
{
}


WR_METHOD_ARGS0_IMPL(void, fire)
{
  TRACE_ENTER("RemoteControl::fire");

  GUI *gui = GUI::get_instance();

  AppletWindow *applet = NULL;
  if (gui != NULL)
    {
      applet = gui->get_applet_window();
    }
  if (applet != NULL)
    {
      applet->fire();
    }
  TRACE_EXIT();
}

WR_METHOD_ARGS0_IMPL(CORBA_boolean, open_main)
{
  TRACE_ENTER("RemoteControl::open_main");
  TRACE_EXIT();
}


WR_METHOD_ARGS0_IMPL(CORBA_boolean, open_preferences)
{
}


WR_METHOD_ARGS0_IMPL(CORBA_boolean, open_network_connect)
{
}


WR_METHOD_ARGS0_IMPL(CORBA_boolean, open_network_log)
{
}

  
WR_METHOD_ARGS0_IMPL(CORBA_boolean, restbreak)
{
  GUI *gui = GUI::get_instance();

  if (gui != NULL)
    {
      gui->restbreak_now();
    }
}


WR_METHOD_ARGS1_IMPL(CORBA_boolean, set_mode, GNOME_Workrave_WorkraveControl_Mode, mode)
{
}


WR_METHOD_ARGS0_IMPL(CORBA_boolean, disconnect_all)
{
#ifdef HAVE_DISTRIBUTION
  DistributionManager *dist_manager = DistributionManager::get_instance();
  if (dist_manager != NULL)
    {
      dist_manager->disconnect_all();
    }
#endif
}


WR_METHOD_ARGS0_IMPL(CORBA_boolean, reconnect_all)
{
#ifdef HAVE_DISTRIBUTION
  DistributionManager *dist_manager = DistributionManager::get_instance();
  if (dist_manager != NULL)
    {
      dist_manager->reconnect_all();
    }
#endif
}


WR_METHOD_ARGS0_IMPL(CORBA_boolean, quit)
{
  GUI *gui = GUI::get_instance();

  if (gui != NULL)
    {
      gui->terminate();
    }
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

  WR_METHOD_REGISTER(open_main);
  WR_METHOD_REGISTER(open_preferences);
  WR_METHOD_REGISTER(open_network_connect);
  WR_METHOD_REGISTER(open_network_log);
  
  WR_METHOD_REGISTER(restbreak);
  WR_METHOD_REGISTER(set_mode);
  WR_METHOD_REGISTER(disconnect_all);
  WR_METHOD_REGISTER(reconnect_all);
  WR_METHOD_REGISTER(quit);
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
