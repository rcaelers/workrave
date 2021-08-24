// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#include "commonui/nls.h"
#include "debug.hh"

#include <gtkmm.h>

#include "GenericDBusApplet.hh"

#include "commonui/TimerBoxControl.hh"
#include "commonui/GUIConfig.hh"
#include "Text.hh"
#include "GUI.hh"
#include "Menus.hh"
#include "commonui/Backend.hh"
#include "config/IConfigurator.hh"

#include "dbus/IDBus.hh"
#include "dbus/DBusException.hh"
#include "DBusGUI.hh"

#define WORKRAVE_APPLET_SERVICE_NAME "org.workrave.Workrave"
#define WORKRAVE_APPLET_SERVICE_IFACE "org.workrave.AppletInterface"
#define WORKRAVE_APPLET_SERVICE_OBJ "/org/workrave/Workrave/UI"

GenericDBusApplet::GenericDBusApplet(MenuModel::Ptr menu_model)
  : menu_model(menu_model)
  , menu_helper(menu_model)
{
  timer_box_control = new TimerBoxControl("applet", this);
  timer_box_view = this;

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      data[i].bar_text = "";
      data[i].bar_primary_color = 0;
      data[i].bar_primary_val = 0;
      data[i].bar_primary_max = 0;
      data[i].bar_secondary_color = 0;
      data[i].bar_secondary_val = 0;
      data[i].bar_secondary_max = 0;
    }

  GUIConfig::trayicon_enabled().connect(this, [this](bool) { send_tray_icon_enabled(); });
  init();
}

void
GenericDBusApplet::set_slot(BreakId id, int slot)
{
  TRACE_ENTER_MSG("GenericDBusApplet::set_slot", int(id) << ", " << slot);
  data[slot].slot = id;
  TRACE_EXIT();
}

void
GenericDBusApplet::set_time_bar(BreakId id,
                                int value,
                                TimerColorId primary_color,
                                int primary_val,
                                int primary_max,
                                TimerColorId secondary_color,
                                int secondary_val,
                                int secondary_max)
{
  TRACE_ENTER_MSG("GenericDBusApplet::set_time_bar", int(id) << "=" << value);
  data[id].bar_text = Text::time_to_string(value);
  data[id].bar_primary_color = (int)primary_color;
  data[id].bar_primary_val = primary_val;
  data[id].bar_primary_max = primary_max;
  data[id].bar_secondary_color = (int)secondary_color;
  data[id].bar_secondary_val = secondary_val;
  data[id].bar_secondary_max = secondary_max;
  TRACE_EXIT();
}

void
GenericDBusApplet::update_view()
{
  TRACE_ENTER("GenericDBusApplet::update_view");

  org_workrave_AppletInterface *iface = org_workrave_AppletInterface::instance(dbus);
  assert(iface != nullptr);
  iface->TimersUpdated(WORKRAVE_APPLET_SERVICE_OBJ, data[BREAK_ID_MICRO_BREAK], data[BREAK_ID_REST_BREAK], data[BREAK_ID_DAILY_LIMIT]);

  TRACE_EXIT();
}

void
GenericDBusApplet::init()
{
  try
    {
      dbus = Backend::get_dbus();
      if (dbus->is_available())
        {
          dbus->connect(WORKRAVE_APPLET_SERVICE_OBJ, WORKRAVE_APPLET_SERVICE_IFACE, this);

          workrave::utils::connect(menu_model->signal_update(), this, [this]() { send_menu_updated_event(); });

          workrave::utils::connect(menu_helper.signal_update(), this, [this](auto node) { update_menu_item(node); });

          menu_helper.setup_event();

          send_menu_updated_event();
        }
    }
  catch (workrave::dbus::DBusException &)
    {
    }
}

void
GenericDBusApplet::applet_embed(bool enable, const std::string &sender)
{
  TRACE_ENTER_MSG("GenericDBusApplet::applet_embed", enable << " " << sender);
  embedded = enable;

  for (const auto &bus_name: active_bus_names)
    {
      dbus->unwatch(bus_name);
    }
  active_bus_names.clear();

  if (sender != "")
    {
      dbus->watch(sender, this);
    }

  if (!enable)
    {
      TRACE_MSG("Disabling");
      visible = false;
      visibility_changed_signal.emit(false);
    }

  TRACE_EXIT();
}

void
GenericDBusApplet::get_menu(std::list<MenuItem> &out)
{
  std::list<MenuItem> items;
  init_menu_list(items, menu_model->get_root());
  out = items;
}

void
GenericDBusApplet::get_tray_icon_enabled(bool &enabled) const
{
  enabled = GUIConfig::applet_icon_enabled()();
}

void
GenericDBusApplet::applet_command(int command)
{
  auto node = menu_helper.find_node(command);
  if (node)
    {
      node->activate();
    }
}

void
GenericDBusApplet::applet_menu_action(const std::string &id)
{
  auto node = menu_helper.find_node(id);
  if (node)
    {
      node->activate();
    }
}

void
GenericDBusApplet::button_clicked(int button)
{
  (void)button;
  timer_box_control->force_cycle();
}

void
GenericDBusApplet::bus_name_presence(const std::string &name, bool present)
{
  TRACE_ENTER_MSG("GenericDBusApplet::bus_name_presence", name << " " << present);
  TRACE_MSG(visible << " " << embedded);
  if (present)
    {
      active_bus_names.insert(name);
      if (!visible && embedded)
        {
          TRACE_MSG("Enabling");
          visible = true;
          visibility_changed_signal.emit(true);
        }
    }
  else
    {
      active_bus_names.erase(name);
      if (active_bus_names.size() == 0)
        {
          TRACE_MSG("Disabling");
          visible = false;
          embedded = false;
          visibility_changed_signal.emit(false);
        }
    }
  TRACE_EXIT();
}

