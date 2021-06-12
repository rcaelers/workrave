// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef TOOLKITMENU_HH
#define TOOLKITMENU_HH

#include "MenuModel.hh"

#include <boost/signals2.hpp>
#include <memory>
#include <string>

#include <QMenu>

#include "utils/Signals.hh"

typedef std::function<bool(MenuNode::Ptr)> MenuNodeFilter;

class ContainerMenuEntry;
class SubMenuEntry;

class MenuEntry
  : public QObject
  , public workrave::utils::Trackable
{
  Q_OBJECT

public:
  typedef std::shared_ptr<MenuEntry> Ptr;

  MenuEntry(MenuNode::Ptr menu_node, MenuNodeFilter filter);

  virtual QAction *get_action() const = 0;

  MenuNode::Ptr get_menu_node() const;
  MenuNodeFilter get_filter() const;

private:
  MenuNode::Ptr menu_node;
  MenuNodeFilter filter;
};

class MenuEntryFactory
{
public:
  static MenuEntry::Ptr create(MenuNode::Ptr menu_node, MenuNodeFilter filter);
};

class SubMenuEntry : public MenuEntry
{
  Q_OBJECT

public:
  typedef std::shared_ptr<SubMenuEntry> Ptr;
  typedef std::list<MenuEntry::Ptr> MenuEntries;

  SubMenuEntry(MenuNode::Ptr menu_node, MenuNodeFilter filter);

  QMenu *get_menu() const;
  QAction *get_action() const override;
  void init();

private:
  QMenu *menu{nullptr};
  MenuEntries children;
};

class ActionMenuEntry : public MenuEntry
{
  Q_OBJECT

public:
  ActionMenuEntry(MenuNode::Ptr menu_node, MenuNodeFilter filter);

  QAction *get_action() const override;

public Q_SLOTS:
  void on_action(bool checked);

private:
  void on_menu_changed();

private:
  QAction *action{nullptr};
};

class SeperatorMenuEntry : public MenuEntry
{
  Q_OBJECT

public:
  SeperatorMenuEntry(MenuNode::Ptr menu_node, MenuNodeFilter filter);

  QAction *get_action() const override;

private:
  QAction *action{nullptr};
};

class ToolkitMenu
  : public QObject
  , public workrave::utils::Trackable
{
  Q_OBJECT

public:
  typedef std::shared_ptr<ToolkitMenu> Ptr;

  ToolkitMenu(MenuModel::Ptr menu_node, MenuNodeFilter filter = 0);

  QMenu *get_menu() const;

private:
  void on_update();

private:
  SubMenuEntry::Ptr menu;
};

#endif // TOOLKITMENU_HH
