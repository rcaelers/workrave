// Copyright (C) 2001 -2013 Rob Caelers <robc@krandor.nl>
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

#include <QApplication>
#include <QTimer>

#include "core/CoreTypes.hh"
#include "utils/ScopedConnections.hh"

#include "AboutDialog.hh"
#include "ExercisesDialog.hh"
#include "IToolkit.hh"
#include "MainWindow.hh"
#include "Menus.hh"
#include "PreferencesDialog.hh"
#include "StatisticsDialog.hh"
#include "StatusIcon.hh"
#include "ToolkitMenu.hh"
#include "IToolkitPlatform.hh"

#ifdef PLATFORM_OS_MACOS
#include "Dock.hh"
#endif

class Toolkit : public QApplication, public IToolkit
{
  Q_OBJECT

public:
  Toolkit(int argc, char **argv);
  ~Toolkit() override = default;

  boost::signals2::signal<void()> &signal_timer() override;
  void init(MenuModel::Ptr menu_model, SoundTheme::Ptr sound_theme) override;
  void terminate() override;
  void run() override;
  const char *get_display_name() override;
  IBreakWindow::Ptr create_break_window(int screen, workrave::BreakId break_id, BreakFlags break_flags) override;
  IPreludeWindow::Ptr create_prelude_window(int screen, workrave::BreakId break_id) override;
  void show_window(WindowType type) override;
  int get_screen_count() const override;
  void create_oneshot_timer(int ms, std::function<void ()> func) override;
  void show_balloon(const std::string &id, const std::string &title, const std::string &balloon) override;

public Q_SLOTS:
  void on_timer();

private:

private:
  QTimer *heartbeat_timer { nullptr };

  MainWindow *main_window { nullptr };
  std::shared_ptr<StatusIcon> status_icon;

  QPointer<PreferencesDialog> preferences_dialog;
  QPointer<ExercisesDialog> exercises_dialog;
  QPointer<AboutDialog> about_dialog;
  QPointer<StatisticsDialog> statistics_dialog;

#ifdef PLATFORM_OS_MACOS
  std::shared_ptr<Dock> dock;
  std::shared_ptr<ToolkitMenu> dock_menu;
#endif

  std::shared_ptr<MenuModel> menu_model;
  std::shared_ptr<SoundTheme> sound_theme;
  std::shared_ptr<IToolkitPlatform> platform;

  boost::signals2::signal<void()> timer_signal;

  scoped_connections connections;
};

class OneshotTimer : public QObject
{
  Q_OBJECT

public:
  OneshotTimer(int ms, std::function<void ()> func) : func(func)
  {
    QTimer::singleShot(ms, this, SLOT(exec()));
  };

public Q_SLOTS:
  void exec()
  {
    func();
    this->deleteLater();
  };

private:
    std::function<void ()> func;
};

#endif // TOOLKIT_HH
