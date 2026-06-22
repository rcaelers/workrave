// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#include "ToolkitMacOS.hh"
#include "commonui/MenuDefs.hh"
#include "MacOSDesktopWindow.hh"

#include <QMenuBar>

// TODO:

// #if defined(PLATFORM_OS_MACOS)
// #  include "MacOSAppletWindow.hh"
// #endif

ToolkitMacOS::ToolkitMacOS(int argc, char **argv)
  : Toolkit(argc, argv)
{
  locker = std::make_shared<MacOSLocker>();
}

void
ToolkitMacOS::init(std::shared_ptr<IApplicationContext> app)
{
  Toolkit::init(app);

  dock_menu = std::make_shared<ToolkitMenu>(app->get_menu_model(),
                                            [](menus::Node::Ptr menu) { return menu->get_id() != MenuId::OPEN; });
  dock_menu->get_menu()->setAsDockMenu();

  // Native macOS menu bar.  QMenuBar with nullptr parent becomes the global
  // system menu bar on macOS.  Qt automatically promotes actions whose text
  // matches "About", "Preferences", or "Quit" into the "Workrave" app menu
  // at the top-left, so they appear there without any extra work.
  menu_bar = new QMenuBar(nullptr);
  menu_bar_menu = std::make_shared<ToolkitMenu>(app->get_menu_model());
  menu_bar_menu->get_menu()->setTitle(QObject::tr("Workrave"));
  menu_bar->addMenu(menu_bar_menu->get_menu());
}

auto
ToolkitMacOS::get_locker() -> std::shared_ptr<Locker>
{
  return locker;
}

auto
ToolkitMacOS::get_desktop_image() -> QPixmap
{
  return MacOSDesktopWindow::get_desktop_image();
}
