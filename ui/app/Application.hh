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

#ifndef APPLICATION_HH
#define APPLICATION_HH

#include <vector>
#include <string>
#include <memory>

#if defined(HAVE_QT)
#  include <QCoreApplication>
#endif

#include "Menus.hh"
#include "core/IApp.hh"
#include "core/ICore.hh"
#include "config/IConfigurator.hh"
#include "ui/IBreakWindow.hh"
#include "ui/IPreludeWindow.hh"
#include "ui/IToolkitFactory.hh"
#include "ui/IToolkit.hh"
#include "ui/SoundTheme.hh"
#include "commonui/Exercise.hh"
#include "utils/Signals.hh"
#include "utils/Logging.hh"

#include "PreferencesRegistry.hh"
#include "Context.hh"

class Application
  : public workrave::IApp
#if !defined(HAVE_CORE_NEXT)
  , public workrave::ICoreEventListener
#endif
  , public workrave::utils::Trackable
{
public:
  Application(int argc, char **argv, std::shared_ptr<IToolkitFactory> toolkit_factory);
  ~Application() override;

  auto get_sound_theme() const -> SoundTheme::Ptr;
  auto get_menu_model() const -> MenuModel::Ptr;
  auto get_core() const -> workrave::ICore::Ptr;
  auto get_configurator() const -> workrave::config::IConfigurator::Ptr;
  auto get_toolkit() const -> IToolkit::Ptr;
  auto get_preferences_registry() const -> IPreferencesRegistry::Ptr;
  auto get_internal_preferences_registry() const -> IPreferencesRegistryInternal::Ptr;

  void main();

  virtual void init_platform_pre() = 0;
  virtual void init_platform_post() = 0;

  // IApp methods
  void create_prelude_window(workrave::BreakId break_id) override;
  void create_break_window(workrave::BreakId break_id, workrave::utils::Flags<workrave::BreakHint> break_hint) override;
  void hide_break_window() override;
  void show_break_window() override;
  void refresh_break_window() override;
  void set_break_progress(int value, int max_value) override;
  void set_prelude_stage(PreludeStage stage) override;
  void set_prelude_progress_text(PreludeProgressText text) override;

#if !defined(HAVE_CORE_NEXT)
  void core_event_notify(workrave::CoreEvent event) override;
#endif

private:
  auto get_timers_tooltip() -> std::string;
  void on_timer();
  void init_configurator();
  void init_args();
  void init_nls();
  void init_core();
  void init_sound_player();
  void init_exercises();
  void init_dbus();
  void init_operation_mode_warning();

  void on_status_icon_activate();
  void on_main_window_closed();
  void on_idle_changed(bool new_idle);
  void on_operation_mode_warning_timer();

#if defined(HAVE_CORE_NEXT)
  void on_break_event(workrave::BreakId break_id, workrave::BreakEvent event);
#endif

private:
  std::shared_ptr<IToolkitFactory> toolkit_factory;
  std::shared_ptr<IToolkit> toolkit;
  std::shared_ptr<workrave::ICore> core;
  std::shared_ptr<Menus> menus;
  std::shared_ptr<MenuModel> menu_model;
  std::shared_ptr<SoundTheme> sound_theme;
  std::shared_ptr<ExerciseCollection> exercises;
  std::shared_ptr<PreferencesRegistry> preferences_registry;
  std::shared_ptr<Context> context;
  std::shared_ptr<workrave::config::IConfigurator> configurator;

  int argc{0};
  char **argv{nullptr};
  std::vector<IBreakWindow::Ptr> break_windows;
  std::vector<IPreludeWindow::Ptr> prelude_windows;
  workrave::BreakId active_break_id{workrave::BREAK_ID_NONE};
  bool muted{false};
  bool closewarn_shown{false};
  bool is_idle{false};
  bool taking{false};
  std::shared_ptr<spdlog::logger> logger{workrave::utils::Logging::create("updater")};
};

inline auto
Application::get_sound_theme() const -> SoundTheme::Ptr
{
  return sound_theme;
}

inline auto
Application::get_menu_model() const -> MenuModel::Ptr
{
  return menu_model;
}

inline auto
Application::get_core() const -> workrave::ICore::Ptr
{
  return core;
}

inline auto
Application::get_configurator() const -> workrave::config::IConfigurator::Ptr
{
  return configurator;
}

inline auto
Application::get_toolkit() const -> IToolkit::Ptr
{
  return toolkit;
}

inline auto
Application::get_preferences_registry() const -> IPreferencesRegistry::Ptr
{
  return preferences_registry;
}

inline auto
Application::get_internal_preferences_registry() const -> IPreferencesRegistryInternal::Ptr
{
  return preferences_registry;
}

#endif // APPLICATION_HH
