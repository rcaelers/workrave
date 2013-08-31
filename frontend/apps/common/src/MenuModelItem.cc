// Menus.cc
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

#include "nls.h"

#include "MenuModelItem.hh"

using namespace std;

MenuItem::Ptr
MenuItem::create()
{
  return Ptr(new MenuItem());
}


MenuItem::Ptr
MenuItem::create(const std::string &text, Activated activated, MenuItemType type)
{
  return Ptr(new MenuItem(text, activated, type));
}


MenuItem::MenuItem()
  : type(MenuItemType::MENU), checked(false)
{
}


MenuItem::MenuItem(const std::string &text, Activated activated, MenuItemType type)
  : text(text), activated(activated), type(type)
{
}


const MenuItem::MenuItemList
MenuItem::get_submenus() const
{
  return submenus;
}


const std::string &
MenuItem::get_text() const
{
  return text;
}


void
MenuItem::set_text(const std::string &text)
{
  if (this->text != text)
    {
      this->text = text;
      changed_signal();
    }
}


MenuItemType
MenuItem::get_type() const
{
  return type;
}


bool
MenuItem::is_checked() const
{
  return checked;
}

void
MenuItem::set_checked(bool checked)
{
  if (this->checked != checked)
    {
      this->checked = checked;
      changed_signal();
    }
}


void
MenuItem::add_menu(MenuItem::Ptr submenu)
{
  submenus.push_back(submenu);
  added_signal(submenu);
}


void
MenuItem::activate()
{
  activated();
}

boost::signals2::signal<void()> &
MenuItem::signal_changed()
{
  return changed_signal;
}


boost::signals2::signal<void(MenuItem::Ptr)> &
MenuItem::signal_added()
{
  return added_signal;
}

