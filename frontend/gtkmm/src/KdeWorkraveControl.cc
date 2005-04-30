// kworkraveapplet.cc --- Workrave applet for KDE
//
// Copyright (C) 2004, 2005 Rob Caelers
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

#include "preinclude.h"

#include <stdio.h>

#include <sigc++/object.h>

#include "GUI.hh"
#include "AppletWindow.hh"

#include "KdeWorkraveControl.hh"
#include "kde_applet/kworkravecontroliface.h"

#include <qcursor.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <krun.h>
#include <kglobal.h>
#include <klocale.h>
#include <kwinmodule.h>
#include <kconfig.h>
#include <kdebug.h>
#include <qsocketnotifier.h>
#include <qeventloop.h>
#include <qptrdict.h>
#include <dcopobject.h>
#include <kcmodule.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <gdk/gdk.h>


#include "debug.hh"

void
KdeWorkraveControl::init()
{
  TRACE_ENTER("KdeWorkraveControl::init");

  DCOPClient *client = kapp->dcopClient();
  //if (!client->isRegistered())
  {
    client->attach();
    client->registerAs("kworkravecontrol", false);
  }
    
  KGlobal::locale()->insertCatalogue("KdeWorkraveControl");
  new KdeWorkraveControl();

  TRACE_EXIT();
}


KdeWorkraveControl::KdeWorkraveControl()
  : DCOPObject("KWorkraveControl")
{
  DCOPClient *client = kapp->dcopClient();

  Glib::RefPtr<Glib::MainContext> context =
    Glib::MainContext::get_default();
  const Glib::RefPtr<Glib::IOChannel> channel =
    Glib::IOChannel::create_from_fd(client->socket());
  const Glib::RefPtr<Glib::IOSource> io_source =
    Glib::IOSource::create(channel, Glib::IO_IN | Glib::IO_HUP);
  
  io_source->connect(sigc::mem_fun(*this, &KdeWorkraveControl::io_handler));
  io_source->attach(context);    
}


KdeWorkraveControl::~KdeWorkraveControl()
{
  KGlobal::locale()->removeCatalogue("kworkravecontrol");
}


void
KdeWorkraveControl::fire()
{
  TRACE_ENTER("KdeWorkraveControl::fire");
  GUI *gui = GUI::get_instance();

  AppletWindow *applet = NULL;
  if (gui != NULL)
    {
      applet = gui->get_applet_window();
    }
  if (applet != NULL)
    {
      applet->fire_kde_applet();
    }
  TRACE_EXIT();
}

bool
KdeWorkraveControl::io_handler(Glib::IOCondition ioc)
{
  (void) ioc;
  kapp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
  return true;
}
  
