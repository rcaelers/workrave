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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/make_shared.hpp>

#include "MenuHandler.hh"

using namespace std;


MenuHandler::Ptr
MenuHandler::create(MenuItem::Ptr top)
{
  return Ptr(new MenuHandler(top));
}


MenuHandler::MenuHandler(MenuItem::Ptr top)
{
  qmenuitem = boost::make_shared<detail::Menu>(top, nullptr, nullptr);
}

MenuHandler::~MenuHandler()
{
}

void
MenuHandler::popup(int x, int y)
{
  qmenuitem->popup(x, y);
}


namespace detail
{

MenuEntry::Ptr
MenuEntry::create(MenuItem::Ptr menuitem, QMenu *parent, QAction *before)
{
  MenuItemType type = menuitem->get_type();

  if (type == MenuItemType::MENU)
    {
      return Ptr(new Menu(menuitem, parent, before));
    }
  else
    {
      return Ptr(new Action(menuitem, parent, before));
    }
}
  
MenuEntry::MenuEntry(MenuItem::Ptr menuitem, QMenu *parent) : menuitem(menuitem), parent(parent)
{
}


MenuItem::Ptr 
MenuEntry::get_menuitem() const
{
  return menuitem;
}


Menu::Menu(MenuItem::Ptr menuitem, QMenu *parent, QAction *before)
  : MenuEntry(menuitem, parent), menu(NULL)
{
  menuitem->signal_added().connect(boost::bind(&Menu::on_menu_added, this, _1, _2)); 
  menuitem->signal_removed().connect(boost::bind(&Menu::on_menu_removed, this, _1)); 
  menuitem->signal_changed().connect(boost::bind(&Menu::on_menu_changed, this)); 

  const char *text = menuitem->get_text().c_str();
  
  menu = new QMenu(text);

  for (const MenuItem::Ptr &item : menuitem->get_submenus())
    {
      MenuEntry::Ptr child = MenuEntry::create(item, menu);
      children.push_back(child);
    }

  if (parent != NULL)
    {
      parent->insertMenu(before, menu);
    }
}

Menu::~Menu()
{
}

QAction *
Menu::get_action() const
{
  return menu->menuAction();
}
  
void
Menu::on_menu_added(MenuItem::Ptr added, MenuItem::Ptr before)
{
  QAction *qbefore = nullptr;
  MenuEntries::iterator pos = children.end();
  if (before)
    {
      pos = std::find_if(children.begin(), children.end(), [&] (MenuEntry::Ptr i) { return i->get_menuitem() == before; });
      if (pos != children.end())
        {
          qbefore = (*pos)->get_action();
        }
    }

  MenuEntry::Ptr child = MenuEntry::create(added, menu, qbefore);
  children.insert(pos, child);
}
  

void
Menu::on_menu_removed(MenuItem::Ptr removed)
{
  MenuEntries::iterator pos = std::find_if(children.begin(), children.end(), [&] (MenuEntry::Ptr i) { return i->get_menuitem() == removed; });
  if (pos != children.end())
    {
      children.erase(pos);
    }
}
  

void
Menu::on_menu_changed()
{
  const char *text = menuitem->get_text().c_str();
  menu->setTitle(text);
}


void
Menu::popup(int x, int y)
{
  menu->popup(QPoint(x, y));
}
  

Action::Action(MenuItem::Ptr menuitem, QMenu *parent, QAction *before)
  : MenuEntry(menuitem, parent), action(NULL)
{
  menuitem->signal_changed().connect(boost::bind(&Action::on_menu_changed, this)); 

  MenuItemType type = menuitem->get_type();
  const char *text = menuitem->get_text().c_str();
  bool checked = menuitem->is_checked();
  
  action = new QAction(text, this);

  action->setCheckable(type == MenuItemType::RADIO || type == MenuItemType::CHECK);
  action->setChecked(checked);
      
  //action->setShortcuts(QKeySequence::New);

  //connect(action, SIGNAL(triggered()), this, SLOT(on_action()));

  connect(action, &QAction::triggered, this, &Action::on_action);
  
  parent->insertAction(before, action);
}


Action::~Action()
{
}


QAction *
Action::get_action() const
{
  return action;
}
  
void
Action::on_menu_changed()
{
  MenuItemType type = menuitem->get_type();
  const char *text = menuitem->get_text().c_str();
  bool checked = menuitem->is_checked();

  if (action != NULL)
    {
      action->setCheckable(type == MenuItemType::RADIO || type == MenuItemType::RADIO);
      action->setChecked(checked);
      action->setText(text);
    }
}

void
Action::on_action(bool checked)
{
  menuitem->activate();
}
  
}
