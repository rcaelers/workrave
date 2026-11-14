// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Rob Caelers

#include "PreferencesRegistry.hh"

#include <algorithm>
#include <iostream>
#include <cstdio>
#include <exception>

using ui::prefwidgets::Def;
using ui::prefwidgets::PanelDef;
using Placement = IPreferencesRegistryInternal::Placement;

template<class T>
concept HasPlacement = requires(T value) { value.get_placement(); };

static_assert(!HasPlacement<Def>);
static_assert(!HasPlacement<PanelDef>);

int
main()
try
  {
    PreferencesRegistry registry;
    auto first = PanelDef::create("ui", "shared", "First plugin");
    auto second = PanelDef::create("ui", "shared", "Second plugin");
    registry.add(first);
    registry.add(second);
    const auto contributions = registry.get_widgets(first->get_id());
    if (contributions.size() != 2 || std::ranges::find(contributions, first) == contributions.end()
        || std::ranges::find(contributions, second) == contributions.end())
      {
        std::cerr << "Contributions to the same panel must coexist\n";
        return 1;
      }

    auto extension = std::make_shared<Def>();
    auto default_extension = std::make_shared<Def>("ignored", "location");
    auto top = registry.find("ui.shared.top");
    auto copy = top;
    copy.add(extension);
    registry.find("ui.shared.bottom").add(extension);
    registry.find("ui.shared").add(default_extension);
    registry.find("ui.other.top").add(extension);
    const auto extensions = registry.get_panel_extensions().at(first->get_id());
    const auto contains = [&](const auto &def, Placement placement) {
      return std::count_if(extensions.begin(),
                           extensions.end(),
                           [&](const auto &entry) {
                             return entry.def == def && entry.placement == placement && entry.page == "ui"
                                    && entry.panel == "shared";
                           })
             == 1;
    };
    if (extensions.size() != 3 || !contains(extension, Placement::Top) || !contains(extension, Placement::Bottom)
        || !contains(default_extension, Placement::Bottom) || registry.get_widgets(first->get_id()).size() != 2)
      {
        std::cerr << "The selected section must determine the insertion target and position\n";
        return 2;
      }

    registry.remove(first);
    const auto remaining = registry.get_widgets(second->get_id());
    if (remaining.size() != 1 || remaining.front() != second || registry.get_panel_extensions().at(second->get_id()).size() != 3)
      {
        std::cerr << "Removing one plugin must preserve the other contributions\n";
        return 3;
      }
    registry.remove(extension);
    const auto remaining_extensions = registry.get_panel_extensions().at(second->get_id());
    if (remaining_extensions.size() != 1 || remaining_extensions.front().def != default_extension
        || !registry.get_panel_extensions().at("ui.other.").empty())
      {
        std::cerr << "Removing a definition must remove all of its insertions only\n";
        return 4;
      }
    registry.remove(default_extension);
    if (!registry.get_panel_extensions().at(second->get_id()).empty())
      {
        return 5;
      }

    for (const auto *location: {"ui", ".applet.top", "ui..top", "ui.applet.middle", "ui.applet.top.extra"})
      {
        auto section = registry.find(location);
        if (section)
          {
            std::cerr << "Invalid insertion location was accepted: " << location << '\n';
            return 6;
          }
        section.add(extension);
      }
    registry.add(nullptr);
    registry.remove(nullptr);
    top.add(nullptr);
    IPreferencesRegistry::Section{}.add(extension);
    if (!registry.get_panel_extensions().at(second->get_id()).empty() || registry.get_widgets(second->get_id()).size() != 1
        || registry.get_panel_extensions().size() != 2)
      {
        std::cerr << "Invalid registrations must leave existing preferences unchanged\n";
        return 7;
      }
    top.add(extension);
    if (registry.get_panel_extensions().at(second->get_id()).size() != 1)
      {
        std::cerr << "Valid sections must still work after invalid registrations\n";
        return 8;
      }

    std::cout << "Shared panels and named insertion sections passed\n";
  }
catch (const std::exception &error)
  {
    std::fputs("Regression test failed: ", stderr);
    std::fputs(error.what(), stderr);
    std::fputs("\n", stderr);
    return 1;
  }
