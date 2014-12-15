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

#include <boost/shared_ptr.hpp>

#include "utils/ScopedConnections.hh"

#include "ICore.hh"

#include "IToolkit.hh"
#include "IApplication.hh"

#include "MenuModel.hh"

class Menus
{
public:
  typedef boost::shared_ptr<Menus> Ptr;

  static Menus::Ptr create(IApplication::Ptr app, IToolkit::Ptr toolkit, workrave::ICore::Ptr core);

  Menus(IApplication::Ptr app, IToolkit::Ptr toolkit, workrave::ICore::Ptr core);
  virtual ~Menus();

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

  // FIXME: for dbus interface.
public:
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

  MenuModel::Ptr quiet_item;
  MenuModel::Ptr suspended_item;
  MenuModel::Ptr normal_item;
  MenuModel::Ptr reading_item;

  IApplication::Ptr app;
  IToolkit::Ptr toolkit;
  workrave::ICore::Ptr core;

  scoped_connections connections;
};

#endif // MENUS_HH
