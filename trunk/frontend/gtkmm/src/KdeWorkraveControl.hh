// KdeWorkraveControl.hh --- KDE Workrave Applet
//
// Copyright (C) 2004, 2005, 2007, 2008 Rob Caelers
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $Id$
//

#ifndef KDEWORKRAVECONTROL_H
#define KDEWORKRAVECONTROL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sigc++/trackable.h>
#include <glibmm.h>
#include "kde_applet/kworkravecontroliface.h"

class KWinModule;

class KdeWorkraveControl : public KWorkraveControlIface, public sigc::trackable
{
public:
  void fire();
  static void init();

public:
  KdeWorkraveControl();
  virtual ~KdeWorkraveControl();

private:
  bool io_handler(Glib::IOCondition ioc);
};

#endif
