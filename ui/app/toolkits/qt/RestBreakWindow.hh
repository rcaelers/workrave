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

#include "ui/SoundTheme.hh"
#include "utils/Signals.hh"

#include "BreakWindow.hh"
#include "TimeBar.hh"

class RestBreakWindow
  : public BreakWindow
  , public workrave::utils::Trackable
{
  Q_OBJECT

public:
  RestBreakWindow(std::shared_ptr<IApplicationContext> app, QScreen *screen, BreakFlags break_flags);

  void set_progress(int value, int max_value) override;

private:
  auto create_gui() -> QWidget * override;
  void update_break_window() override;

  void install_exercises_panel();
  void install_info_panel();
  auto get_exercise_count() -> int;

private:
  std::shared_ptr<IApplicationContext> app;
  SoundTheme::Ptr sound_theme;
  TimeBar *timebar{nullptr};
  QHBoxLayout *pluggable_panel{nullptr};
};

#endif // RESTBREAKWINDOW_HH
