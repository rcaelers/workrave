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

#ifndef DBUSMENU_HH
#define DBUSMENU_HH

#include <string>
#include <memory>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "commonui/MenuModel.hh"
#include "ui/Plugin.hh"

#include "utils/Signals.hh"

namespace detail
{
  class DbusSubMenuEntry;

  class DbusMenuEntry : public workrave::utils::Trackable
  {
  public:
    using Ptr = std::shared_ptr<DbusMenuEntry>;

    DbusMenuEntry() = default;
    ~DbusMenuEntry();

    DbusmenuMenuitem *get_item() const;

  protected:
    DbusmenuMenuitem *item{nullptr};
    std::list<DbusMenuEntry::Ptr> children;
  };

  class DbusSubMenuEntry : public DbusMenuEntry
  {
  public:
    using Ptr = std::shared_ptr<DbusSubMenuEntry>;

    DbusSubMenuEntry(DbusSubMenuEntry *parent, menus::SubMenuNode::Ptr node);
    DbusSubMenuEntry(DbusmenuServer *server, menus::SubMenuNode::Ptr node);

    void init();
    void add(DbusmenuMenuitem *child);
    void add_section();

  private:
    DbusmenuServer *server{nullptr};
    DbusSubMenuEntry *parent{nullptr};
    menus::SubMenuNode::Ptr node;
  };

  class DbusActionMenuEntry : public DbusMenuEntry
  {
  public:
    DbusActionMenuEntry(DbusSubMenuEntry *parent, menus::ActionNode::Ptr node);

  private:
    static void static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data);
    void menu_item_activated(DbusmenuMenuitem *mi);

  private:
    menus::ActionNode::Ptr node;
  };

  class DbusToggleMenuEntry : public DbusMenuEntry
  {
  public:
    DbusToggleMenuEntry(DbusSubMenuEntry *parent, menus::ToggleNode::Ptr node);

  private:
    static void static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data);
    void menu_item_activated(DbusmenuMenuitem *mi);

  private:
    menus::ToggleNode::Ptr node;
  };

  class DbusRadioMenuEntry : public DbusMenuEntry
  {
  public:
    DbusRadioMenuEntry(DbusSubMenuEntry *parent, menus::RadioNode::Ptr node);

  private:
    static void static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data);
    void menu_item_activated(DbusmenuMenuitem *mi);

  private:
    menus::RadioNode::Ptr node;
  };

  class DbusRadioGroupMenuEntry : public DbusMenuEntry
  {
  public:
    DbusRadioGroupMenuEntry(DbusSubMenuEntry *parent, menus::RadioGroupNode::Ptr node);

  private:
    std::list<DbusRadioMenuEntry::Ptr> children;
  };

  class DbusSectionMenuEntry : public DbusMenuEntry
  {
  public:
    DbusSectionMenuEntry(DbusSubMenuEntry *parent, menus::SectionNode::Ptr node);

  private:
    std::list<DbusMenuEntry::Ptr> children;
  };

  class DbusSeparatorMenuEntry : public DbusMenuEntry
  {
  public:
    DbusSeparatorMenuEntry(DbusSubMenuEntry *parent, menus::SeparatorNode::Ptr node);
  };

  class DbusMenuEntryFactory
  {
  public:
    static DbusMenuEntry::Ptr create(DbusSubMenuEntry *parent, menus::Node::Ptr node);
  };

} // namespace detail

class DbusMenu
  : public Plugin<DbusMenu>
  , public workrave::utils::Trackable
{
public:
  using Ptr = std::shared_ptr<DbusMenu>;

  explicit DbusMenu(std::shared_ptr<IPluginContext> context);
  ~DbusMenu() override = default;

  std::string get_plugin_id() const override
  {
    return "workrave.DbusMenu";
  }

  auto get_server() const
  {
    return server;
  }

  auto get_root_menu_item() const
  {
    return entry->get_item();
  }

private:
  DbusmenuServer *server{nullptr};
  MenuModel::Ptr menu_model;
  detail::DbusSubMenuEntry::Ptr entry;
};

#endif // DBUSMENU_HH
