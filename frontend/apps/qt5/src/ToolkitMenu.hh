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

#ifndef MENUHANDLER_HH
#define MENUHANDLER_HH

#include "MenuModel.hh"

#include <string>

#include "utils/ScopedConnections.hh"

#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <QMenu>

typedef boost::function<bool (MenuModel::Ptr)> MenuModelFilter;

namespace detail
{
  class MenuEntry: public QObject
  {
    Q_OBJECT

  public:
    typedef boost::shared_ptr<MenuEntry> Ptr;
    typedef std::list<Ptr> MenuEntries;

    static Ptr create(MenuModel::Ptr menu_model, MenuModelFilter filter);

    MenuEntry(MenuModel::Ptr menu_model, MenuModelFilter filter);
    virtual ~MenuEntry() {};
    virtual QAction* get_action() const = 0;
    virtual MenuModel::Ptr get_menu_model() const;

  protected:
    MenuModel::Ptr menu_model;
    MenuModelFilter filter;
  };

  class SubMenuEntry: public MenuEntry
  {
    Q_OBJECT

  public:
    typedef boost::shared_ptr<SubMenuEntry> Ptr;

    SubMenuEntry(MenuModel::Ptr menu_model, MenuModelFilter filter);
    virtual ~SubMenuEntry();

    QMenu *get_menu() const;
    virtual QAction* get_action() const;

  private:
    void on_menu_added(MenuModel::Ptr added, MenuModel::Ptr before);
    void on_menu_removed(MenuModel::Ptr removed);
    void on_menu_changed();

    void add_menu(MenuModel::Ptr menu_to_add, MenuModel::Ptr before);

  private:
    MenuEntries children;
    QMenu *menu;
    scoped_connections connections;
  };

  class ActionMenuEntry : public MenuEntry
  {
    Q_OBJECT

  public:
    typedef boost::shared_ptr<ActionMenuEntry> Ptr;

    ActionMenuEntry(MenuModel::Ptr menu_model, MenuModelFilter filter);
    virtual ~ActionMenuEntry();

    virtual QAction* get_action() const;

  public slots:
    void on_action(bool checked);

  private:
    void on_menu_changed();

  private:
    QAction *action;
    scoped_connections connections;
  };
}

class ToolkitMenu : public QObject
{
  Q_OBJECT

public:
  typedef boost::shared_ptr<ToolkitMenu> Ptr;

  static Ptr create(MenuModel::Ptr top, MenuModelFilter filter = 0);

  ToolkitMenu(MenuModel::Ptr top, MenuModelFilter filter = 0);
  virtual ~ToolkitMenu();

  QMenu *get_menu() const;

private:
  detail::SubMenuEntry::Ptr menu;
};


#endif // MENUHANDLER_HH
