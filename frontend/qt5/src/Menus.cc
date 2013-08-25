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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "Menus.hh"
#include "CoreTypes.hh"


using namespace std;
using namespace workrave;

Menus::Ptr
Menus::create(ICore::Ptr core)
{
  return Ptr(new Menus(core));
}


Menus::Menus(ICore::Ptr core)
  : core(core)
{
  top = MenuItem::create();
  init();
}


Menus::~Menus()
{
}


const MenuItem::Ptr
Menus::get_top() const
{
  return top;
}


void
Menus::init()
{
  workrave::OperationMode mode = workrave::OPERATION_MODE_NORMAL;
  workrave::UsageMode usage = workrave::USAGE_MODE_READING;

  MenuItem::Ptr item;
  
  item = MenuItem::create(_("Open"),
                          boost::bind(&Menus::on_menu_open_main_window, this));
  top->add_menu(item);
  
  item = MenuItem::create(_("Preferences"),
                          boost::bind(&Menus::on_menu_preferences, this));
  top->add_menu(item);
  
  item = MenuItem::create(_("Rest break"),
                          boost::bind(&Menus::on_menu_restbreak_now, this));
  top->add_menu(item);
  
  item = MenuItem::create(_("Exercises"),
                          boost::bind(&Menus::on_menu_exercises, this));
  top->add_menu(item);
  
  MenuItem::Ptr modemenu = MenuItem::create(_("Mode"), 0, MenuItemFlag::SUBMENU);
  top->add_menu(modemenu);
  
  item = MenuItem::create(_("Normal"),
                          boost::bind(&Menus::on_menu_normal, this),
                          MenuItemFlag::RADIO
                          | (mode == workrave::OPERATION_MODE_NORMAL ? MenuItemFlag::ACTIVE : MenuItemFlag::NONE));
  modemenu->add_menu(item);
  
  item = MenuItem::create(_("Suspended"),
                          boost::bind(&Menus::on_menu_suspend, this),
                          MenuItemFlag::RADIO
                          | (mode == workrave::OPERATION_MODE_SUSPENDED ? MenuItemFlag::ACTIVE : MenuItemFlag::NONE));
  modemenu->add_menu(item);
  
  item = MenuItem::create(_("Quiet"),
                          boost::bind(&Menus::on_menu_quiet, this),
                          MenuItemFlag::RADIO
                          | (mode == workrave::OPERATION_MODE_QUIET ? MenuItemFlag::ACTIVE : MenuItemFlag::NONE));
  modemenu->add_menu(item);

  item = MenuItem::create(_("Reading mode"),
                          boost::bind(&Menus::on_menu_reading, this),
                          MenuItemFlag::CHECK
                          | (usage == workrave::USAGE_MODE_READING ? MenuItemFlag::ACTIVE : MenuItemFlag::NONE));
  modemenu->add_menu(item);
  
  item = MenuItem::create(_("Statistics"),
                          boost::bind(&Menus::on_menu_statistics, this));
  top->add_menu(item);
  
  item = MenuItem::create(_("About..."),
                          boost::bind(&Menus::on_menu_about, this));
  top->add_menu(item);
  
  item = MenuItem::create(_("_Quit"),
                          boost::bind(&Menus::on_menu_quit, this));
  top->add_menu(item);
}

void
Menus::on_menu_open_main_window()
{
}

void
Menus::on_menu_restbreak_now()
{
}

void
Menus::on_menu_about()
{
}

void
Menus::on_menu_quit()
{
}

void
Menus::on_menu_preferences()
{
}

void
Menus::on_menu_exercises()
{
}

void
Menus::on_menu_statistics()
{
}

void
Menus::on_menu_normal()
{
  TRACE_ENTER("Menus::on_menu_normal");
  set_operation_mode(OPERATION_MODE_NORMAL);
  TRACE_EXIT();
}

void
Menus::on_menu_suspend()
{
  TRACE_ENTER("Menus::on_menu_suspend");
  set_operation_mode(OPERATION_MODE_SUSPENDED);
  TRACE_EXIT();
}

void
Menus::on_menu_quiet()
{
  TRACE_ENTER("Menus::on_menu_quiet");
  set_operation_mode(OPERATION_MODE_QUIET);
  TRACE_EXIT();
}

void
Menus::on_menu_reading()
{
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
