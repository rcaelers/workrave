// MacOSGtkMenu.hh --- Menu using Gtk+
//
// Copyright (C) 2001 - 2008, 2013 Rob Caelers & Raymond Penners
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

#ifndef MACOSGTKMENU_HH
#define MACOSGTKMENU_HH

#include "config.h"

#include <string>

#include "MainGtkMenu.hh"

#ifdef HAVE_IGE_MAC_INTEGRATION
#  include "ige-mac-dock.h"
#endif

#ifdef HAVE_GTK_MAC_INTEGRATION
#  include "gtk-mac-dock.h"
#  define IgeMacDock GtkMacDock
#endif

class MacOSGtkMenu : public MainGtkMenu
{
public:
  MacOSGtkMenu(bool show_open);
  virtual ~MacOSGtkMenu();

  virtual void create_ui();
  virtual void popup(const guint button, const guint activate_time);

private:
  static void dock_clicked(IgeMacDock *dock, void *data);
  static void dock_quit(IgeMacDock *dock, void *data);
};

#endif // MACOSGTKMENU_HH
