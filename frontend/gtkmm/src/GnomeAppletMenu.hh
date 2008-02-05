// GnomeAppletMenu.hh --- Menu using GnomeApplet+
//
// Copyright (C) 2001 - 2008 Rob Caelers & Raymond Penners
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
// $Id: GnomeAppletMenu.hh 1436 2008-02-03 18:03:23Z rcaelers $
//

#ifndef GNOMEAPPLETMENU_HH
#define GNOMEAPPLETMENU_HH

#include "config.h"

#include <string>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/iconfactory.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/uimanager.h>

#include "Menu.hh"

class GnomeAppletWindow;

class GnomeAppletMenu : public Menu
{
public:
  GnomeAppletMenu(GnomeAppletWindow *applet_window);
  virtual ~GnomeAppletMenu();

  virtual void init() = 0;
  virtual void popup(const guint button, const guint activate_time);
  virtual void resync(workrave::OperationMode mode, bool show_log);
  
private:
  GnomeAppletWindow *applet_window;  
};

#endif // GNOMEAPPLETMENU_HH
