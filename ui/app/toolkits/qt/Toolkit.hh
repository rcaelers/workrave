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
#include <map>
#include <boost/signals2.hpp>

#include <QApplication>
#include <QTimer>

#include "AboutDialog.hh"
#include "DebugDialog.hh"
#include "ExercisesDialog.hh"
#include "IToolkitPrivate.hh"
#include "MainWindow.hh"
#include "PreferencesDialog.hh"
#include "StatisticsDialog.hh"
#include "StatusIcon.hh"
#include "ToolkitMenu.hh"

#include "core/CoreTypes.hh"
#include "ui/IApplicationContext.hh"
#include "ui/IToolkit.hh"
#include "utils/Signals.hh"

class Toolkit
  : public QApplication
  , public IToolkit
  , public IToolkitPrivate
{
  Q_OBJECT

public:
  Toolkit(int argc, char **argv);
  ~Toolkit() override = default;

  // IToolkit
  void preinit(std::shared_ptr<workrave::config::IConfigurator> config) override;
  void init(std::shared_ptr<IApplicationContext> app) override;
  void deinit() override;

  void terminate() override;
  void run() override;

  void hold() override;
  void release() override;

  auto get_head_count() const -> int override;

  auto create_break_window(int screen, workrave::BreakId break_id, BreakFlags break_flags) -> IBreakWindow::Ptr override;
  auto create_prelude_window(int screen, workrave::BreakId break_id) -> IPreludeWindow::Ptr override;
  void show_window(WindowType type) override;

  auto get_display_name() const -> const char * override;
  void create_oneshot_timer(int ms, std::function<void()> func) override;
  void show_notification(const std::string &id,
                         const std::string &title,
                         const std::string &balloon,
                         std::function<void()> func) override;
  void show_tooltip(const std::string &tip) override;

  auto signal_timer() -> boost::signals2::signal<void()> & override;
  auto signal_main_window_closed() -> boost::signals2::signal<void()> & override;
  auto signal_session_idle_changed() -> boost::signals2::signal<void(bool)> & override;
  auto signal_session_unlocked() -> boost::signals2::signal<void()> & override;
  auto signal_status_icon_activated() -> boost::signals2::signal<void()> & override;

public Q_SLOTS:
  void on_timer();

private:
  void show_about();
  void show_debug();
  void show_exercises();
  void show_main_window();
  void show_preferences();
  void show_statistics();

  void on_main_window_closed();
  void on_status_icon_balloon_activated(const std::string &id);
  void on_status_icon_activated();

protected:
  void notify_add_confirm_function(const std::string &id, std::function<void()> func);
  void notify_confirm(const std::string &id);

protected:
  std::shared_ptr<IApplicationContext> app;
  MainWindow *main_window{nullptr};

private:
  int argc{};
  char **argv{};
  QPointer<StatisticsDialog> statistics_dialog;
  QPointer<PreferencesDialog> preferences_dialog;
  // QPointer<DebugDialog> debug_dialog;
  QPointer<ExercisesDialog> exercises_dialog;
  QPointer<AboutDialog> about_dialog;
  std::shared_ptr<StatusIcon> status_icon;
  int hold_count{0};

  QTimer *heartbeat_timer{nullptr};

  std::shared_ptr<MenuModel> menu_model;
  std::shared_ptr<SoundTheme> sound_theme;

  std::map<std::string, std::function<void()>> notifiers;

  boost::signals2::signal<void()> timer_signal;
  boost::signals2::signal<void()> main_window_closed_signal;
  boost::signals2::signal<void(bool)> session_idle_changed_signal;
  boost::signals2::signal<void()> session_unlocked_signal;
  boost::signals2::signal<void()> status_icon_activated_signal;
};

class OneshotTimer : public QObject
{
  Q_OBJECT

public:
  OneshotTimer(int ms, std::function<void()> func)
    : func(func)
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
  std::function<void()> func;
};

#endif // TOOLKIT_HH
