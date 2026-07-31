// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
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

#ifndef GIOMENU_HH
#define GIOMENU_HH

#include <list>
#include <memory>
#include <string>

#include <gio/gio.h>

#include "commonui/MenuModel.hh"
#include "utils/Signals.hh"

class GioMenu : public workrave::utils::Trackable
{
public:
  explicit GioMenu(MenuModel::Ptr menu_model);
  ~GioMenu();

  GioMenu(const GioMenu &) = delete;
  GioMenu &operator=(const GioMenu &) = delete;
  GioMenu(GioMenu &&) = delete;
  GioMenu &operator=(GioMenu &&) = delete;

  [[nodiscard]] GMenu *get_menu() const;
  [[nodiscard]] GSimpleActionGroup *get_actions() const;

  void update();

private:
  struct Entry
  {
    ~Entry();

    std::string name;
    GSimpleAction *action{nullptr};
    menus::Node::Ptr node;
  };
  using EntryPtr = std::shared_ptr<Entry>;

  void add_children(GMenu *menu, GMenu **section, const menus::ContainerNode::Ptr &parent);
  void add_node(GMenu *menu, GMenu **section, const menus::Node::Ptr &node);
  void add_submenu(GMenu *section, const menus::SubMenuNode::Ptr &node);
  void add_action(GMenu *section, const menus::ActionNode::Ptr &node);
  void add_toggle(GMenu *section, const menus::ToggleNode::Ptr &node);
  void add_radio_group(GMenu *section, const menus::RadioGroupNode::Ptr &node);
  void add_radio(GMenu *section, const menus::RadioNode::Ptr &node);

  EntryPtr add_entry(const std::string &name, GSimpleAction *action, menus::Node::Ptr node);
  static GMenu *append_new_section(GMenu *menu);
  static std::string detailed_action(const std::string &name);
  static constexpr const char *ACTION_PREFIX = "indicator";

  MenuModel::Ptr menu_model;
  GMenu *menu{nullptr};
  GSimpleActionGroup *actions{nullptr};
  std::list<EntryPtr> entries;
};

#endif // GIOMENU_HH
