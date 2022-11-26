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

#ifndef WORKRAVE_UI_IPREFERENCES_REGISTRY_INTERNAL_HH
#define WORKRAVE_UI_IPREFERENCES_REGISTRY_INTERNAL_HH

#include <memory>

#include "ui/prefwidgets/Widget.hh"
#include "ui/IPreferencesRegistry.hh"

class IPreferencesRegistryInternal : public IPreferencesRegistry
{
public:
  using Ptr = std::shared_ptr<IPreferencesRegistryInternal>;

  ~IPreferencesRegistryInternal() override = default;

  virtual std::list<std::shared_ptr<ui::prefwidgets::Def>> get_widgets(const std::string &location) const = 0;
  virtual std::map<std::string, std::list<std::shared_ptr<ui::prefwidgets::Def>>> get_widgets() const = 0;
  virtual std::map<std::string, std::pair<std::string, std::string>> get_pages() const = 0;
};

#endif // WORKRAVE_UI_IPREFERENCES_REGISTRY_INTERNAL_HH
