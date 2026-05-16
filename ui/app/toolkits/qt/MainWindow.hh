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

#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <boost/signals2.hpp>

#include "ui/TimerBoxControl.hh"
#include "ui/IApplicationContext.hh"
#include "utils/Signals.hh"

#include "TimerBoxView.hh"
#include "ToolkitMenu.hh"

class MainWindow
  : public QWidget
  , public workrave::utils::Trackable
{
  Q_OBJECT

public:
  explicit MainWindow(std::shared_ptr<IApplicationContext> app, QWidget *parent = nullptr);

  void set_can_close(bool can_close);

  void open_window();
  void close_window();
  void heartbeat();

  auto signal_closed() -> boost::signals2::signal<void()> &;

  void closeEvent(QCloseEvent *event) override;
  void moveEvent(QMoveEvent *event) override;

public Q_SLOTS:
  void on_show_contextmenu(const QPoint &pos);

private:
  void move_to_start_position();
  void on_enabled_changed();

private:
  std::shared_ptr<IApplicationContext> app;
  std::shared_ptr<ToolkitMenu> menu;
  std::shared_ptr<TimerBoxControl> timer_box_control;
  TimerBoxView *timer_box_view{nullptr};
  bool enabled{false};
  bool can_close{false};
  boost::signals2::signal<void()> closed_signal;
};

#endif // MAINWINDOW_HH
