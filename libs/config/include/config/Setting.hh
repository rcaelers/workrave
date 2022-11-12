// Copyright (C) 2013 Rob Caelers <rob.caelers@gmail.com>
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

#ifndef WORKRAVE_CONFIG_SETTING_HH
#define WORKRAVE_CONFIG_SETTING_HH

#include <boost/lexical_cast/bad_lexical_cast.hpp>
#include <boost/signals2.hpp>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>

#include <utility>
#include <chrono>

#include "utils/Enum.hh"
#include "utils/Signals.hh"
#include "config/IConfigurator.hh"
#include "config/IConfiguratorListener.hh"

namespace workrave
{
  namespace config
  {
    class SettingBase
    {
    public:
      virtual ~SettingBase() = default;
    };

    class SettingGroup
      : public SettingBase
      , IConfiguratorListener
      , boost::noncopyable
    {
    private:
      using NotifyType = boost::signals2::signal<void()>;

    public:
      explicit SettingGroup(workrave::config::IConfigurator::Ptr config, std::string setting)
        : config(config)
        , setting(std::move(setting))
      {
      }

      ~SettingGroup() override = default;

      std::string key() const
      {
        return setting;
      }

      template<typename F>
      auto connect(workrave::utils::Trackable *track_target, F func)
      {
        config->add_listener(key(), this);
        auto slot = typename NotifyType::slot_type(func);
        return signal.connect(slot.track_foreign(track_target->tracker_object()));
      }

      template<typename F>
      auto connect(workrave::utils::Trackable &track_target, F func)
      {
        config->add_listener(key(), this);
        auto slot = typename NotifyType::slot_type(func);
        return signal.connect(slot.track_foreign(track_target.tracker_object()));
      }

      void config_changed_notify(const std::string &key) override
      {
        (void)key;

        if (signal.empty())
          {
            config->remove_listener(this);
          }

        signal();
      }

    private:
      workrave::config::IConfigurator::Ptr config;
      std::string setting;
      NotifyType signal;
    };

    template<class Tag, class R, class T>
    struct setting_cast_impl;

    struct cast_tag
    {
    };

    template<class R, class T>
    struct setting_cast_impl<cast_tag, R, T>
    {
      constexpr static R call(const T &t)
      {
        return static_cast<R>(t);
      }
    };

    template<>
    struct setting_cast_impl<cast_tag, std::string, std::string>
    {
      constexpr static std::string call(const std::string &t)
      {
        return t;
      }
    };

    template<class T>
    struct setting_cast_impl<cast_tag, std::string, T>
    {
      constexpr static std::string call(const T &t)
      {
        if constexpr (workrave::utils::enum_has_names_v<T>)
          {
            return std::string{workrave::utils::enum_to_string(t)};
          }
        else
          {
            return static_cast<std::string>(t);
          }
      }
    };

    template<class R>
    struct setting_cast_impl<cast_tag, R, std::string>
    {
      constexpr static R call(std::string t)
      {
        if constexpr (workrave::utils::enum_has_names_v<R>)
          {
            return workrave::utils::enum_from_string<R>(t);
          }
        else
          {
            return static_cast<std::string>(t);
          }
      }
    };

    template<class R, class Rep, class Period>
    struct setting_cast_impl<cast_tag, R, std::chrono::duration<Rep, Period>>
    {
      using T = std::chrono::duration<Rep, Period>;
      constexpr static R call(const T &t)
      {
        return t.count();
      }
    };

    template<class Clock, class Duration, class R>
    struct setting_cast_impl<cast_tag, R, std::chrono::time_point<Clock, Duration>>
    {
      using T = const std::chrono::time_point<Clock, Duration>;
      constexpr static int64_t call(const T &t)
      {
        return std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch()).count();
      }
    };

    template<class Clock, class Duration, class T>
    struct setting_cast_impl<cast_tag, std::chrono::time_point<Clock, Duration>, T>
    {
      using R = const std::chrono::time_point<Clock, Duration>;
      constexpr static R call(const T &t)
      {
        return R{std::chrono::seconds{t}};
      }
    };

    template<class R, class T>
    constexpr R setting_cast(const T &t)
    {
      using impl = setting_cast_impl<cast_tag, R, T>;
      return impl::call(t);
    }

