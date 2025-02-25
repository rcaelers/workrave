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

#ifndef MENUHELPER_HH
#define MENUHELPER_HH

#include <string>

#include "commonui/MenuModel.hh"
#include "utils/Signals.hh"

namespace detail
{
  class MenuHelperEntry : public workrave::utils::Trackable
  {
  public:
    using Ptr = std::shared_ptr<MenuHelperEntry>;
    MenuHelperEntry(menus::Node::Ptr node, boost::signals2::signal<void(menus::Node::Ptr)> &update_signal);

    menus::Node::Ptr get_node() const;

  private:
    menus::Node::Ptr node;
    boost::signals2::signal<void(menus::Node::Ptr node)> &update_signal;
  };
} // namespace detail

class MenuHelper : public workrave::utils::Trackable
{
public:
  explicit MenuHelper(MenuModel::Ptr menu_model);
  ~MenuHelper();

public:
  uint32_t allocate_command(const std::string &id);
  std::string lookup(uint32_t command);

  menus::Node::Ptr find_node(const std::string &id);
  menus::Node::Ptr find_node(uint32_t command);

  void setup_event();
  boost::signals2::signal<void(menus::Node::Ptr)> &signal_update();

private:
  void update_model();
  void handle_node(menus::Node::Ptr node);

private:
  MenuModel::Ptr menu_model;
  std::map<std::string, uint32_t> menu_id_mapping;
  std::list<detail::MenuHelperEntry::Ptr> menu_entries;
  boost::signals2::signal<void(menus::Node::Ptr)> update_signal;
};

#endif // GENERICDBUSAPPLET_HH
