// Copyright (C) 2001 - 2021 Rob Caelers & Raymond Penners
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

#ifndef GENERICDBUSAPPLET_HH
#define GENERICDBUSAPPLET_HH

#include <string>
#include <set>

#include "commonui/MenuDefs.hh"
#include "commonui/MenuModel.hh"
#include "commonui/MenuHelper.hh"
#include "ui/ITimerBoxView.hh"
#include "ui/TimerBoxControl.hh"
#include "utils/Signals.hh"
#include "dbus/IDBus.hh"
#include "dbus/IDBusWatch.hh"
#include "ui/AppHold.hh"
#include "ui/Plugin.hh"

#include "ui/prefwidgets/Widgets.hh"

class AppletControl;

class GenericDBusApplet
  : public Plugin<GenericDBusApplet>
  , public ITimerBoxView
  , public workrave::dbus::IDBusWatch
  , public workrave::utils::Trackable
{
public:
  std::string get_plugin_id() const override
  {
    return "workrave.GenericDBusApplet";
  }

  struct TimerData
  {
    std::string bar_text;
    int slot;
    uint32_t bar_secondary_color;
    uint32_t bar_secondary_val;
    uint32_t bar_secondary_max;
    uint32_t bar_primary_color;
    uint32_t bar_primary_val;
    uint32_t bar_primary_max;
  };

  struct MenuItem
  {
    MenuItem() = default;
    MenuItem(std::string text,
             std::string dynamic_text,
             std::string action,
             uint32_t command,
             MenuItemType type,
             uint8_t flags = 0)
      : text(std::move(text))
      , dynamic_text(std::move(dynamic_text))
      , action(std::move(action))
      , command(command)
      , type(static_cast<std::underlying_type_t<MenuItemType>>(type))
      , flags(flags)
    {
    }
    MenuItem(std::string text, std::string dynamic_text, std::string action, uint32_t command, uint8_t type, uint8_t flags = 0)
      : text(std::move(text))
      , dynamic_text(std::move(dynamic_text))
      , action(std::move(action))
      , command(command)
      , type(type)
      , flags(flags)
    {
    }
    std::string text;
    std::string dynamic_text;
    std::string action;
    uint32_t command;
    uint8_t type;
    uint8_t flags;
  };

  explicit GenericDBusApplet(std::shared_ptr<IPluginContext> context);
  ~GenericDBusApplet() override;

  void init();

  // DBus
  virtual void get_menu(std::list<MenuItem> &out);
  virtual void get_tray_icon_enabled(bool &enabled) const;
  virtual void applet_menu_action(const std::string &action);
  virtual void applet_command(int command);
  virtual void applet_embed(bool enable, const std::string &sender);
  virtual void button_clicked(int button);

  using MenuItems = std::list<MenuItem>;

private:
  // ITimerBoxView
  void set_slot(workrave::BreakId id, int slot) override;
  void set_time_bar(workrave::BreakId id,
                    int value,
                    TimerColorId primary_color,
                    int primary_value,
                    int primary_max,
                    TimerColorId secondary_color,
                    int secondary_value,
                    int secondary_max) override;
  void set_icon(OperationModeIcon icon) override;
  void set_geometry(Orientation orientation, int size) override;
  void update_view() override;

  // IDBusWatch
  void bus_name_presence(const std::string &name, bool present) override;

  void send_menu_updated_event();
  void init_menu_list(std::list<MenuItem> &items, menus::Node::Ptr node);
  void update_menu_item(menus::Node::Ptr node);
  void send_tray_icon_enabled();

private:
  std::shared_ptr<IPluginContext> context;
  MenuModel::Ptr menu_model;
  MenuHelper menu_helper;
  AppHold apphold;
  bool visible{false};
  bool embedded{false};
  TimerData data[workrave::BREAK_ID_SIZEOF];
  std::set<std::string> active_bus_names;
  workrave::dbus::IDBus::Ptr dbus;
  std::shared_ptr<TimerBoxControl> control;
};

#endif // GENERICDBUSAPPLET_HH
