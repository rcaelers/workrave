// Copyright (C) 2011 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UI_IPREFERENCES_REGISTRY_HH
#define WORKRAVE_UI_IPREFERENCES_REGISTRY_HH

#include <memory>

#include "ui/prefwidgets/Def.hh"

class IPreferencesRegistry
{
public:
  using Ptr = std::shared_ptr<IPreferencesRegistry>;

  virtual ~IPreferencesRegistry() = default;

  virtual void add(std::shared_ptr<ui::prefwidgets::Def> def) = 0;
  virtual void remove(std::shared_ptr<ui::prefwidgets::Def> def) = 0;
  virtual void add_page(const std::string &id, const std::string &label, const std::string &image) = 0;
};

#endif // WORKRAVE_UI_IPREFERENCES_REGISTRY_HH
