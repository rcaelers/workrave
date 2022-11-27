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

#ifndef PREFERENCES_REGISTRY_HH
#define PREFERENCES_REGISTRY_HH

#include <map>
#include <boost/signals2.hpp>

#include "ui/IPreferencesRegistry.hh"
#include "ui/IPreferencesRegistryInternal.hh"

#include "utils/Signals.hh"

class PreferencesRegistry : public IPreferencesRegistryInternal
{
public:
  using Ptr = std::shared_ptr<PreferencesRegistry>;

  PreferencesRegistry() = default;
  ~PreferencesRegistry() override = default;

  void add(std::shared_ptr<ui::prefwidgets::Def> def) override;
  void remove(std::shared_ptr<ui::prefwidgets::Def> def) override;
  void add_page(const std::string &id, const std::string &label, const std::string &image) override;

  std::list<std::shared_ptr<ui::prefwidgets::Def>> get_widgets(const std::string &location) const override;
  std::map<std::string, std::list<std::shared_ptr<ui::prefwidgets::Def>>> get_widgets() const override;
  std::map<std::string, std::pair<std::string, std::string>> get_pages() const override;

private:
  std::map<std::string, std::list<std::shared_ptr<ui::prefwidgets::Def>>> widgets;
  std::map<std::string, std::pair<std::string, std::string>> pages;
};

#endif // PREFERENCES_REGISTRY_HH
