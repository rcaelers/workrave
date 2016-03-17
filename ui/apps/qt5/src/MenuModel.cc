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

#include "MenuModel.hh"

MenuModel::MenuModel()
  : id("workrave:root"), type(MenuModelType::MENU), checked(false)
{
}

MenuModel::MenuModel(const std::string &id, const std::string &text, Activated activated, MenuModelType type)
  : id(id), text(text), activated(activated), type(type)
{
}

const MenuModel::MenuModelList
MenuModel::get_submenus() const
{
  return submenus;
}

const std::string &
MenuModel::get_text() const
{
  return text;
}

void
MenuModel::set_text(const std::string &text)
{
  if (this->text != text)
    {
      this->text = text;
      changed_signal();
    }
}

std::string
MenuModel::get_id() const
{
  return id;
}

MenuModelType
MenuModel::get_type() const
{
  return type;
}

bool
MenuModel::is_checked() const
{
  return checked;
}

void
MenuModel::set_checked(bool checked)
{
  if (this->checked != checked)
    {
      this->checked = checked;
      changed_signal();
    }
}

void
MenuModel::add_menu(MenuModel::Ptr submenu, MenuModel::Ptr before)
{
  MenuModelList::iterator pos = submenus.end();
  if (before)
    {
      pos = std::find(submenus.begin(), submenus.end(), before);
    }

  submenus.insert(pos, submenu);
  added_signal(submenu, before);
}

void
MenuModel::remove_menu(MenuModel::Ptr submenu)
{
  submenus.remove(submenu);
  removed_signal(submenu);
}

void
MenuModel::activate()
{
  activated();
}

boost::signals2::signal<void()> &
MenuModel::signal_changed()
{
  return changed_signal;
}

boost::signals2::signal<void(MenuModel::Ptr, MenuModel::Ptr)> &
MenuModel::signal_added()
{
  return added_signal;
}

boost::signals2::signal<void(MenuModel::Ptr)> &
MenuModel::signal_removed()
{
  return removed_signal;
}
