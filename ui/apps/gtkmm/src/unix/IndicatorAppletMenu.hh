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

#ifndef INDICATORAPPLETMENU_HH
#define INDICATORAPPLETMENU_HH

#include <string>
#include <memory>

#include <iostream>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "MenuModel.hh"

#include "utils/Signals.hh"

namespace detail
{
  class IndicatorSubMenuEntry;

  class IndicatorMenuEntry : public workrave::utils::Trackable
  {

  public:
    using Ptr = std::shared_ptr<IndicatorMenuEntry>;

    IndicatorMenuEntry();
    ~IndicatorMenuEntry();

    DbusmenuMenuitem *get_item() const;

  protected:
    DbusmenuMenuitem *item{nullptr};
    std::list<IndicatorMenuEntry::Ptr> children;
  };

  class IndicatorSubMenuEntry : public IndicatorMenuEntry
  {
  public:
    using Ptr = std::shared_ptr<IndicatorSubMenuEntry>;

    IndicatorSubMenuEntry(IndicatorSubMenuEntry *parent, menus::SubMenuNode::Ptr node);
    IndicatorSubMenuEntry(DbusmenuServer *server, menus::SubMenuNode::Ptr node);

    void init();
    void add(DbusmenuMenuitem *item);
    void add_section();

  private:
    DbusmenuServer *server{nullptr};
    IndicatorSubMenuEntry *parent{nullptr};
    menus::SubMenuNode::Ptr node;
  };

  class IndicatorActionMenuEntry : public IndicatorMenuEntry
  {
  public:
    IndicatorActionMenuEntry(IndicatorSubMenuEntry *parent, menus::ActionNode::Ptr node);

  private:
    static void static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data);
    void menu_item_activated(DbusmenuMenuitem *mi);

  private:
    menus::ActionNode::Ptr node;
  };

  class IndicatorToggleMenuEntry : public IndicatorMenuEntry
  {
  public:
    IndicatorToggleMenuEntry(IndicatorSubMenuEntry *parent, menus::ToggleNode::Ptr node);

  private:
    static void static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data);
    void menu_item_activated(DbusmenuMenuitem *mi);

  private:
    menus::ToggleNode::Ptr node;
  };

  class IndicatorRadioMenuEntry : public IndicatorMenuEntry
  {
  public:
    IndicatorRadioMenuEntry(IndicatorSubMenuEntry *parent, menus::RadioNode::Ptr node);

  private:
    static void static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data);
    void menu_item_activated(DbusmenuMenuitem *mi);

  private:
    menus::RadioNode::Ptr node;
  };

  class IndicatorRadioGroupMenuEntry : public IndicatorMenuEntry
  {
  public:
    IndicatorRadioGroupMenuEntry(IndicatorSubMenuEntry *parent, menus::RadioGroupNode::Ptr node);

  private:
    std::list<IndicatorRadioMenuEntry::Ptr> children;
  };

  class IndicatorSeparatorMenuEntry : public IndicatorMenuEntry
  {
  public:
    IndicatorSeparatorMenuEntry(IndicatorSubMenuEntry *parent, menus::SeparatorNode::Ptr node);
  };

  class IndicatorMenuEntryFactory
  {
  public:
    static IndicatorMenuEntry::Ptr create(IndicatorSubMenuEntry *parent, menus::Node::Ptr node);
  };

} // namespace detail

class IndicatorAppletMenu : public workrave::utils::Trackable
{
public:
  using Ptr = std::shared_ptr<IndicatorAppletMenu>;

  IndicatorAppletMenu(MenuModel::Ptr menu_node);
  ~IndicatorAppletMenu() = default;

private:
  DbusmenuServer *server{nullptr};
  MenuModel::Ptr menu_model;
  detail::IndicatorSubMenuEntry::Ptr entry;
};

#endif // INDICATORAPPLETMENU_HH
