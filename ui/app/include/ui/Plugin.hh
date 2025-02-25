// Copyright (C) 2022, 2024 Rob Caelers <robc@krandor.nl>
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
#include <set>
#include <unordered_map>
#include <functional>
#include <typeindex>

#include "IPlugin.hh"
#include "IPluginContext.hh"

class PluginRegistry
{
public:
  static PluginRegistry &instance()
  {
    static auto *r = new PluginRegistry();
    return *r;
  }

  PluginRegistry() = default;

  template<typename T, typename... Deps>
  void register_plugin()
  {
    plugin_factories[typeid(T)] = [this](std::shared_ptr<IPluginContext> context) -> std::shared_ptr<IPlugin> {
      if (!in_progress.insert(typeid(T)).second)
        {
          spdlog::error("Cyclic plugin dependency detected: {}", typeid(T).name());
          return {};
        }

      auto instance = std::shared_ptr<T>(new T(context, create<Deps>(context)...), [this](T *obj) {
        in_progress.erase(typeid(T));
        delete obj;
      });

      in_progress.erase(typeid(T));

      spdlog::info("Started plugin: {}", instance->get_plugin_id());
      return instance;
    };
  }

  template<typename T>
  std::shared_ptr<T> create(std::shared_ptr<IPluginContext> context)
  {
    std::type_index id = typeid(T);
    auto plugin_it = plugins.find(id);
    if (plugin_it != plugins.end())
      {
        return std::static_pointer_cast<T>(plugin_it->second);
      }

    auto factory_it = plugin_factories.find(id);
    if (factory_it == plugin_factories.end())
      {
        spdlog::error("No factory for type {}", typeid(T).name());
        return {};
      }

    std::shared_ptr<T> instance = std::static_pointer_cast<T>(factory_it->second(context));
    plugins[id] = instance;
    return instance;
  }

  void build(std::shared_ptr<IPluginContext> context)
  {
    for (auto &[id, factory]: plugin_factories)
      {
        if (plugins.find(id) == plugins.end())
          {
            plugins[id] = factory(context);
          }
      }
  }

  void deinit()
  {
    plugin_factories.clear();
    plugins.clear();
  }

private:
  std::unordered_map<std::type_index, std::function<std::shared_ptr<IPlugin>(std::shared_ptr<IPluginContext>)>> plugin_factories;
  std::unordered_map<std::type_index, std::shared_ptr<IPlugin>> plugins;
  std::set<std::type_index> in_progress;
};

template<class Base, class... Dependencies>
class Plugin : public IPlugin
{
public:
  Plugin()
  {
    (void)registered;
  }

  static bool register_plugin()
  {
    PluginRegistry::instance().register_plugin<Base, Dependencies...>();
    return true;
  }

private:
  static bool registered;
};

template<class Base, class... Dependencies>
bool Plugin<Base, Dependencies...>::registered = Plugin<Base, Dependencies...>::register_plugin();

#endif // WORKRAVE_UI_PLUGIN_HH
