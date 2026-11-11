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

#include <spdlog/spdlog.h>

#include "debug.hh"

PreferencesRegistry::~PreferencesRegistry()
{
  TRACE_ENTRY();
}

void
PreferencesRegistry::add(std::shared_ptr<ui::prefwidgets::PanelDef> def)
{
  if (def == nullptr)
    {
      spdlog::error("Cannot register a null preferences panel");
      return;
    }
  auto id = def->get_id();
  widgets[id].push_back(def);
}

IPreferencesRegistry::Section
PreferencesRegistry::find(const std::string &location)
{
  auto page_end = location.find('.');
  if (page_end == std::string::npos || page_end == 0)
    {
      spdlog::error("Invalid preferences section '{}': expected page.panel[.top|.bottom]", location);
      return {};
    }
  auto panel_end = location.find('.', page_end + 1);
  auto page = location.substr(0, page_end);
  auto panel = location.substr(page_end + 1, panel_end == std::string::npos ? panel_end : panel_end - page_end - 1);
  if (panel.empty())
    {
      spdlog::error("Invalid preferences section '{}': panel is missing", location);
      return {};
    }
  auto placement = Placement::Bottom;
  if (panel_end != std::string::npos)
    {
      auto position = location.substr(panel_end + 1);
      if (position == "top")
        {
          placement = Placement::Top;
        }
      else if (position != "bottom")
        {
          spdlog::error("Invalid preferences section '{}': unknown insertion position '{}'", location, position);
          return {};
        }
    }
  auto &extensions = panel_extensions[page + "." + panel + "."];
  if (extensions == nullptr)
    {
      extensions = std::make_shared<std::list<PanelExtension>>();
    }
  return Section([extensions, page, panel, placement](auto def) {
    if (def == nullptr)
      {
        spdlog::error("Cannot add null preferences to panel '{}.{}'", page, panel);
        return;
      }
    extensions->push_back({std::move(def), placement, page, panel});
  });
}

void
PreferencesRegistry::remove(std::shared_ptr<ui::prefwidgets::Def> def)
{
  if (def == nullptr)
    {
      spdlog::error("Cannot remove null preferences");
      return;
    }
  auto id = def->get_id();
  if (widgets.contains(id))
    {
      widgets[id].remove(def);
    }
  // A definition may be inserted into locations unrelated to its own IDs.
  for (auto &[location, extensions]: panel_extensions)
    {
      extensions->remove_if([&def](const auto &extension) { return extension.def == def; });
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

std::map<std::string, std::list<IPreferencesRegistryInternal::PanelExtension>>
PreferencesRegistry::get_panel_extensions() const
{
  std::map<std::string, std::list<PanelExtension>> result;
  for (const auto &[location, extensions]: panel_extensions)
    {
      result.emplace(location, *extensions);
    }
  return result;
}
