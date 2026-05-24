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

#include <algorithm>

#include <QCloseEvent>
#include <QIcon>
#include <QMoveEvent>
#include <QApplication>
#include <QScreen>
#include <QWindow>

#include "debug.hh"
#include "ui/GUIConfig.hh"

#include "ToolkitMenu.hh"
#include "commonui/MenuDefs.hh"
#include "Ui.hh"

MainWindow::MainWindow(std::shared_ptr<IApplicationContext> app, QWidget *parent)
  : QWidget(parent)
  , app(app)
{
  setWindowTitle("Workrave");
  setWindowIcon(QIcon(Ui::get_status_icon_filename(OperationModeIcon::Normal)));
  setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);

  timer_box_view = new TimerBoxView(app->get_core());
  timer_box_control = std::make_shared<TimerBoxControl>(app->get_core(), "main_window", timer_box_view);

  auto *layout = new QVBoxLayout();
  layout->setContentsMargins(2, 2, 2, 2);
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
  setFixedSize(sizeHint());
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

  timer_box_control->update();
  setFixedSize(sizeHint());

  show();
  if (isMinimized())
    {
      showNormal();
    }
  raise();
  activateWindow();
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

  if (head >= app->get_toolkit()->get_head_count())
    {
      head = 0;
    }
  convert_monitor_to_display(x, y, head);

  QScreen *screen = QGuiApplication::screenAt(QPoint(x, y));
  if (screen == nullptr)
    {
      screen = QGuiApplication::primaryScreen();
    }
  if (screen == nullptr)
    {
      return;
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

int
MainWindow::convert_display_to_monitor(int &x, int &y)
{
  const QList<QScreen *> screens = QGuiApplication::screens();
  const QRect frame = frameGeometry();

  for (int i = 0; i < screens.size(); i++)
    {
      QScreen *screen = screens.at(i);
      if (screen == nullptr)
        {
          continue;
        }

      QRect geometry = screen->geometry();
      if (x >= geometry.left() && y >= geometry.top() && x < geometry.left() + geometry.width()
          && y < geometry.top() + geometry.height())
        {
          if (x - geometry.left() >= geometry.width() / 2)
            {
              const int frame_right = x + frame.width();
              const int screen_right = geometry.left() + geometry.width();
              x = frame_right - screen_right - 1;
            }
          else
            {
              x -= geometry.left();
            }

          if (y - geometry.top() >= geometry.height() / 2)
            {
              const int frame_bottom = y + frame.height();
              const int screen_bottom = geometry.top() + geometry.height();
              y = frame_bottom - screen_bottom - 1;
            }
          else
            {
              y -= geometry.top();
            }
          return i;
        }
    }

  x = y = 100;
  return 0;
}

void
MainWindow::convert_monitor_to_display(int &x, int &y, int head)
{
  const QList<QScreen *> screens = QGuiApplication::screens();
  if (head < 0 || head >= screens.size() || screens.at(head) == nullptr)
    {
      return;
    }

  QRect geometry = screens.at(head)->geometry();
  const QSize frame_size = frameGeometry().size();

  if (x < 0)
    {
      const int screen_right = geometry.left() + geometry.width();
      x = x <= -frame_size.width() ? screen_right + x : screen_right + x + 1 - frame_size.width();
    }
  else
    {
      x = std::clamp(x, 0, geometry.width());
      x += geometry.left();
    }

  if (y < 0)
    {
      const int screen_bottom = geometry.top() + geometry.height();
      y = y <= -frame_size.height() ? screen_bottom + y : screen_bottom + y + 1 - frame_size.height();
    }
  else
    {
      y = std::clamp(y, 0, geometry.height());
      y += geometry.top();
    }
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
      int x = frameGeometry().x();
      int y = frameGeometry().y();
      int screen_index = convert_display_to_monitor(x, y);

      GUIConfig::main_window_x().set(x);
      GUIConfig::main_window_y().set(y);
      GUIConfig::main_window_head().set(screen_index);
    }
  QWidget::moveEvent(event);
}
