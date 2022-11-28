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

#ifndef WORKRAVE_UI_IAPPLICATIONCONTEXT_HH
#define WORKRAVE_UI_IAPPLICATIONCONTEXT_HH

#include <memory>
#include <boost/signals2.hpp>

namespace workrave::core
{
  class ICore;
}

#include "ui/SoundTheme.hh"
#include "commonui/MenuModel.hh"
#include "commonui/Exercise.hh"
#include "ui/IToolkit.hh"
#include "ui/IPreferencesRegistryInternal.hh"

class IApplicationContext
{
public:
  using Ptr = std::shared_ptr<IApplicationContext>;

  virtual ~IApplicationContext() = default;

  virtual workrave::ICore::Ptr get_core() const = 0;
  virtual IToolkit::Ptr get_toolkit() const = 0;
  virtual SoundTheme::Ptr get_sound_theme() const = 0;
  virtual ExerciseCollection::Ptr get_exercises() const = 0;
  virtual MenuModel::Ptr get_menu_model() const = 0;
  virtual IPreferencesRegistryInternal::Ptr get_internal_preferences_registry() const = 0;
};

#endif // WORKRAVE_UI_IAPPLICATIONCONTEXT_HH