void
GenericDBusApplet::send_tray_icon_enabled()
{
  TRACE_ENTER("GenericDBusApplet::send_tray_icon_enabled");
  bool on = GUIConfig::applet_icon_enabled()();

  org_workrave_AppletInterface *iface = org_workrave_AppletInterface::instance(dbus);
  assert(iface != nullptr);
  iface->TrayIconUpdated(WORKRAVE_APPLET_SERVICE_OBJ, on);

  TRACE_EXIT();
}

bool
GenericDBusApplet::is_visible() const
{
  return visible;
}

void
GenericDBusApplet::send_menu_updated_event()
{
  std::list<MenuItem> items;
  init_menu_list(items, menu_model->get_root());

  org_workrave_AppletInterface *iface = org_workrave_AppletInterface::instance(dbus);
  iface->MenuUpdated(WORKRAVE_APPLET_SERVICE_OBJ, items);
}

void
GenericDBusApplet::init_menu_list(std::list<MenuItem> items, menus::Node::Ptr node)
{
  uint32_t command = menu_helper.allocate_command(node->get_id());

  uint8_t flags = MENU_ITEM_FLAG_NONE;

  if (node->is_visible())
    {
      flags |= MENU_ITEM_FLAG_VISIBLE;
    }

  if (auto n = std::dynamic_pointer_cast<menus::SubMenuNode>(node); n)
    {
      bool add_sub_menu = items.size() > 0;

      if (add_sub_menu)
        {
          items.emplace_back(n->get_text(), node->get_id(), command, MenuItemType::SubMenuBegin, flags);
        }
      for (auto &menu_to_add: n->get_children())
        {
          init_menu_list(items, menu_to_add);
        }
      if (add_sub_menu)
        {
          items.emplace_back(n->get_text(), node->get_id(), command, MenuItemType::SubMenuEnd, flags);
        }
    }

  else if (auto n = std::dynamic_pointer_cast<menus::RadioGroupNode>(node); n)
    {
      items.emplace_back(n->get_text(), node->get_id(), command, MenuItemType::RadioGroupBegin, flags);
      for (auto &menu_to_add: n->get_children())
        {
          init_menu_list(items, menu_to_add);
        }
      items.emplace_back(n->get_text(), node->get_id(), command, MenuItemType::RadioGroupEnd, flags);
    }

  else if (auto n = std::dynamic_pointer_cast<menus::ActionNode>(node); n)
    {
      items.emplace_back(n->get_text(), node->get_id(), command, MenuItemType::Action, flags);
    }

  else if (auto n = std::dynamic_pointer_cast<menus::ToggleNode>(node); n)
    {
      if (n->is_checked())
        {
          flags |= MENU_ITEM_FLAG_ACTIVE;
        }
      items.emplace_back(n->get_text(), node->get_id(), command, MenuItemType::Check, flags);
    }

  else if (auto n = std::dynamic_pointer_cast<menus::RadioNode>(node); n)
    {
      if (n->is_checked())
        {
          flags |= MENU_ITEM_FLAG_ACTIVE;
        }
      items.emplace_back(n->get_text(), node->get_id(), command, MenuItemType::Radio, flags);
    }

  else if (auto n = std::dynamic_pointer_cast<menus::SeparatorNode>(node); n)
    {
      items.emplace_back(n->get_text(), node->get_id(), command, MenuItemType::Separator, flags);
    }
}

void
GenericDBusApplet::update_menu_item(menus::Node::Ptr node)
{
  uint32_t command = menu_helper.allocate_command(node->get_id());
  uint8_t flags = MENU_ITEM_FLAG_NONE;
  MenuItemType type{};

  if (node->is_visible())
    {
      flags |= MENU_ITEM_FLAG_VISIBLE;
    }

  if (auto n = std::dynamic_pointer_cast<menus::SubMenuNode>(node); n)
    {
      type = MenuItemType::SubMenuBegin;
    }
  else if (auto n = std::dynamic_pointer_cast<menus::RadioGroupNode>(node); n)
    {
      type = MenuItemType::RadioGroupBegin;
    }
  else if (auto n = std::dynamic_pointer_cast<menus::ActionNode>(node); n)
    {
      type = MenuItemType::Action;
    }
  else if (auto n = std::dynamic_pointer_cast<menus::ToggleNode>(node); n)
    {
      if (n->is_checked())
        {
          flags |= MENU_ITEM_FLAG_ACTIVE;
        }
      type = MenuItemType::Check;
    }

  else if (auto n = std::dynamic_pointer_cast<menus::RadioNode>(node); n)
    {
      if (n->is_checked())
        {
          flags |= MENU_ITEM_FLAG_ACTIVE;
        }
      type = MenuItemType::Radio;
    }

  else if (auto n = std::dynamic_pointer_cast<menus::SeparatorNode>(node); n)
    {
      type = MenuItemType::Separator;
    }

  MenuItem item(node->get_text(), node->get_id(), command, type, flags);
  org_workrave_AppletInterface *iface = org_workrave_AppletInterface::instance(dbus);
  iface->MenuItemUpdated(WORKRAVE_APPLET_SERVICE_OBJ, item);
}
