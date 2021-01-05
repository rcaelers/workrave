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

#include "commonui/SoundTheme.hh"
#include "utils/ScopedConnections.hh"

#include "BreakWindow.hh"
#include "TimeBar.hh"

class RestBreakWindow : public BreakWindow
{
  Q_OBJECT

public:
  RestBreakWindow(IToolkitPlatform::Ptr platform, SoundTheme::Ptr sound_theme, QScreen *screen, BreakFlags break_flags);

  void set_progress(int value, int max_value) override;

private:
  QWidget *create_gui() override;
  void update_break_window() override;

  void install_exercises_panel();
  void install_info_panel();
  int get_exercise_count();

private:
  SoundTheme::Ptr sound_theme;
  TimeBar *timebar{nullptr};
  QHBoxLayout *pluggable_panel{nullptr};

  scoped_connections connections;
};

#endif // RESTBREAKWINDOW_HH
