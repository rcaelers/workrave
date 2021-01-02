// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef MENUS_HH
#define MENUS_HH

#include <memory>
#include <string>

#include "core/ICore.hh"
#include "utils/ScopedConnections.hh"

#include "IApplication.hh"
#include "IToolkit.hh"
#include "MenuModel.hh"

class Menus
{
public:
  typedef std::shared_ptr<Menus> Ptr;

  Menus(std::shared_ptr<IApplication> app, std::shared_ptr<IToolkit> toolkit, std::shared_ptr<workrave::ICore> core);

  const MenuModel::Ptr get_menu_model() const;

  static const std::string PREFERENCES;
  static const std::string EXERCISES;
  static const std::string REST_BREAK;
  static const std::string MODE;
  static const std::string MODE_NORMAL;
  static const std::string MODE_QUIET;
  static const std::string MODE_SUSPENDED;
  static const std::string STATISTICS;
  static const std::string ABOUT;
  static const std::string MODE_READING;
  static const std::string OPEN;
  static const std::string QUIT;

private:
  void init();
  void set_operation_mode(workrave::OperationMode m);
  void set_usage_mode(workrave::UsageMode m);

  void on_menu_open_main_window();
  void on_menu_restbreak_now();
  void on_menu_about();
  void on_menu_quit();
  void on_menu_preferences();
  void on_menu_exercises();
  void on_menu_statistics();
  void on_menu_normal();
  void on_menu_suspend();
  void on_menu_quiet();
  void on_menu_reading();
  void on_menu_reading(bool on);
  void on_operation_mode_changed(const workrave::OperationMode m);
  void on_usage_mode_changed(const workrave::UsageMode m);

private:
  MenuModel::Ptr menu_model;
  MenuNode::Ptr quiet_item;
  MenuNode::Ptr suspended_item;
  MenuNode::Ptr normal_item;
  MenuNode::Ptr reading_item;

  std::shared_ptr<IApplication> app;
  std::shared_ptr<IToolkit> toolkit;
  std::shared_ptr<workrave::ICore> core;

  scoped_connections connections;

  // TODO: DBUS stubs, refactor
  friend class org_workrave_ControlInterface_Stub;
};

#endif // MENUS_HH
