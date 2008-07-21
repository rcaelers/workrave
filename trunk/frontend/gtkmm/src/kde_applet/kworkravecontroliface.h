// KdeWorkraveControl.hh --- KDE Workrave Applet
//
// Copyright (C) 2004, 2005, 2007 Rob Caelers
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

#ifndef KWORKRAVECONTROLIFACE_H
#define KWORKRAVECONTROLIFACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dcopobject.h>
#include <qxembed.h>
#include <kcmodule.h>
#include <kconfig.h>

class KWinModule;

class KWorkraveControlIface : virtual public DCOPObject
{
  K_DCOP

k_dcop:
  virtual void fire() = 0;
};

#endif
