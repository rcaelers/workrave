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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <array>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>

#include <boost/signals2/signal.hpp>

#include "commonui/MenuDefs.hh"
#include "commonui/MenuModel.hh"
#include "commonui/MenuHelper.hh"
#include "ui/ITimerBoxView.hh"
#include "ui/TimerBoxControl.hh"
#include "utils/Signals.hh"
#include "ui/AppHold.hh"
#include "ui/Plugin.hh"

#if defined(HAVE_DBUS)
#  include "rpc/dbus/Registration.hh"
#endif

class AppletControl;

// @rpc(service="workrave.AppletService")
// @rpc.dbus(interface="org.workrave.AppletInterface")
class GenericDBusApplet
  : public Plugin<GenericDBusApplet>
  , public ITimerBoxView
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
    uint32_t command{};
    uint8_t type{};
    uint8_t flags{};
  };

  using MenuItems = std::list<MenuItem>;

  explicit GenericDBusApplet(std::shared_ptr<IPluginContext> context);
  ~GenericDBusApplet() override;

  void init();

  // @rpc(name="Embed")
  virtual void applet_embed(bool enabled, const std::string &sender);
  // @rpc(name="Command")
  virtual void applet_command(int command);
  // @rpc(name="MenuAction")
  virtual void applet_menu_action(const std::string &action);
  // @rpc(name="ButtonClicked")
  virtual void button_clicked(uint32_t button);
  // @rpc(name="GetMenu")
  // @rpc.param(menuitems, dir=out)
  virtual void get_menu(std::list<MenuItem> &menuitems);
  // @rpc(name="GetTrayIconEnabled")
  // @rpc.param(enabled, dir=out)
  virtual void get_tray_icon_enabled(bool &enabled) const;

  // @rpc.signal(name="TimersUpdated", fields="micro,rest,daily")
  boost::signals2::signal<void(TimerData, TimerData, TimerData)> &signal_timers_updated();
  // @rpc.signal(name="MenuUpdated", fields="menuitems")
  boost::signals2::signal<void(MenuItems)> &signal_menu_updated();
  // @rpc.signal(name="MenuItemUpdated", fields="menuitem")
  boost::signals2::signal<void(MenuItem)> &signal_menu_item_updated();
  // @rpc.signal(name="TrayIconUpdated", fields="enabled")
  boost::signals2::signal<void(bool)> &signal_tray_icon_updated();

#if defined(HAVE_DBUS)
  using NameWatcher =
    std::function<workrave::rpc::dbus::Registration(std::string, std::function<void(bool)>)>;
  void set_name_watcher(NameWatcher watcher);
  void publish_state();
#endif

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
  void update_view() override;

  void bus_name_presence(const std::string &name, bool present);

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
  std::array<TimerData, workrave::BREAK_ID_SIZEOF> data;
  std::set<std::string> active_bus_names;
#if defined(HAVE_DBUS)
  NameWatcher name_watcher;
  std::map<std::string, workrave::rpc::dbus::Registration> name_watches;
#endif
  std::shared_ptr<TimerBoxControl> control;
  boost::signals2::signal<void(TimerData, TimerData, TimerData)> timers_updated_signal;
  boost::signals2::signal<void(MenuItems)> menu_updated_signal;
  boost::signals2::signal<void(MenuItem)> menu_item_updated_signal;
  boost::signals2::signal<void(bool)> tray_icon_updated_signal;
};

#endif // GENERICDBUSAPPLET_HH
