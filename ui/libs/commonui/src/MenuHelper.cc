// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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
#  include "config.h"
#endif

#include "commonui/MenuHelper.hh"

#include "commonui/MenuDefs.hh"
#include "debug.hh"

namespace
{
  const std::map<std::string_view, MenuAction> legacyMapping = {
    {MenuId::PREFERENCES, MenuAction::Preferences},
    {MenuId::EXERCISES, MenuAction::Exercises},
    {MenuId::REST_BREAK, MenuAction::Restbreak},
    {MenuId::MODE_MENU, MenuAction::ModeMenu},
    {MenuId::MODE, MenuAction::Mode},
    {MenuId::MODE_NORMAL, MenuAction::ModeNormal},
    {MenuId::MODE_QUIET, MenuAction::ModeQuiet},
    {MenuId::MODE_SUSPENDED, MenuAction::ModeSuspended},
    {MenuId::STATISTICS, MenuAction::Statistics},
    {MenuId::ABOUT, MenuAction::About},
    {MenuId::MODE_READING, MenuAction::ModeReading},
    {MenuId::OPEN, MenuAction::Open},
    {MenuId::QUIT, MenuAction::Quit},
  };
}

MenuHelper::MenuHelper(MenuModel::Ptr menu_model)
  : menu_model(menu_model)
{
  TRACE_ENTRY();
  for (auto [id, command]: legacyMapping)
    {
      menu_id_mapping[std::string(id)] = static_cast<std::underlying_type_t<MenuAction>>(command);
    }
}

MenuHelper::~MenuHelper()
{
  TRACE_ENTRY();
}

uint32_t
MenuHelper::allocate_command(const std::string &id)
{
  uint32_t command = 1;
  auto i = menu_id_mapping.find(id);
  if (i != std::end(menu_id_mapping))
    {
      command = i->second;
    }
  else
    {
      auto i = std::max_element(std::begin(menu_id_mapping), std::end(menu_id_mapping), [](const auto &p1, const auto &p2) {
        return p1.second < p2.second;
      });

      if (i != std::end(menu_id_mapping))
        {
          command = i->second + 1;
        }
      menu_id_mapping[id] = command;
    }
  return command;
}

std::string
MenuHelper::lookup(uint32_t command)
{
  auto i = std::find_if(menu_id_mapping.begin(), menu_id_mapping.end(), [command](const auto &m) { return m.second == command; });
  if (i != menu_id_mapping.end())
    {
      return i->first;
    }
  return "";
}

menus::Node::Ptr
MenuHelper::find_node(const std::string &id)
{
  auto i = std::find_if(menu_entries.begin(), menu_entries.end(), [id](const auto &n) { return n->get_node()->get_id() == id; });
  if (i != menu_entries.end())
    {
      return (*i)->get_node();
    }
  return {};
}

menus::Node::Ptr
MenuHelper::find_node(uint32_t command)
{
  auto id = lookup(command);
  if (!id.empty())
    {
      return find_node(id);
    }
  return {};
}

void
MenuHelper::setup_event()
{
  workrave::utils::connect(menu_model->signal_update(), this, [this]() { update_model(); });

  update_model();
}

void
MenuHelper::update_model()
{
  menu_entries.clear();
  handle_node(menu_model->get_root());
}

void
MenuHelper::handle_node(menus::Node::Ptr node)
{
  menu_entries.push_back(std::make_shared<detail::MenuHelperEntry>(node, update_signal));
  if (auto n = std::dynamic_pointer_cast<menus::ContainerNode>(node); n)
    {
      for (auto &sub_node: n->get_children())
        {
          handle_node(sub_node);
        }
    }

  else if (auto n = std::dynamic_pointer_cast<menus::RadioGroupNode>(node); n)
    {
      for (auto &sub_node: n->get_children())
        {
          handle_node(sub_node);
        }
    }
}

boost::signals2::signal<void(menus::Node::Ptr)> &
MenuHelper::signal_update()
{
  return update_signal;
}

namespace detail
{
  MenuHelperEntry::MenuHelperEntry(menus::Node::Ptr node, boost::signals2::signal<void(menus::Node::Ptr)> &update_signal)
    : node(node)
    , update_signal(update_signal)
  {
    workrave::utils::connect(node->signal_changed(), this, [this]() { this->update_signal(this->node); });
  }
  menus::Node::Ptr MenuHelperEntry::get_node() const
  {
    return node;
  }

} // namespace detail
