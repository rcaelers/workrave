// MicroBreakWindow.hh --- window for the microbreak
//
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
#include "GUIConfig.hh"

#include "TimeBar.hh"

class MicroBreakWindow : public BreakWindow
{
  Q_OBJECT

public:
  MicroBreakWindow(int screen, BreakFlags break_flags, GUIConfig::BlockMode mode);
  virtual ~MicroBreakWindow();

  static IBreakWindow::Ptr create(int screen, BreakFlags break_flags, GUIConfig::BlockMode mode);

  virtual void set_progress(int value, int max_value);
  //void heartbeat();

protected:
  virtual QWidget *create_gui();

private:
  virtual void update_break_window();
  void update_time_bar();
  void update_label();

  QAbstractButton *create_restbreaknow_button(bool label);
  void on_restbreaknow_button_clicked();

private:
  //! Time bar
  TimeBar *time_bar;

  // Label
  QLabel *label;

  //! Progress
  int progress_value;

  //! Progress
  int progress_max_value;

  //! Label size has been fixed?
  bool fixed_size;
};



#endif // MICROBREAKWINDOW_HH
