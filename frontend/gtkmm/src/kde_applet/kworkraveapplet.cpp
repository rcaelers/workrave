// kworkraveapplet.cc --- Workrave applet for KDE
//
// Copyright (C) 2004 Rob Caelers
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

#include <stdio.h>

#include <qcursor.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <krun.h>
#include <kglobal.h>
#include <kapplication.h>
#include <klocale.h>
#include <kwinmodule.h>
#include <kconfig.h>
#include <kdebug.h>

#include "kworkraveapplet.h"

#include <X11/Xlib.h>

# include <X11/X.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/Xatom.h>
extern "C"
{
  KPanelApplet* init(QWidget *parent, const QString configFile)
  {
    DCOPClient *client = kapp->dcopClient();
    //if (!client->isRegistered())
    {
      client->attach();
      client->registerAs("kworkrave", false);
    }
    
    KGlobal::locale()->insertCatalogue("KWorkraveApplet");
    return new KWorkraveApplet(configFile,
                               KPanelApplet::Stretch,
                               KPanelApplet::About,
                               parent, "KWorkraveApplet");
  }
}


KWorkraveApplet::KWorkraveApplet(const QString& configFile, Type type, int actions,
                                 QWidget *parent, const char *name)
  : KPanelApplet(configFile, type, actions, parent, name),
    DCOPObject("KWorkrave"),
    embed(NULL)
{
}

KWorkraveApplet::~KWorkraveApplet()
{
  KGlobal::locale()->removeCatalogue("kworkraveapplet");
}


void
KWorkraveApplet::embed_window(int window_id)
{
  if (embed != NULL)
    {
      delete embed;
    }
  embed = new QXEmbed(this);
  embed->embed(window_id);
  embed->show();

  connect(embed, SIGNAL(embeddedWindowDestroyed()), SLOT(embedded_window_destroyed()));
}


void
KWorkraveApplet::embedded_window_destroyed()
{
  embed->hide();
  delete embed;
  embed = NULL;
}

long
KWorkraveApplet::get_size()
{
  Orientation o = orientation();
  
  if (o == Qt::Vertical)
    {
      return size().width();
    }
  else
    {
      return size().height();
    }
}


bool
KWorkraveApplet::get_vertical()
{
  Orientation o = orientation();
  
  return o == Qt::Vertical;
}


void
KWorkraveApplet::set_size(int width, int height)
{
  if (embed != NULL)
    {
      Orientation o = orientation();

      embed->resize(width, height);
      if (o == Qt::Vertical)
        {
          embed->move((size().width() - width) / 2, 0);
        }
      else
        {
          embed->move(0, (size().height() - height) / 2);
        }
    }
}


// bool KWorkraveApplet::eventFilter( QObject *o, QEvent * e) {
// 	// when clicking on the combo box, it should gain focus
// 	if ( e->type() == QEvent::MouseButtonRelease ) {
// 		emit requestFocus();
// 	}
// 	return KPanelApplet::eventFilter( o, e );
// }

// void KWorkraveApplet::mousePressEvent(QMouseEvent *e) {
// 	if (e->button() == Qt::LeftButton) {
// 		emit requestFocus();
// 	}
// }
