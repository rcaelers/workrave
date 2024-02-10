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

#include "DbusMenu.hh"

#include <string>

#include "indicator-applet.h"
#include "commonui/MenuModel.hh"

using namespace std;
using namespace detail;

DbusMenu::DbusMenu(std::shared_ptr<IPluginContext> context)
  : menu_model(context->get_menu_model())
{
  menus::SubMenuNode::Ptr root = menu_model->get_root();

  server = dbusmenu_server_new(WORKRAVE_INDICATOR_MENU_OBJ);
  entry = std::make_shared<detail::DbusSubMenuEntry>(server, menu_model->get_root());
  workrave::utils::connect(menu_model->signal_update(), this, [this]() { entry->init(); });
}

DbusMenuEntry::Ptr
DbusMenuEntryFactory::create(DbusSubMenuEntry *parent, menus::Node::Ptr node)
{
  if (auto n = std::dynamic_pointer_cast<menus::SubMenuNode>(node); n)
    {
      return std::make_shared<DbusSubMenuEntry>(parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::RadioGroupNode>(node); n)
    {
      return std::make_shared<DbusRadioGroupMenuEntry>(parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::ActionNode>(node); n)
    {
      return std::make_shared<DbusActionMenuEntry>(parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::ToggleNode>(node); n)
    {
      return std::make_shared<DbusToggleMenuEntry>(parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::RadioNode>(node); n)
    {
      return std::make_shared<DbusRadioMenuEntry>(parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::SeparatorNode>(node); n)
    {
      return std::make_shared<DbusSeparatorMenuEntry>(parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::SectionNode>(node); n)
    {
      return std::make_shared<DbusSectionMenuEntry>(parent, n);
    }

  return {};
}

DbusMenuEntry::~DbusMenuEntry()
{
  if (item != nullptr)
    {
      GList *c = dbusmenu_menuitem_take_children(item);
      // g_list_foreach (c, (GFunc)g_object_unref, NULL);
      g_list_free(c);
      g_object_unref(item);
    }
}

DbusmenuMenuitem *
DbusMenuEntry::get_item() const
{
  return item;
}

//////////////////////////////////////////////////////////////////////

DbusSubMenuEntry::DbusSubMenuEntry(DbusSubMenuEntry *parent, menus::SubMenuNode::Ptr node)
  : parent(parent)
  , node(node)
{
  init();
}

DbusSubMenuEntry::DbusSubMenuEntry(DbusmenuServer *server, menus::SubMenuNode::Ptr node)
  : server(server)
  , node(node)
{
  init();
}

void
DbusSubMenuEntry::init()
{
  auto *new_item = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set(new_item, DBUSMENU_MENUITEM_PROP_LABEL, node->get_text().c_str());

  if (parent != nullptr)
    {
      parent->add(new_item);
    }
  else if (server != nullptr)
    {
      dbusmenu_server_set_root(server, new_item);
      dbusmenu_menuitem_property_set_bool(new_item, DBUSMENU_MENUITEM_PROP_VISIBLE, TRUE);
    }

  if (item != nullptr)
    {
      GList *c = dbusmenu_menuitem_take_children(item);
      // g_list_foreach (c, (GFunc)g_object_unref, NULL);
      g_list_free(c);
      children.clear();
      //   g_object_unref(item);
    }
  item = new_item;

  for (auto menu_to_add: node->get_children())
    {
      DbusMenuEntry::Ptr child = DbusMenuEntryFactory::create(this, menu_to_add);
      children.push_back(child);
    }
}

void
DbusSubMenuEntry::add(DbusmenuMenuitem *child)
{
  dbusmenu_menuitem_child_append(item, child);
};

void DbusSubMenuEntry::add_section(){};

//////////////////////////////////////////////////////////////////////

DbusRadioGroupMenuEntry::DbusRadioGroupMenuEntry(DbusSubMenuEntry *parent, menus::RadioGroupNode::Ptr node)
{
  item = parent->get_item();
  for (auto child_node: node->get_children())
    {
      auto child = std::make_shared<DbusRadioMenuEntry>(parent, child_node);
      children.push_back(child);
    }
}

//////////////////////////////////////////////////////////////////////

DbusActionMenuEntry::DbusActionMenuEntry(DbusSubMenuEntry *parent, menus::ActionNode::Ptr node)
  : node(node)
{
  item = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_LABEL, node->get_text().c_str());

  g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(static_menu_item_activated), this);

  parent->add(item);
}

void
DbusActionMenuEntry::static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data)
{
  (void)timestamp;
  (void)mi;

  auto *menu = (DbusActionMenuEntry *)user_data;
  menu->menu_item_activated(mi);
}

void
DbusActionMenuEntry::menu_item_activated(DbusmenuMenuitem *mi)
{
  (void)mi;
  node->activate();
}

//////////////////////////////////////////////////////////////////////

DbusToggleMenuEntry::DbusToggleMenuEntry(DbusSubMenuEntry *parent, menus::ToggleNode::Ptr node)
  : node(node)
{
  item = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_LABEL, node->get_text().c_str());
  dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE, DBUSMENU_MENUITEM_TOGGLE_CHECK);
  dbusmenu_menuitem_property_set_int(item,
                                     DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                     node->is_checked() ? DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED
                                                        : DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);

  g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(static_menu_item_activated), this);

  workrave::utils::connect(node->signal_changed(), this, [this, node]() {
    dbusmenu_menuitem_property_set_int(item,
                                       DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                       node->is_checked() ? DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED
                                                          : DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);
  });

  parent->add(item);
}

void
DbusToggleMenuEntry::static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data)
{
  (void)timestamp;
  (void)mi;

  auto *menu = (DbusToggleMenuEntry *)user_data;
  menu->menu_item_activated(mi);
}

void
DbusToggleMenuEntry::menu_item_activated(DbusmenuMenuitem *mi)
{
  (void)mi;
  node->activate();
}

//////////////////////////////////////////////////////////////////////

DbusRadioMenuEntry::DbusRadioMenuEntry(DbusSubMenuEntry *parent, menus::RadioNode::Ptr node)
  : node(node)
{
  item = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_LABEL, node->get_text().c_str());
  dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE, DBUSMENU_MENUITEM_TOGGLE_RADIO);
  dbusmenu_menuitem_property_set_int(item,
                                     DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                     node->is_checked() ? DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED
                                                        : DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);

  g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(static_menu_item_activated), this);

  workrave::utils::connect(node->signal_changed(), this, [this, node]() {
    dbusmenu_menuitem_property_set_int(item,
                                       DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                       node->is_checked() ? DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED
                                                          : DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);
  });

  parent->add(item);
}

void
DbusRadioMenuEntry::static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data)
{
  (void)timestamp;
  (void)mi;

  auto *menu = (DbusRadioMenuEntry *)user_data;
  menu->menu_item_activated(mi);
}

void
DbusRadioMenuEntry::menu_item_activated(DbusmenuMenuitem *mi)
{
  (void)mi;
  node->activate();
}

//////////////////////////////////////////////////////////////////////

DbusSeparatorMenuEntry::DbusSeparatorMenuEntry(DbusSubMenuEntry *parent, menus::SeparatorNode::Ptr)
{
  parent->add_section();
}

//////////////////////////////////////////////////////////////////////

DbusSectionMenuEntry::DbusSectionMenuEntry(DbusSubMenuEntry *parent, menus::SectionNode::Ptr node)
{
  item = parent->get_item();
  for (auto child_node: node->get_children())
    {
      auto child = DbusMenuEntryFactory::create(parent, child_node);
      children.push_back(child);
    }
}
