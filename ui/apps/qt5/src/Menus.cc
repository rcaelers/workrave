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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "Menus.hh"

#include "debug.hh"
#include "commonui/nls.h"

using namespace workrave;

const std::string Menus::PREFERENCES = "workrave:preferences";
const std::string Menus::EXERCISES = "workrave:exercises";
const std::string Menus::REST_BREAK = "workrave:restbreak";
const std::string Menus::MODE = "workrave:mode";
const std::string Menus::MODE_NORMAL = "workrave:mode_normal";
const std::string Menus::MODE_QUIET = "workrave:mode_quiet";
const std::string Menus::MODE_SUSPENDED = "workrave:mode_suspended";
const std::string Menus::STATISTICS = "workrave:statistics";
const std::string Menus::ABOUT = "workrave:about";
const std::string Menus::MODE_READING = "workrave:mode_reading";
const std::string Menus::OPEN = "workrave:open";
const std::string Menus::QUIT = "workrave:quit";

Menus::Menus(std::shared_ptr<IApplication> app, std::shared_ptr<IToolkit> toolkit, std::shared_ptr<workrave::ICore> core)
  : app(app)
  , toolkit(toolkit)
  , core(core)
{
  menu_model = std::make_shared<MenuModel>();
  init();
}

const MenuModel::Ptr
Menus::get_menu_model() const
{
  return menu_model;
}

void
Menus::init()
{
  workrave::OperationMode mode = core->get_operation_mode();
  workrave::UsageMode usage = core->get_usage_mode();

  MenuNode::Ptr root = menu_model->get_root();

  MenuNode::Ptr item = std::make_shared<MenuNode>(OPEN, N_("Open"), std::bind(&Menus::on_menu_open_main_window, this));
  root->add_menu_item(item);

  MenuNode::Ptr separator = std::make_shared<MenuNode>(MenuNodeType::SEPARATOR);
  root->add_menu_item(separator);

  item = std::make_shared<MenuNode>(PREFERENCES, N_("Preferences"), std::bind(&Menus::on_menu_preferences, this));
  root->add_menu_item(item);

  item = std::make_shared<MenuNode>(REST_BREAK, N_("Rest break"), std::bind(&Menus::on_menu_restbreak_now, this));
  root->add_menu_item(item);

  item = std::make_shared<MenuNode>(EXERCISES, N_("Exercises"), std::bind(&Menus::on_menu_exercises, this));
  root->add_menu_item(item);

  item = std::make_shared<MenuNode>(STATISTICS, N_("Statistics"), std::bind(&Menus::on_menu_statistics, this));
  root->add_menu_item(item);

  separator = std::make_shared<MenuNode>(MenuNodeType::SEPARATOR);
  root->add_menu_item(separator);

  MenuNode::Ptr modemenu = std::make_shared<MenuNode>(MODE, N_("Mode"), nullptr, MenuNodeType::MENU);
  root->add_menu_item(modemenu);

  normal_item = std::make_shared<MenuNode>(MODE_NORMAL, N_("Normal"), std::bind(&Menus::on_menu_normal, this), MenuNodeType::RADIO);
  normal_item->set_checked(mode == workrave::OperationMode::Normal);
  modemenu->add_menu_item(normal_item);

  suspended_item =
    std::make_shared<MenuNode>(MODE_SUSPENDED, N_("Suspended"), std::bind(&Menus::on_menu_suspend, this), MenuNodeType::RADIO);

  suspended_item->set_checked(mode == workrave::OperationMode::Suspended);
  modemenu->add_menu_item(suspended_item);

  quiet_item = std::make_shared<MenuNode>(MODE_QUIET, N_("Quiet"), std::bind(&Menus::on_menu_quiet, this), MenuNodeType::RADIO);
  quiet_item->set_checked(mode == workrave::OperationMode::Quiet);
  modemenu->add_menu_item(quiet_item);

  reading_item = std::make_shared<MenuNode>(MODE_READING,
                                            N_("Reading mode"),
                                            std::bind(static_cast<void (Menus::*)()>(&Menus::on_menu_reading), this),
                                            MenuNodeType::CHECK);
  reading_item->set_checked(usage == workrave::UsageMode::Reading);
  root->add_menu_item(reading_item);

  separator = std::make_shared<MenuNode>(MenuNodeType::SEPARATOR);
  root->add_menu_item(separator);

  item = std::make_shared<MenuNode>(ABOUT, N_("About..."), std::bind(&Menus::on_menu_about, this));
  root->add_menu_item(item);

  item = std::make_shared<MenuNode>(QUIT, N_("Quit"), std::bind(&Menus::on_menu_quit, this));
  root->add_menu_item(item);
  menu_model->update();

  connections.connect(core->signal_operation_mode_changed(),
                      std::bind(&Menus::on_operation_mode_changed, this, std::placeholders::_1));
  connections.connect(core->signal_usage_mode_changed(), std::bind(&Menus::on_usage_mode_changed, this, std::placeholders::_1));
}

void
Menus::on_menu_open_main_window()
{
  toolkit->show_window(IToolkit::WindowType::Main);
}

void
Menus::on_menu_restbreak_now()
{
  app->restbreak_now();
}

void
Menus::on_menu_about()
{
  toolkit->show_window(IToolkit::WindowType::About);
}

void
Menus::on_menu_quit()
{
  app->terminate();
}

void
Menus::on_menu_preferences()
{
  toolkit->show_window(IToolkit::WindowType::Preferences);
}

void
Menus::on_menu_exercises()
{
  toolkit->show_window(IToolkit::WindowType::Exercises);
}

void
Menus::on_menu_statistics()
{
  toolkit->show_window(IToolkit::WindowType::Statistics);
}

void
Menus::on_menu_normal()
{
  set_operation_mode(OperationMode::Normal);
}

void
Menus::on_menu_suspend()
{
  set_operation_mode(OperationMode::Suspended);
}

void
Menus::on_menu_quiet()
{
  set_operation_mode(OperationMode::Quiet);
}

void
Menus::on_menu_reading()
{
  set_usage_mode(reading_item->is_checked() ? UsageMode::Reading : UsageMode::Normal);
}

void
Menus::on_menu_reading(bool on)
{
  set_usage_mode(on ? UsageMode::Reading : UsageMode::Normal);
}

void
Menus::set_operation_mode(OperationMode m)
{
  core->set_operation_mode(m);
}

void
Menus::set_usage_mode(UsageMode m)
{
  core->set_usage_mode(m);
}

void
Menus::on_operation_mode_changed(const OperationMode m)
{
  normal_item->set_checked(m == OperationMode::Normal);
  suspended_item->set_checked(m == OperationMode::Suspended);
  quiet_item->set_checked(m == OperationMode::Quiet);
}

void
Menus::on_usage_mode_changed(const UsageMode m)
{
  reading_item->set_checked(m == UsageMode::Reading);
}
