// Copyright (C) 2001 - 2021 Rob Caelers & Raymond Penners
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

#include "DebugDialog.hh"
#include "ExercisesDialog.hh"
#include "HeadInfo.hh"
#include "IToolkitPrivate.hh"
#include "MainWindow.hh"
#include "PreferencesDialog.hh"
#include "StatisticsDialog.hh"
#include "StatusIcon.hh"

#include "core/CoreTypes.hh"
#include "ui/IApplicationContext.hh"
#include "ui/IToolkit.hh"
#include "utils/Signals.hh"

#if defined(PLATFORM_OS_MACOS)
#  include "ui/macos/MacOSDock.hh"
#endif

class Toolkit
  : public IToolkit
  , public IToolkitPrivate
{
public:
  Toolkit(int argc, char **argv);
  ~Toolkit() override;

  // IToolkit
  void init(std::shared_ptr<IApplicationContext> app) override;
  void deinit() override;

  HeadInfo get_head_info(int screen_index) const override;
  int get_head_count() const override;

  void terminate() override;
  void run() override;

  void hold() override;
  void release() override;

  IBreakWindow::Ptr create_break_window(int screen, workrave::BreakId break_id, BreakFlags break_flags) override;
  IPreludeWindow::Ptr create_prelude_window(int screen, workrave::BreakId break_id) override;
  void show_window(WindowType type) override;

  const char *get_display_name() const override;
  void create_oneshot_timer(int ms, std::function<void()> func) override;
  void show_notification(const std::string &id,
                         const std::string &title,
                         const std::string &balloon,
                         std::function<void()> func) override;
  void show_tooltip(const std::string &tip) override;

  boost::signals2::signal<void()> &signal_timer() override;
  boost::signals2::signal<void()> &signal_main_window_closed() override;
  boost::signals2::signal<void(bool)> &signal_session_idle_changed() override;
  boost::signals2::signal<void()> &signal_session_unlocked() override;
  boost::signals2::signal<void()> &signal_status_icon_activated() override;

  // IToolkitPrivate
  void attach_menu(Gtk::Menu *menu) override;

protected:
  void notify_add_confirm_funcation(const std::string &id, std::function<void()> func);
  void notify_confirm(const std::string &id);

private:
  void show_about();
  void show_debug();
  void show_exercises();
  void show_main_window();
  void show_preferences();
  void show_statistics();

  void init_multihead();
  void init_gui();
  void init_debug();

  bool on_timer();
  void on_main_window_closed();
  void on_status_icon_balloon_activated(const std::string &id);
  void on_status_icon_activated();

protected:
  std::shared_ptr<IApplicationContext> app;
  MainWindow *main_window{nullptr};
  Glib::RefPtr<Gtk::Application> gapp;

private:
  int argc{};
  char **argv{};
  StatisticsDialog *statistics_dialog{nullptr};
  PreferencesDialog *preferences_dialog{nullptr};
  DebugDialog *debug_dialog{nullptr};
  ExercisesDialog *exercises_dialog{nullptr};
  Gtk::AboutDialog *about_dialog{nullptr};
  StatusIcon *status_icon{nullptr};
  int hold_count{0};

  std::shared_ptr<MenuModel> menu_model;
  std::shared_ptr<SoundTheme> sound_theme;

  std::map<std::string, std::function<void()>> notifiers;

  std::list<sigc::connection> event_connections;
  workrave::utils::Trackable tracker;

  boost::signals2::signal<void()> timer_signal;
  boost::signals2::signal<void()> main_window_closed_signal;
  boost::signals2::signal<void(bool)> session_idle_changed_signal;
  boost::signals2::signal<void()> session_unlocked_signal;
  boost::signals2::signal<void()> status_icon_activated_signal;
};

#endif // TOOLKIT_HH
