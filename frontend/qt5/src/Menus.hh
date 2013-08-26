// Menus.cc
//
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

#include "ICore.hh"

#include "IToolkit.hh"
#include "IApplication.hh"

#include "MenuItem.hh"

class Menus
{
public:
  typedef boost::shared_ptr<Menus> Ptr;

  static Menus::Ptr create(IApplication::Ptr app, IToolkit::Ptr toolkit, workrave::ICore::Ptr core);

  Menus(IApplication::Ptr app, IToolkit::Ptr toolkit, workrave::ICore::Ptr core);
  virtual ~Menus();

  const MenuItem::Ptr get_top() const;

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

  void on_operation_mode_changed(const workrave::OperationMode m);
  void on_usage_mode_changed(const workrave::UsageMode m);
  
private:
  MenuItem::Ptr top;

  MenuItem::Ptr quiet_item;
  MenuItem::Ptr suspended_item;
  MenuItem::Ptr normal_item;
  MenuItem::Ptr reading_item;
  
  IApplication::Ptr app;
  IToolkit::Ptr toolkit;
  workrave::ICore::Ptr core;
};

#endif // MENUS_HH
