// OSXGtkMenu.hh --- Menu using Gtk+
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

#ifndef OSXGTKMENU_HH
#define OSXGTKMENU_HH

#include "config.h"

#include <string>

#include "MainGtkMenu.hh"

#include "ige-mac-dock.h"


class OSXGtkMenu
  : public MainGtkMenu
{
public:
  OSXGtkMenu(bool show_open);
  virtual ~OSXGtkMenu();

  virtual void create_ui();
  virtual void popup(const guint button, const guint activate_time);
  
private:
  static void dock_clicked(IgeMacDock *dock, void *data);
  static void dock_quit(IgeMacDock *dock, void *data);
};

#endif // OSXGTKMENU_HH
