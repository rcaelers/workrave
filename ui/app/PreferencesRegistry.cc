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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "PreferencesRegistry.hh"

void
PreferencesRegistry::add(std::shared_ptr<ui::prefwidgets::Def> def)
{
  auto id = def->get_id();
  widgets[id].push_back(def);
}

void
PreferencesRegistry::remove(std::shared_ptr<ui::prefwidgets::Def> def)
{
  auto id = def->get_id();
  if (widgets.contains(id))
    {
      widgets[id].remove(def);
    }
}

std::list<std::shared_ptr<ui::prefwidgets::Def>>
PreferencesRegistry::get_widgets(const std::string &location) const
{
  if (widgets.contains(location))
    {
      return widgets.at(location);
    }

  return {};
}

void
PreferencesRegistry::add_page(const std::string &id, const std::string &label, const std::string &image)
{
  pages[id] = std::make_pair(label, image);
}

std::map<std::string, std::pair<std::string, std::string>>
PreferencesRegistry::get_pages() const
{
  return pages;
}

std::map<std::string, std::list<std::shared_ptr<ui::prefwidgets::Def>>>
PreferencesRegistry::get_widgets() const
{
  return widgets;
}
