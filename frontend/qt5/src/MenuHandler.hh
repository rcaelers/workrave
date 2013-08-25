// MenuHandler.cc
//
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

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Menus.hh"

namespace detail
{
  class MenuEntry: public QObject
  {
    Q_OBJECT

  public:
    typedef boost::shared_ptr<MenuEntry> Ptr;
    typedef std::list<Ptr> MenuEntries;
  
    static Ptr create(MenuItem::Ptr menuitem);

    virtual ~MenuEntry() {};
    virtual void init(QMenu *parent, QAction *before = NULL) = 0;
  };

  
  class Menu: public MenuEntry
  {
    Q_OBJECT

  public:
    typedef boost::shared_ptr<Menu> Ptr;
  
    Menu(MenuItem::Ptr menuitem);
    virtual ~Menu();

    void popup(int x, int y);

    virtual void init(QMenu *parent, QAction *before = NULL);

  private:
    void on_menu_added(MenuItem::Ptr added);
    void on_menu_changed();
  
  private:
    MenuItem::Ptr menuitem;
    MenuEntries children;
    QMenu *menu;
  };


  class Action : public MenuEntry
  {
    Q_OBJECT

  public:
    typedef boost::shared_ptr<Action> Ptr;

    Action(MenuItem::Ptr menuitem);
    virtual ~Action();

    virtual void init(QMenu *parent, QAction *before = NULL);

  public slots:
    void on_action(bool checked);
    
  private:
    void on_menu_changed();
  
  private:
    MenuItem::Ptr menuitem;
    QAction *action;
  };
}

class MenuHandler : public QObject
{
  Q_OBJECT

public:
  typedef boost::shared_ptr<MenuHandler> Ptr;

  static Ptr create(MenuItem::Ptr top);

  MenuHandler(MenuItem::Ptr top);
  virtual ~MenuHandler();

  virtual void popup(int x, int y);
  
private:
  detail::Menu::Ptr qmenuitem;
};


#endif // MENUHANDLER_HH
