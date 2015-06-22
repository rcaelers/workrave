// Tookit.hh
//
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

#include "utils/ScopedConnections.hh"

#include <QApplication>
#include <QTimer>

#include "IToolkit.hh"
#include "CoreTypes.hh"
#include "Menus.hh"

#include "MainWindow.hh"
#include "PreferencesDialog.hh"
#include "ExercisesDialog.hh"
#include "AboutDialog.hh"

#include "StatusIcon.hh"
#include "ToolkitMenu.hh"

#ifdef PLATFORM_OS_OSX
#include "Dock.hh"
#endif

class Toolkit : public QApplication, public IToolkit
{
  Q_OBJECT

public:

  typedef std::shared_ptr<Toolkit> Ptr;

  Toolkit(int argc, char **argv);
  ~Toolkit() override;

  boost::signals2::signal<void()> &signal_timer() override;

  void init(MenuModel::Ptr menu_model, SoundTheme::Ptr sound_theme) override;
  void terminate() override;
  void run() override;
  void grab() override;
  void ungrab() override;
  std::string get_display_name() override;
  IBreakWindow::Ptr create_break_window(int screen, workrave::BreakId break_id, BreakFlags break_flags) override;
  IPreludeWindow::Ptr create_prelude_window(int screen, workrave::BreakId break_id) override;
  int get_screen_count() const override;
  void show_window(WindowType type) override;
  void hide_window(WindowType type) override;
  void create_oneshot_timer(int ms, std::function<void ()> func) override;
  void show_balloon(std::string id, const std::string& title, const std::string& balloon) override;

public slots:
  void on_timer();
  void on_exercises_closed();
  void on_preferences_closed();
  void on_about_closed();

private:
  std::shared_ptr<QTimer> heartbeat_timer;

  std::shared_ptr<MainWindow> main_window;
  std::shared_ptr<PreferencesDialog> preferences_dialog;
  std::shared_ptr<ExercisesDialog> exercises_dialog;
  std::shared_ptr<AboutDialog> about_dialog;

  std::shared_ptr<StatusIcon> status_icon;

#ifdef PLATFORM_OS_OSX
  std::shared_ptr<Dock> dock;
  ToolkitMenu::Ptr dock_menu;
#endif
  MenuModel::Ptr menu_model;
  SoundTheme::Ptr sound_theme;

  //! Timer signal.
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

public slots:
  void exec()
  {
    func();
    this->deleteLater();
  };

private:
    std::function<void ()> func;
};

#endif // TOOLKIT_HH
