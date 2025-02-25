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

#ifndef WORKRAVE_UI_IPLUGINCONTEXT_HH
#define WORKRAVE_UI_IPLUGINCONTEXT_HH

#include <memory>
#include <boost/signals2.hpp>

#include "config/IConfigurator.hh"
#include "ui/SoundTheme.hh"
#include "commonui/MenuModel.hh"
#include "ui/IToolkit.hh"
#include "ui/IPreferencesRegistry.hh"

class IPluginContext
{
public:
  using Ptr = std::shared_ptr<IPluginContext>;

  virtual ~IPluginContext() = default;

  virtual workrave::ICore::Ptr get_core() const = 0;
  virtual workrave::config::IConfigurator::Ptr get_configurator() const = 0;
  virtual IToolkit::Ptr get_toolkit() const = 0;
  virtual SoundTheme::Ptr get_sound_theme() const = 0;
  virtual MenuModel::Ptr get_menu_model() const = 0;
  virtual IPreferencesRegistry::Ptr get_preferences_registry() const = 0;
};

#endif // WORKRAVE_UI_IPLUGINCONTEXT_HH
