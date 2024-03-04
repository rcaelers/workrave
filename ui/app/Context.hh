// Copyright (C) 2022 Rob Caelers <robc@krandor.nl>
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

#ifndef PLUGIN_CONTEXT_HH
#define PLUGIN_CONTEXT_HH

#include <utility>

#include "PreferencesRegistry.hh"

#include "ui/IPluginContext.hh"
#include "ui/IApplicationContext.hh"

#include "debug.hh"

class Context
  : public IApplicationContext
  , public IPluginContext
{
public:
  Context()
  {
    TRACE_ENTRY();
  }
  ~Context() override
  {
    TRACE_ENTRY();
  };

  auto get_core() const -> workrave::ICore::Ptr override;
  auto get_configurator() const -> workrave::config::IConfigurator::Ptr override;
  auto get_toolkit() const -> IToolkit::Ptr override;
  auto get_sound_theme() const -> SoundTheme::Ptr override;
  auto get_exercises() const -> ExerciseCollection::Ptr override;
  auto get_menu_model() const -> MenuModel::Ptr override;
  auto get_preferences_registry() const -> IPreferencesRegistry::Ptr override;
  auto get_internal_preferences_registry() const -> IPreferencesRegistryInternal::Ptr override;
  // virtual IPreferencesRegistryInternal::Ptr get_internal_preferences_registry() const;

  void set_core(workrave::ICore::Ptr core);
  void set_configurator(workrave::config::IConfigurator::Ptr configurator);
  void set_toolkit(IToolkit::Ptr toolkit);
  void set_sound_theme(SoundTheme::Ptr sound_theme);
  void set_exercises(ExerciseCollection::Ptr exercises);
  void set_menu_model(MenuModel::Ptr menu_model);
  void set_preferences_registry(PreferencesRegistry::Ptr preferences_registry);

private:
  std::shared_ptr<workrave::ICore> core;
  workrave::config::IConfigurator::Ptr configurator;
  std::weak_ptr<IToolkit> toolkit;
  std::shared_ptr<MenuModel> menu_model;
  std::shared_ptr<SoundTheme> sound_theme;
  ExerciseCollection::Ptr exercises;
  std::shared_ptr<PreferencesRegistry> preferences_registry;
};

inline auto
Context::get_sound_theme() const -> SoundTheme::Ptr
{
  return sound_theme;
}

inline auto
Context::get_exercises() const -> ExerciseCollection::Ptr
{
  return exercises;
}

inline auto
Context::get_menu_model() const -> MenuModel::Ptr
{
  return menu_model;
}

inline auto
Context::get_core() const -> workrave::ICore::Ptr
{
  return core;
}

inline auto
Context::get_configurator() const -> workrave::config::IConfigurator::Ptr
{
  return configurator;
}

inline auto
Context::get_toolkit() const -> IToolkit::Ptr
{
  return toolkit.lock();
}

inline auto
Context::get_preferences_registry() const -> IPreferencesRegistry::Ptr
{
  return preferences_registry;
}

inline auto
Context::get_internal_preferences_registry() const -> IPreferencesRegistryInternal::Ptr
{
  return preferences_registry;
}

inline void
Context::set_core(workrave::ICore::Ptr core)
{
  this->core = std::move(core);
}

inline void
Context::set_configurator(workrave::config::IConfigurator::Ptr configurator)
{
  this->configurator = std::move(configurator);
}

inline void
Context::set_toolkit(IToolkit::Ptr toolkit)
{
  this->toolkit = std::move(toolkit);
}

inline void
Context::set_sound_theme(SoundTheme::Ptr sound_theme)
{
  this->sound_theme = std::move(sound_theme);
}

inline void
Context::set_exercises(ExerciseCollection::Ptr exercises)
{
  this->exercises = std::move(exercises);
}

inline void
Context::set_menu_model(MenuModel::Ptr menu_model)
{
  this->menu_model = std::move(menu_model);
}

inline void
Context::set_preferences_registry(PreferencesRegistry::Ptr preferences_registry)
{
  this->preferences_registry = std::move(preferences_registry);
}

#endif // PLUGIN_CONTEXT_HH
