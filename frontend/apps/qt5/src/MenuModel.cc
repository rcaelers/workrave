// MenuModel.cc
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "MenuModel.hh"
#include "CoreTypes.hh"

using namespace std;
using namespace workrave;

MenuModel::Ptr
MenuModel::create(IApplication::Ptr app, IToolkit::Ptr toolkit, workrave::ICore::Ptr core)
{
  return Ptr(new MenuModel(app, toolkit, core));
}


MenuModel::MenuModel(IApplication::Ptr app, IToolkit::Ptr toolkit, workrave::ICore::Ptr core)
  : app(app), toolkit(toolkit), core(core)
{
  top = MenuItem::create();
  init();
}


MenuModel::~MenuModel()
{
}


const MenuItem::Ptr
MenuModel::get_top() const
{
  return top;
}


void
MenuModel::init()
{
  workrave::OperationMode mode = core->get_operation_mode();
  workrave::UsageMode usage = core->get_usage_mode();

  MenuItem::Ptr item;
  
  item = MenuItem::create(_("Open"),
                          boost::bind(&MenuModel::on_menu_open_main_window, this));
  top->add_menu(item);
  
  item = MenuItem::create(_("Preferences"),
                          boost::bind(&MenuModel::on_menu_preferences, this));
  top->add_menu(item);
  
  item = MenuItem::create(_("Rest break"),
                          boost::bind(&MenuModel::on_menu_restbreak_now, this));
  top->add_menu(item);
  
  item = MenuItem::create(_("Exercises"),
                          boost::bind(&MenuModel::on_menu_exercises, this));
  top->add_menu(item);
  
  MenuItem::Ptr modemenu = MenuItem::create(_("Mode"), 0, MenuItemType::MENU);
  top->add_menu(modemenu);
  
  normal_item = MenuItem::create(_("Normal"),
                                 boost::bind(&MenuModel::on_menu_normal, this),
                                 MenuItemType::RADIO);
  normal_item->set_checked(mode == workrave::OperationMode::Normal);
  modemenu->add_menu(normal_item);
  
  suspended_item = MenuItem::create(_("Suspended"),
                                    boost::bind(&MenuModel::on_menu_suspend, this),
                                    MenuItemType::RADIO);
  
  suspended_item->set_checked(mode == workrave::OperationMode::Suspended);
  modemenu->add_menu(suspended_item);
  
  quiet_item = MenuItem::create(_("Quiet"),
                                boost::bind(&MenuModel::on_menu_quiet, this),
                                MenuItemType::RADIO);
  quiet_item->set_checked(mode == workrave::OperationMode::Quiet);
  modemenu->add_menu(quiet_item);

  reading_item = MenuItem::create(_("Reading mode"),
                                  boost::bind(&MenuModel::on_menu_reading, this),
                                  MenuItemType::CHECK);
  reading_item->set_checked(usage == workrave::UsageMode::Reading);
  top->add_menu(reading_item);
  
  item = MenuItem::create(_("Statistics"),
                          boost::bind(&MenuModel::on_menu_statistics, this));
  top->add_menu(item);
  
  item = MenuItem::create(_("About..."),
                          boost::bind(&MenuModel::on_menu_about, this));
  top->add_menu(item);
  
  item = MenuItem::create(_("Quit"),
                          boost::bind(&MenuModel::on_menu_quit, this));
  top->add_menu(item);

  core->signal_operation_mode_changed().connect(boost::bind(&MenuModel::on_operation_mode_changed, this, _1)); 
  core->signal_usage_mode_changed().connect(boost::bind(&MenuModel::on_usage_mode_changed, this, _1));
}

void
MenuModel::on_menu_open_main_window()
{
  toolkit->show_window(IToolkit::WindowType::Main);
}

void
MenuModel::on_menu_restbreak_now()
{
  app->restbreak_now();
}

void
MenuModel::on_menu_about()
{
  toolkit->show_window(IToolkit::WindowType::About);
}

void
MenuModel::on_menu_quit()
{
  app->terminate();
}

void
MenuModel::on_menu_preferences()
{
  toolkit->show_window(IToolkit::WindowType::Preferences);
}

void
MenuModel::on_menu_exercises()
{
  toolkit->show_window(IToolkit::WindowType::Exercises);
}

void
MenuModel::on_menu_statistics()
{
  toolkit->show_window(IToolkit::WindowType::Statistics);
}

void
MenuModel::on_menu_normal()
{
  TRACE_ENTER("MenuModel::on_menu_normal");
  set_operation_mode(OperationMode::Normal);
  TRACE_EXIT();
}

void
MenuModel::on_menu_suspend()
{
  TRACE_ENTER("MenuModel::on_menu_suspend");
  set_operation_mode(OperationMode::Suspended);
  TRACE_EXIT();
}

void
MenuModel::on_menu_quiet()
{
  TRACE_ENTER("MenuModel::on_menu_quiet");
  set_operation_mode(OperationMode::Quiet);
  TRACE_EXIT();
}

void
MenuModel::on_menu_reading()
{
  set_usage_mode(reading_item->is_checked() ? UsageMode::Reading : UsageMode::Normal);
}

void
MenuModel::on_menu_reading(bool on)
{
  set_usage_mode(on ? UsageMode::Reading : UsageMode::Normal);
}


void
MenuModel::set_operation_mode(OperationMode m)
{
  core->set_operation_mode(m);
}


void
MenuModel::set_usage_mode(UsageMode m)
{
  core->set_usage_mode(m);
}

void
MenuModel::on_operation_mode_changed(const OperationMode m)
{
  normal_item->set_checked(m == OperationMode::Normal);
  suspended_item->set_checked(m == OperationMode::Suspended);
  quiet_item->set_checked(m == OperationMode::Quiet);
}

void
MenuModel::on_usage_mode_changed(const UsageMode m)
{
  reading_item->set_checked(m == UsageMode::Reading);
}
