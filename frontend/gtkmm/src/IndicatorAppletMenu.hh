// IndicatorAppletMenu.hh --- Menu using IndicatorApplet+
//
// Copyright (C) 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef INDICATORAPPLETMENU_HH
#define INDICATORAPPLETMENU_HH

#include <string>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "Menus.hh"
#include "MenuBase.hh"
#include "MenuCommand.hh"

class GenericDBusApplet;

class IndicatorAppletMenu : public MenuBase
{
public:
  IndicatorAppletMenu();
  virtual ~IndicatorAppletMenu();

  virtual void init();
  virtual void resync(workrave::OperationMode mode, workrave::UsageMode usage, bool show_log);

private:
  enum MenuItemType { Radio, Check, Normal };

  DbusmenuMenuitem *menu_item_append(DbusmenuMenuitem *parent, const char *label);
  DbusmenuMenuitem *menu_item_append(DbusmenuMenuitem *parent, const char *label, int cmd);
  DbusmenuMenuitem *menu_item_append(DbusmenuMenuitem *parent, const char *label, MenuItemType type, int cmd);
  void menu_item_set_checked(int cmd, bool checked);

  int find_menu_item(DbusmenuMenuitem *item) const;

  static void static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data);
  void menu_item_activated(DbusmenuMenuitem *mi);

private:
  DbusmenuServer *server;
  DbusmenuMenuitem *root;
  DbusmenuMenuitem *menu_items[MENU_COMMAND_SIZEOF];
};

#endif // INDICATORAPPLETMENU_HH
