// Copyright (C) 2001 - 2021 Rob Caelers <robc@krandor.nl>
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

#ifndef TOOLKIT_HH
#define TOOLKIT_HH

#include <memory>
#include <boost/signals2.hpp>

#include "core/CoreTypes.hh"
#include "utils/Signals.hh"

#include "IToolkit.hh"

//#include "MainWindow.hh"
#include "PreferencesDialog.hh"
#include "StatisticsDialog.hh"
#include "ExercisesDialog.hh"
#include "DebugDialog.hh"
//#include "StatusIcon.hh"
#include "HeadInfo.hh"

#ifdef PLATFORM_OS_MACOS
#  include "Dock.hh"
#endif

class Toolkit : public IToolkit
{
public:
  Toolkit(int argc, char **argv);
  ~Toolkit() override = default;

  // IToolkit
  void init(MenuModel::Ptr menu_model, SoundTheme::Ptr sound_theme) override;

  HeadInfo get_head_info(int screen_index) const override;
  int get_head_count() const override;

  IBreakWindow::Ptr create_break_window(int screen, workrave::BreakId break_id, BreakFlags break_flags) override;
  IPreludeWindow::Ptr create_prelude_window(int screen, workrave::BreakId break_id) override;
  void show_window(WindowType type) override;

private:
  void show_about();
  void show_debug();
  void show_exercises();
  void show_main_window();
  void show_preferences();
  void show_statistics();

private:
  // MainWindow *main_window{nullptr};
  Gtk::AboutDialog *about_dialog{nullptr};
  StatisticsDialog *statistics_dialog{nullptr};
  PreferencesDialog *preferences_dialog{nullptr};
  DebugDialog *debug_dialog{nullptr};
  ExercisesDialog *exercises_dialog{nullptr};

  std::shared_ptr<MenuModel> menu_model;
  std::shared_ptr<SoundTheme> sound_theme;
  //   std::shared_ptr<IToolkitPlatform> platform;

  boost::signals2::signal<void()> timer_signal;
};

#endif // TOOLKIT_HH
