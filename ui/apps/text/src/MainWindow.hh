// MainWindow.hh --- Main info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2007 Rob Caelers & Raymond Penners
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

#include "preinclude.h"

#include <string>

#include "config/IConfiguratorListener.hh"

class GUI;
class TimerBoxControl;
class TimerBoxTextView;

using namespace workrave;

class MainWindow : public workrave::config::IConfiguratorListener
{
public:
  MainWindow();
  virtual ~MainWindow();

  static bool get_always_on_top();
  static void set_always_on_top(bool b);

  void update();

private:
  //! Window enabled
  bool enabled;

  //! Table containing all timer information
  TimerBoxControl *timer_box_control;

  //! Table containing all timer information
  TimerBoxTextView *timer_box_view;

  //! Is the monitoring function suspended?
  bool monitor_suspended;

private:
  //
  void init();
  void setup();
  void config_changed_notify(const std::string &key);

public:
  static void set_start_in_tray(bool b);
  static bool get_start_in_tray();

  static void get_start_position(int &x, int &y, int &head);
  static void set_start_position(int x, int y, int head);

  static const std::string CFG_KEY_MAIN_WINDOW;
  static const std::string CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP;
  static const std::string CFG_KEY_MAIN_WINDOW_START_IN_TRAY;
  static const std::string CFG_KEY_MAIN_WINDOW_X;
  static const std::string CFG_KEY_MAIN_WINDOW_Y;
  static const std::string CFG_KEY_MAIN_WINDOW_HEAD;
};

#endif // MAINWINDOW_HH
