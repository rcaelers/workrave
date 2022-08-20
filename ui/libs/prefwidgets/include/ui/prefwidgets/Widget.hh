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

#ifndef WORKRAVE_UI_PREFWIDGETS_WIDGET_HH
#define WORKRAVE_UI_PREFWIDGETS_WIDGET_HH

#include <memory>
#include <string>
#include <utility>
#include <type_traits>

#include "config/Setting.hh"
#include "config/IConfigurator.hh"
#include "utils/Signals.hh"

#include <spdlog/spdlog.h>

namespace ui::prefwidgets
{
  namespace detail
  {
    template<typename S, typename T>
    std::shared_ptr<S> stream(std::shared_ptr<S> s, T t, const std::true_type &enabler)
    {
      *s << t;
      return s;
    }
  } // namespace detail

  enum class Orientation
  {
    Horizontal,
    Vertical
  };

  class SizeGroup;

  class Widget : public std::enable_shared_from_this<Widget>
  {
  public:
    Widget() = default;
    explicit Widget(const std::string &label);
    virtual ~Widget() = default;

    std::string get_label() const;

    virtual std::list<std::shared_ptr<SizeGroup>> get_size_groups() const = 0;

  protected:
    template<class Derived>
    std::shared_ptr<Derived> shared_from_base()
    {
      return std::static_pointer_cast<Derived>(shared_from_this());
    }

  private:
    std::string label;
  };

  template<typename S, typename T>
  std::shared_ptr<S> operator<<(std::shared_ptr<S> s, std::shared_ptr<T> t)
  {
    return detail::stream(s, t, std::is_convertible<S *, Widget *>());
  }

  template<class Derived, class Type>
  class WidgetBase
    : public Widget
    , public workrave::config::IConfiguratorListener
  {
  public:
    WidgetBase() = default;
    explicit WidgetBase(const std::string &label)
      : Widget(label)
    {
    }

    ~WidgetBase() override
    {
      config->remove_listener(this);
    }

    void init(std::function<void(Type)> update_func)
    {
      this->update_func = update_func;
    }

    template<class T, class R>
    std::shared_ptr<Derived> connect(workrave::config::Setting<T, R> *setting)
    {
      std::string key = setting->key();

      config = setting->get_config();
      config->add_listener(key, this);

      keys.push_back(key);
      if (keys.size() == 1)
        {
          type = setting->get_type();

          if constexpr (std::is_convertible_v<T, Type>)
            {
              value = static_cast<Type>(setting->get());
            }
        }
      return shared_from_base<Derived>();
    }

    template<class R>
    std::shared_ptr<Derived> when(workrave::config::Setting<bool, R> *setting)
    {
      if (sensitive_key.empty())
        {
          sensitive_key = setting->key();
          sensitive = static_cast<bool>(setting->get());

          config = setting->get_config();
          config->add_listener(sensitive_key, this);
        }
      return shared_from_base<Derived>();
    }

    std::shared_ptr<Derived> on_load(std::function<Type()> func)
    {
      load_func = func;
      return shared_from_base<Derived>();
    }

    std::shared_ptr<Derived> on_save(std::function<void(Type)> func)
    {
      save_func = func;
      return shared_from_base<Derived>();
    }

    Type get_value() const
    {
      return value;
    }

    bool get_sensitive() const
    {
      return sensitive;
    }

    void set_value(Type val)
    {
      if (value != val)
        {
          value = val;

          if (save_func)
            {
              save_func(value);
            }
          else if (!keys.empty())
            {
              config->set_value(keys.front(), value);
            }
        }
    }

    std::shared_ptr<Derived> size_group(std::shared_ptr<SizeGroup> size_group)
    {
      size_groups.push_back(size_group);
      return shared_from_base<Derived>();
    }

    std::list<std::shared_ptr<SizeGroup>> get_size_groups() const override
    {
      return size_groups;
    }

  private:
    void config_changed_notify(const std::string &changed) override
    {
      if (changed == sensitive_key)
        {
          bool s = sensitive;
          config->get_value(sensitive_key, s);
          if (sensitive != s)
            {
              sensitive = s;
              // FIXME: own event/function
              if (update_func)
                {
                  update_func(value);
                }
            }
        }
      else
        {
          Type new_value = value;

          if (load_func)
            {
              new_value = load_func();
            }
          else if (!keys.empty())
            {
              auto v = config->get_value(keys.front(), type);
              if (v.has_value())
                {
                  new_value = std::visit(
                    [](auto &&arg) {
                      using T = std::decay_t<decltype(arg)>;

                      if constexpr (std::is_convertible_v<T, Type>)
                        {
                          return static_cast<Type>(arg);
                        }
                      else
                        {
                          return Type{};
                        }
                    },
                    v.value());
                }
            }

          if (new_value != value)
            {
              value = new_value;
              if (update_func)
                {
                  update_func(value);
                }
            }
        }
    }

  private:
    std::list<std::string> keys;
    std::list<std::shared_ptr<SizeGroup>> size_groups;
    std::string sensitive_key;
    bool sensitive{true};
    workrave::config::ConfigType type{workrave::config::ConfigType::Unknown};
    Type value{};
    std::function<void(Type)> update_func;
    std::function<Type()> load_func;
    std::function<void(Type)> save_func;
    workrave::config::IConfigurator::Ptr config;
  };

  template<class Derived>
  class ContainerBase : public Widget
  {
  public:
    explicit ContainerBase(const std::string &label)
      : Widget(label)
    {
    }
    ContainerBase() = default;
    ~ContainerBase() override = default;

    auto operator<<(std::shared_ptr<Widget> w)
    {
      content.push_back(w);
      return shared_from_base<Derived>();
    }

    std::list<std::shared_ptr<Widget>> &get_content()
    {
      return content;
    }

    std::shared_ptr<Derived> size_group(std::shared_ptr<SizeGroup> size_group)
    {
      size_groups.push_back(size_group);
      return shared_from_base<Derived>();
    }

    std::list<std::shared_ptr<SizeGroup>> get_size_groups() const override
    {
      return size_groups;
    }

  private:
    std::list<std::shared_ptr<Widget>> content;
    std::list<std::shared_ptr<SizeGroup>> size_groups;
  };

} // namespace ui::prefwidgets

#endif // WORKRAVE_UI_PREFWIDGETS_WIDGET_HH
