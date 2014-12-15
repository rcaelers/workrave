// RestBreakWindow.hh --- window for the microbreak
//
// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#ifndef RESTBREAKWINDOW_HH
#define RESTBREAKWINDOW_HH

#include "utils/ScopedConnections.hh"

#include "BreakWindow.hh"
#include "GUIConfig.hh"

#include "TimeBar.hh"

class RestBreakWindow : public BreakWindow
{
  Q_OBJECT

public:
  RestBreakWindow(int screen, BreakFlags break_flags, GUIConfig::BlockMode mode);
  virtual ~RestBreakWindow();

  static IBreakWindow::Ptr create(int screen, BreakFlags break_flags, GUIConfig::BlockMode mode);

  void start();
  void set_progress(int value, int max_value);

private:
  virtual QWidget *create_gui();
  virtual void update_break_window();

  void draw_time_bar();
  void suspend_break();
  QHBoxLayout *create_info_panel();
  void set_ignore_activity(bool i);

  void install_exercises_panel();
  void install_info_panel();
  void clear_pluggable_panel();
  int get_exercise_count();

private:
  //! The Time
  TimeBar *timebar;

  //! Progress
  int progress_value;

  //! Progress
  int progress_max_value;

  QHBoxLayout *pluggable_panel;

  scoped_connections connections;
};

#endif // RESTBREAKWINDOW_HH
