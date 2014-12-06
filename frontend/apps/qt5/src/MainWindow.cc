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

#include "debug.hh"

#include <QMoveEvent>
#include <QApplication>
#include <QDesktopWidget>

#include "ToolkitMenu.hh"
#include "Menus.hh"
#include "GUIConfig.hh"

MainWindow::MainWindow(MenuModel::Ptr menu_model)
{
  setFixedSize(minimumSize());
  setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::CustomizeWindowHint);

  timer_box_control = new TimerBoxControl("main_window", *this);

  menu = ToolkitMenu::create(menu_model, [](MenuModel::Ptr menu) { return menu->get_id() != Menus::OPEN; });
  
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(on_show_contextmenu(const QPoint&)));

  GUIConfig::main_window_always_on_top().connect_and_get([&] (bool enabled)
                                                         {
                                                           if (enabled)
                                                             {
                                                               setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
                                                             }
                                                           else
                                                             {
                                                               setWindowFlags(windowFlags() & (~Qt::WindowStaysOnTopHint));
                                                             }
                                                           show();
                                                         });

  move_to_start_position();
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
MainWindow::move_to_start_position()
{
  TRACE_ENTER("MainWindow:move_to_start_positionp");

  int x = GUIConfig::main_window_x()();
  int y = GUIConfig::main_window_y()();
  int head = GUIConfig::main_window_head()();

  const QDesktopWidget * const desktop = QApplication::desktop();
  if (head >= desktop->numScreens())
    {
      head = desktop->primaryScreen();
    }
  else if (head < 0)
    {
      head = 0;
    }
  
  const QRect availableGeometry = desktop->availableGeometry(head);

  QRect geometry = frameGeometry();
  geometry.moveTo(x, y);

  if (!geometry.intersects(availableGeometry)) 
    {
      geometry.moveBottom(qMin(geometry.bottom(), availableGeometry.bottom()));
      geometry.moveLeft(qMax(geometry.left(), availableGeometry.left()));
      geometry.moveRight(qMin(geometry.right(), availableGeometry.right()));
    }
  geometry.moveTop(qMax(geometry.top(), availableGeometry.top()));

  TRACE_MSG(x << " " << y << " " << head);
  TRACE_MSG(geometry.x() << " " << geometry.y() << " " << head);

  move(geometry.topLeft());

  TRACE_EXIT();
}

void
MainWindow::on_show_contextmenu(const QPoint &pos)
{
  QPoint globalPos = mapToGlobal(pos);
  menu->get_menu()->popup(globalPos);
}

void 
MainWindow::moveEvent(QMoveEvent * event)
{
  if (isVisible())
    {
      GUIConfig::main_window_x().set(frameGeometry().x());
      GUIConfig::main_window_y().set(frameGeometry().y());
      GUIConfig::main_window_head().set(QApplication::desktop()->screenNumber(this));
    }
  QWidget::moveEvent(event);
}
