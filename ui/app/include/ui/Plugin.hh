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

#ifndef WORKRAVE_UI_PLUGIN_HH
#define WORKRAVE_UI_PLUGIN_HH

#include <memory>
#include <list>

#include "IPlugin.hh"
#include "IApplication.hh"

class PluginRegistry
{
public:
  using PluginFactory = std::unique_ptr<IPlugin> (*)(std::shared_ptr<IApplication> context);

  static PluginRegistry &instance()
  {
    static auto *r = new PluginRegistry();
    return *r;
  }

  PluginRegistry() = default;

  bool register_plugin(PluginFactory factory)
  {
    std::cout << "register plugin\n";
    plugin_factories.push_back(factory);
    return true;
  }

  void build(std::shared_ptr<IApplication> context)
  {
    std::cout << "build plugins\n";
    for (auto &f: plugin_factories)
      {
        std::cout << "build plugin\n";
        plugins.emplace_back(f(context));
      }
    for (auto &f: plugins)
      {
        std::cout << f->get_plugin_id() << "\n";
      }
  }

private:
  std::list<PluginFactory> plugin_factories;
  std::list<std::unique_ptr<IPlugin>> plugins;
};

template<class Base>
class Plugin : public IPlugin
{
public:
  Plugin()
  {
    std::cout << "Plugin::Plugin\n";
    (void)registered;
  }

  static bool register_plugin()
  {
    std::cout << "register_plugin\n";
    return PluginRegistry::instance().register_plugin(
      [](std::shared_ptr<IApplication> context) -> std::unique_ptr<IPlugin> { return std::make_unique<Base>(context); });
  }

protected:
  static bool registered;
};

template<class Base>
bool Plugin<Base>::registered = Plugin<Base>::register_plugin();

#endif // WORKRAVE_UI_PLUGIN_HH
