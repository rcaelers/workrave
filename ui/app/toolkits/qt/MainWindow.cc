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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "MainWindow.hh"

#include <QCloseEvent>
#include <QMoveEvent>
#include <QApplication>
#include <QScreen>
#include <QWindow>

#include "debug.hh"
#include "ui/GUIConfig.hh"

#include "ToolkitMenu.hh"
#include "commonui/MenuDefs.hh"

MainWindow::MainWindow(std::shared_ptr<IApplicationContext> app, QWidget *parent)
  : QWidget(parent)
  , app(app)
{
  setFixedSize(minimumSize());
  setWindowFlags(Qt::Tool);

  timer_box_view = new TimerBoxView;
  timer_box_control = std::make_shared<TimerBoxControl>(app->get_core(), "main_window", timer_box_view);

  auto *layout = new QVBoxLayout();
  layout->setContentsMargins(1, 1, 1, 1);
  setLayout(layout);

  layout->addWidget(timer_box_view);

  menu = std::make_shared<ToolkitMenu>(app->get_menu_model(),
                                       [](menus::Node::Ptr menu) { return menu->get_id() != MenuId::OPEN; });

  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(on_show_contextmenu(const QPoint &)));

  GUIConfig::main_window_always_on_top().attach(this, [&](bool enabled) {
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

  GUIConfig::key_timerbox("main_window").connect(this, [this]() { on_enabled_changed(); });
  timer_box_control->update();
  enabled = GUIConfig::timerbox_enabled("main_window")();
  move_to_start_position();

  if (enabled)
    {
      open_window();
    }
}

void
MainWindow::open_window()
{
  TRACE_ENTRY();
  if (app->get_toolkit()->get_head_count() <= 0)
    {
      return;
    }

  show();
  raise();
  move_to_start_position();
  GUIConfig::timerbox_enabled("main_window").set(true);
}

void
MainWindow::close_window()
{
  TRACE_ENTRY();
  if (can_close)
    {
      hide();
    }
  else
    {
      showMinimized();
    }

  GUIConfig::timerbox_enabled("main_window").set(false);
}

void
MainWindow::set_can_close(bool can_close)
{
  TRACE_ENTRY_PAR(can_close);
  this->can_close = can_close;

  if (!can_close && !isVisible())
    {
      open_window();
    }
}

void
MainWindow::on_enabled_changed()
{
  TRACE_ENTRY();
  bool new_enabled = GUIConfig::timerbox_enabled("main_window")();

  if (enabled != new_enabled)
    {
      enabled = new_enabled;
      if (enabled)
        {
          open_window();
        }
      else
        {
          close_window();
        }
    }
}

void
MainWindow::heartbeat()
{
  timer_box_control->update();
}

auto
MainWindow::signal_closed() -> boost::signals2::signal<void()> &
{
  return closed_signal;
}

void
MainWindow::move_to_start_position()
{
  TRACE_ENTRY();
  int x = GUIConfig::main_window_x()();
  int y = GUIConfig::main_window_y()();
  int head = GUIConfig::main_window_head()();

  QList<QScreen *> screens = QGuiApplication::screens();
  QScreen *screen = nullptr;

  if (head < 0 || head >= screens.size())
    {
      screen = QGuiApplication::primaryScreen();
    }
  else
    {
      screen = screens.at(head);
    }

  const QRect availableGeometry = screen->availableGeometry();

  QRect geometry = frameGeometry();
  geometry.moveTo(x, y);

  if (!geometry.intersects(availableGeometry))
    {
      geometry.moveBottom(qMin(geometry.bottom(), availableGeometry.bottom()));
      geometry.moveLeft(qMax(geometry.left(), availableGeometry.left()));
      geometry.moveRight(qMin(geometry.right(), availableGeometry.right()));
    }
  geometry.moveTop(qMax(geometry.top(), availableGeometry.top()));

  TRACE_VAR(x, y, head);
  TRACE_VAR(geometry.x(), geometry.y(), head);

  move(geometry.topLeft());
}

void
MainWindow::on_show_contextmenu(const QPoint &pos)
{
  bool taking = app->get_core()->is_taking();
  if (taking && (GUIConfig::block_mode()() == BlockMode::All || GUIConfig::block_mode()() == BlockMode::Input))
    {
      return;
    }

  app->get_menu_model()->update();
  QPoint globalPos = mapToGlobal(pos);
  menu->get_menu()->popup(globalPos);
}

void
MainWindow::closeEvent(QCloseEvent *event)
{
  TRACE_ENTRY();
  if (can_close)
    {
      close_window();
    }
  closed_signal();
  event->ignore();
}

void
MainWindow::moveEvent(QMoveEvent *event)
{
  if (isVisible())
    {
      GUIConfig::main_window_x().set(frameGeometry().x());
      GUIConfig::main_window_y().set(frameGeometry().y());

      QScreen *screen = window()->windowHandle()->screen();
      auto screen_index = QGuiApplication::screens().indexOf(screen);
      GUIConfig::main_window_head().set(static_cast<int>(screen_index));
    }
  QWidget::moveEvent(event);
}
