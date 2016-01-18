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
#include "config.h"
#endif

#include "Menus.hh"

#include "commonui/nls.h"
#include "debug.hh"

using namespace std;
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

Menus::Menus(std::shared_ptr<IApplication> app,
             std::shared_ptr<IToolkit> toolkit,
             std::shared_ptr<workrave::ICore> core)
  : app(app), toolkit(toolkit), core(core)
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

  MenuModel::Ptr item;

  item = std::make_shared<MenuModel>(OPEN,
                                     tr("Open").toStdString(),
                                     std::bind(&Menus::on_menu_open_main_window, this));
  menu_model->add_menu(item);

  item = std::make_shared<MenuModel>(PREFERENCES,
                                     tr("Preferences").toStdString(),
                                     std::bind(&Menus::on_menu_preferences, this));
  menu_model->add_menu(item);

  item = std::make_shared<MenuModel>(REST_BREAK,
                                     tr("Rest break").toStdString(),
                                     std::bind(&Menus::on_menu_restbreak_now, this));
  menu_model->add_menu(item);

  item = std::make_shared<MenuModel>(EXERCISES,
                                     tr("Exercises").toStdString(),
                                     std::bind(&Menus::on_menu_exercises, this));
  menu_model->add_menu(item);

  MenuModel::Ptr modemenu = std::make_shared<MenuModel>(MODE, tr("Mode").toStdString(), nullptr, MenuModelType::MENU);
  menu_model->add_menu(modemenu);

  normal_item = std::make_shared<MenuModel>(MODE_NORMAL,
                                            tr("Normal").toStdString(),
                                            std::bind(&Menus::on_menu_normal, this),
                                            MenuModelType::RADIO);
  normal_item->set_checked(mode == workrave::OperationMode::Normal);
  modemenu->add_menu(normal_item);

  suspended_item = std::make_shared<MenuModel>(MODE_SUSPENDED,
                                               tr("Suspended").toStdString(),
                                               std::bind(&Menus::on_menu_suspend, this),
                                               MenuModelType::RADIO);

  suspended_item->set_checked(mode == workrave::OperationMode::Suspended);
  modemenu->add_menu(suspended_item);

  quiet_item = std::make_shared<MenuModel>(MODE_QUIET,
                                           tr("Quiet").toStdString(),
                                           std::bind(&Menus::on_menu_quiet, this),
                                           MenuModelType::RADIO);
  quiet_item->set_checked(mode == workrave::OperationMode::Quiet);
  modemenu->add_menu(quiet_item);

  reading_item = std::make_shared<MenuModel>(MODE_READING,
                                             tr("Reading mode").toStdString(),
                                             std::bind(static_cast<void (Menus::*)()>(&Menus::on_menu_reading), this),
                                             MenuModelType::CHECK);
  reading_item->set_checked(usage == workrave::UsageMode::Reading);
  menu_model->add_menu(reading_item);

  item = std::make_shared<MenuModel>(STATISTICS,
                                     tr("Statistics").toStdString(),
                                     std::bind(&Menus::on_menu_statistics, this));
  menu_model->add_menu(item);

  item = std::make_shared<MenuModel>(ABOUT,
                                     tr("About...").toStdString(),
                                     std::bind(&Menus::on_menu_about, this));
  menu_model->add_menu(item);

  item = std::make_shared<MenuModel>(QUIT,
                                     tr("Quit").toStdString(),
                                     std::bind(&Menus::on_menu_quit, this));
  menu_model->add_menu(item);

  connections.connect(core->signal_operation_mode_changed(), std::bind(&Menus::on_operation_mode_changed, this, std::placeholders::_1));
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
