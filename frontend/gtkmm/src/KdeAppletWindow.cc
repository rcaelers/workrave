// KdeAppletWindow.cc --- Applet info Window
//
// Copyright (C) 2001, 2002, 2003, 2004 Rob Caelers & Raymond Penners
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "KdeAppletWindow.hh"

#ifdef HAVE_KDE
#include <dcopclient.h>
#include <kapp.h>
#include <kde_applet/kworkraveapplet/kworkraveapplet_stub.h>
#endif


bool
KdeAppletWindow::plug_window(int w)
{
  TRACE_ENTER("KdeAppletWindow::plug_window");
  KWorkraveApplet_stub dcop("kworkrave", "KWorkrave"); 
  dcop.embed_window(w);
  TRACE_MSG(dcop.ok());
  TRACE_EXIT();

  return dcop.ok();
}


bool
KdeAppletWindow::get_size(int &size)
{
  TRACE_ENTER("KdeAppletWindow::get_size");
  KWorkraveApplet_stub dcop("kworkrave", "KWorkrave"); 
  size = dcop.get_size();
  TRACE_MSG(dcop.ok());
  TRACE_EXIT();

  return dcop.ok();
}

bool
KdeAppletWindow::get_vertical(bool &vertical)
{
  TRACE_ENTER("KdeAppletWindow::get_vertical");
  KWorkraveApplet_stub dcop("kworkrave", "KWorkrave"); 
  vertical = dcop.get_vertical();
  TRACE_MSG(dcop.ok());
  TRACE_EXIT();

  return dcop.ok();
}

bool
KdeAppletWindow::set_size(int width, int height)
{
  TRACE_ENTER_MSG("KdeAppletWindow::set_size", width << " " << height);
  KWorkraveApplet_stub dcop("kworkrave", "KWorkrave"); 
  dcop.set_size(width, height);
  TRACE_MSG(dcop.ok());
  TRACE_EXIT();

  return dcop.ok();
}