    template<class T, class R = T>
    class Setting
      : public SettingBase
      , IConfiguratorListener
      , boost::noncopyable
    {
    public:
      using config_type = T;
      using representation_type = R;

    private:
      using NotifyType = boost::signals2::signal<void(const R &)>;

    public:
      Setting(workrave::config::IConfigurator::Ptr config, std::string setting)
        : config(config)
        , setting(std::move(setting))
        , has_default_value(false)
      {
      }

      Setting(workrave::config::IConfigurator::Ptr config, std::string setting, R default_value)
        : config(config)
        , setting(std::move(setting))
        , has_default_value(true)
        , default_value(std::move(default_value))
      {
      }

      ~Setting() override = default;

      constexpr ConfigType get_type() const
      {
        if constexpr (std::is_same_v<bool, T>)
          {
            return ConfigType::Boolean;
          }
        else if constexpr (std::is_same_v<int64_t, T>)
          {
            return ConfigType::Int64;
          }
        else if constexpr (std::is_same_v<int32_t, T>)
          {
            return ConfigType::Int32;
          }
        else if constexpr (std::is_same_v<double, T>)
          {
            return ConfigType::Double;
          }
        else if constexpr (std::is_same_v<std::string, T>)
          {
            return ConfigType::String;
          }
        return ConfigType::Unknown;
      }

      std::string key() const
      {
        return setting;
      }

      R operator()() const
      {
        return get();
      }

      R operator()(const T def) const
      {
        return get(def);
      }

      R get() const
      {
        T ret = T();
        if (has_default_value)
          {
            config->get_value_with_default(setting, ret, setting_cast<T>(default_value));
          }
        else
          {
            config->get_value(setting, ret);
          }
        return setting_cast<R>(ret);
      }

      R get(const R def) const
      {
        T ret = T();
        config->get_value_with_default(setting, ret, setting_cast<T>(def));
        return setting_cast<R>(ret);
      }

      void set(const R &val)
      {
        config->set_value(setting, setting_cast<T>(val));
      }

      template<typename F>
      auto connect(workrave::utils::Trackable *track_target, F func)
      {
        config->add_listener(key(), this);
        auto slot = typename NotifyType::slot_type(func);
        return signal.connect(slot.track_foreign(track_target->tracker_object()));
      }

      template<typename F>
      auto connect(workrave::utils::Trackable &track_target, F func)
      {
        config->add_listener(key(), this);
        auto slot = typename NotifyType::slot_type(func);
        return signal.connect(slot.track_foreign(track_target.tracker_object()));
      }

      template<typename F>
      auto attach(workrave::utils::Trackable *track_target, F func)
      {
        config->add_listener(key(), this);
        auto slot = typename NotifyType::slot_type(func);
        auto connection = signal.connect(slot.track_foreign(track_target->tracker_object()));
        signal(get());
        return connection;
      }

      template<typename F>
      auto attach(workrave::utils::Trackable &track_target, F func)
      {
        config->add_listener(key(), this);
        auto slot = typename NotifyType::slot_type(func);
        auto connection = signal.connect(slot.track_foreign(track_target.tracker_object()));
        signal(get());
        return connection;
      }

      // TODO: refactor settings, it should not depend on IConfigurator
      workrave::config::IConfigurator::Ptr get_config()
      {
        return config;
      }

    private:
      void config_changed_notify(const std::string &key) override
      {
        (void)key;

        if (signal.empty())
          {
            config->remove_listener(this);
          }
        else
          {
            signal(get());
          }
      }

    private:
      workrave::config::IConfigurator::Ptr config;
      std::string setting;
      bool has_default_value;
      R default_value;
      NotifyType signal;
    };

    template<class T, class R>
    class Setting<std::vector<T>, std::vector<R>> : public Setting<std::string>
    {
    public:
      using base = Setting<std::string>;

      Setting(workrave::config::IConfigurator::Ptr config, std::string setting)
        : Setting<std::string>(config, setting)
      {
      }

      Setting(workrave::config::IConfigurator::Ptr config, std::string setting, std::vector<R> default_value)
        : Setting<std::string>(config, setting, convert(default_value))
      {
      }

      ~Setting() override = default;

      std::vector<R> operator()() const
      {
        return get();
      }

      std::vector<R> get() const
      {
        std::string value = base::get();
        std::vector<R> ret;
        if (!value.empty())
          {
            std::vector<std::string> items;
            boost::split(items, value, boost::is_any_of(";"));
            try
              {
                std::transform(items.begin(), items.end(), std::back_inserter(ret), [](const auto &s) {
                  return setting_cast<R>(boost::lexical_cast<T, std::string>(s));
                });
              }
            catch (boost::bad_lexical_cast &e)
              {
                // FIXME: LOG
              }
          }
        return ret;
      }

      void set(const std::vector<R> &val)
      {
        base::set(convert(val));
      }

    private:
      std::string convert(const std::vector<R> &val)
      {
        using boost::adaptors::transformed;
        using boost::algorithm::join;

        using TT = std::decay_t<R>;

        if constexpr (!std::is_same_v<std::string, TT>)
          {
            return join(val | transformed([](R d) { return std::to_string(setting_cast<T>(d)); }), ";");
          }
        else
          {
            return join(val, ";");
          }
      }
    };

  } // namespace config
} // namespace workrave

#endif // WORKRAVE_CONFIG_SETTING_HH
