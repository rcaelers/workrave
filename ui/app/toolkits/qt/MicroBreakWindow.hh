// Copyright (C) 2001 - 2013 Raymond Penners & Rob Caelers
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

#ifndef MICROBREAKWINDOW_HH
#define MICROBREAKWINDOW_HH

#include "BreakWindow.hh"
#include "TimeBar.hh"

class MicroBreakWindow : public BreakWindow
{
  Q_OBJECT

public:
  MicroBreakWindow(std::shared_ptr<IApplicationContext> app, QScreen *screen, BreakFlags break_flags);

  void set_progress(int value, int max_value) override;

protected:
  auto create_gui() -> QWidget * override;

private:
  void update_break_window() override;
  void update_time_bar();
  void update_label();

  auto create_restbreaknow_button(bool label) -> QAbstractButton *;
  void on_restbreaknow_button_clicked();

private:
  std::shared_ptr<IApplicationContext> app;
  TimeBar *time_bar{nullptr};
  QLabel *label{nullptr};
};

#endif // MICROBREAKWINDOW_HH
