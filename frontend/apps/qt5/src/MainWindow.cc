// Copyright (C) 2006, 2007, 2013 Raymond Penners & Rob Caelers
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

#include "MainWindow.hh"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ToolkitMenu.hh"


MainWindow::MainWindow(MenuModel::Ptr menu_model)
{
  setFixedSize(minimumSize());
  setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::CustomizeWindowHint);

  timer_box_control = new TimerBoxControl("main_window", *this);

  menu = ToolkitMenu::create(menu_model);

  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(on_show_contextmenu(const QPoint&)));
}


MainWindow::~MainWindow()
{
  delete timer_box_control;
}

void
MainWindow::heartbeat()
{
  timer_box_control->update();
}

void
MainWindow::on_show_contextmenu(const QPoint &pos)
{
  QPoint globalPos = mapToGlobal(pos);
  menu->get_menu()->popup(globalPos);
}
