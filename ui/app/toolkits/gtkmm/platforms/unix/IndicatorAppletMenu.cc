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

#include "IndicatorAppletMenu.hh"

#include <string>

#include "debug.hh"

#include "indicator-applet.h"
#include "ui/MenuModel.hh"

using namespace std;
using namespace detail;

IndicatorAppletMenu::IndicatorAppletMenu(std::shared_ptr<IApplication> app)
  : menu_model(app->get_menu_model())
{
  menus::SubMenuNode::Ptr root = menu_model->get_root();

  server = dbusmenu_server_new(WORKRAVE_INDICATOR_MENU_OBJ);

  entry = std::make_shared<detail::IndicatorSubMenuEntry>(server, menu_model->get_root());

  workrave::utils::connect(menu_model->signal_update(), this, [this]() { entry->init(); });
}

IndicatorMenuEntry::Ptr
IndicatorMenuEntryFactory::create(IndicatorSubMenuEntry *parent, menus::Node::Ptr node)
{
  if (auto n = std::dynamic_pointer_cast<menus::SubMenuNode>(node); n)
    {
      return std::make_shared<IndicatorSubMenuEntry>(parent, n);
    }

  else if (auto n = std::dynamic_pointer_cast<menus::RadioGroupNode>(node); n)
    {
      return std::make_shared<IndicatorRadioGroupMenuEntry>(parent, n);
    }

  else if (auto n = std::dynamic_pointer_cast<menus::ActionNode>(node); n)
    {
      return std::make_shared<IndicatorActionMenuEntry>(parent, n);
    }

  else if (auto n = std::dynamic_pointer_cast<menus::ToggleNode>(node); n)
    {
      return std::make_shared<IndicatorToggleMenuEntry>(parent, n);
    }

  else if (auto n = std::dynamic_pointer_cast<menus::RadioNode>(node); n)
    {
      return std::make_shared<IndicatorRadioMenuEntry>(parent, n);
    }

  else if (auto n = std::dynamic_pointer_cast<menus::SeparatorNode>(node); n)
    {
      return std::make_shared<IndicatorSeparatorMenuEntry>(parent, n);
    }

  else
    {
      return IndicatorMenuEntry::Ptr();
    }
}

IndicatorMenuEntry::IndicatorMenuEntry()
{
}

IndicatorMenuEntry::~IndicatorMenuEntry()
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
IndicatorMenuEntry::get_item() const
{
  return item;
}

//////////////////////////////////////////////////////////////////////

IndicatorSubMenuEntry::IndicatorSubMenuEntry(IndicatorSubMenuEntry *parent, menus::SubMenuNode::Ptr node)
  : parent(parent)
  , node(node)
{
  init();
}

IndicatorSubMenuEntry::IndicatorSubMenuEntry(DbusmenuServer *server, menus::SubMenuNode::Ptr node)
  : server(server)
  , node(node)
{
  init();
}

void
IndicatorSubMenuEntry::init()
{
  auto new_item = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set(new_item, DBUSMENU_MENUITEM_PROP_LABEL, node->get_text().c_str());

  if (parent)
    {
      parent->add(new_item);
    }
  else if (server)
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
      IndicatorMenuEntry::Ptr child = IndicatorMenuEntryFactory::create(this, menu_to_add);
      children.push_back(child);
    }
}

void
IndicatorSubMenuEntry::add(DbusmenuMenuitem *child)
{
  dbusmenu_menuitem_child_append(item, child);
};

void IndicatorSubMenuEntry::add_section(){};

//////////////////////////////////////////////////////////////////////

IndicatorRadioGroupMenuEntry::IndicatorRadioGroupMenuEntry(IndicatorSubMenuEntry *parent, menus::RadioGroupNode::Ptr node)
{
  item = parent->get_item();
  for (auto child_node: node->get_children())
    {
      auto child = std::make_shared<IndicatorRadioMenuEntry>(parent, child_node);
      children.push_back(child);
    }
}

//////////////////////////////////////////////////////////////////////

IndicatorActionMenuEntry::IndicatorActionMenuEntry(IndicatorSubMenuEntry *parent, menus::ActionNode::Ptr node)
  : node(node)
{
  item = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_LABEL, node->get_text().c_str());

  g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(static_menu_item_activated), this);

  parent->add(item);
}

void
IndicatorActionMenuEntry::static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data)
{
  (void)timestamp;
  (void)mi;

  IndicatorActionMenuEntry *menu = (IndicatorActionMenuEntry *)user_data;
  menu->menu_item_activated(mi);
}

void
IndicatorActionMenuEntry::menu_item_activated(DbusmenuMenuitem *mi)
{
  (void)mi;
  node->activate();
}

//////////////////////////////////////////////////////////////////////

IndicatorToggleMenuEntry::IndicatorToggleMenuEntry(IndicatorSubMenuEntry *parent, menus::ToggleNode::Ptr node)
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
IndicatorToggleMenuEntry::static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data)
{
  (void)timestamp;
  (void)mi;

  IndicatorToggleMenuEntry *menu = (IndicatorToggleMenuEntry *)user_data;
  menu->menu_item_activated(mi);
}

void
IndicatorToggleMenuEntry::menu_item_activated(DbusmenuMenuitem *mi)
{
  (void)mi;
  node->activate();
}

//////////////////////////////////////////////////////////////////////

IndicatorRadioMenuEntry::IndicatorRadioMenuEntry(IndicatorSubMenuEntry *parent, menus::RadioNode::Ptr node)
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
IndicatorRadioMenuEntry::static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data)
{
  (void)timestamp;
  (void)mi;

  IndicatorRadioMenuEntry *menu = (IndicatorRadioMenuEntry *)user_data;
  menu->menu_item_activated(mi);
}

void
IndicatorRadioMenuEntry::menu_item_activated(DbusmenuMenuitem *mi)
{
  (void)mi;
  node->activate();
}

//////////////////////////////////////////////////////////////////////

IndicatorSeparatorMenuEntry::IndicatorSeparatorMenuEntry(IndicatorSubMenuEntry *parent, menus::SeparatorNode::Ptr)
{
  parent->add_section();
}

//////////////////////////////////////////////////////////////////////

IndicatorSectionMenuEntry::IndicatorSectionMenuEntry(IndicatorSubMenuEntry *parent, menus::RadioGroupNode::Ptr node)
{
  item = parent->get_item();
  for (auto child_node: node->get_children())
    {
      auto child = IndicatorMenuEntryFactory::create(parent, child_node);
      children.push_back(child);
    }
}
