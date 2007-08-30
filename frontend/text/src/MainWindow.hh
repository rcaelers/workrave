// MainWindow.hh --- Main info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include "preinclude.h"

#include <string>

class GUI;
class TimerBoxControl;
class TimerBoxTextView;

using namespace std;

#include "ConfiguratorListener.hh"

class MainWindow :
  public ConfiguratorListener
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
  void config_changed_notify(std::string key);

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
