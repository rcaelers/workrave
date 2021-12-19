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

#include <boost/signals2.hpp>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>
#include <utility>

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

    template<class T, class R = T>
    class Setting
      : public SettingBase
      , IConfiguratorListener
      , boost::noncopyable
    {
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
            config->get_value_with_default(setting, ret, static_cast<T>(default_value));
          }
        else
          {
            config->get_value(setting, ret);
          }
        return static_cast<R>(ret);
      }

      R get(const R def) const
      {
        const T ret = T();
        config->get_value_with_default(setting, ret, static_cast<T>(def));
        return static_cast<R>(ret);
      }

      void set(const R &val)
      {
        config->set_value(setting, static_cast<T>(val));
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

    template<class T>
    class Setting<std::vector<T>, std::vector<T>> : public Setting<std::string>
    {
    public:
      using base = Setting<std::string>;

      Setting(workrave::config::IConfigurator::Ptr config, std::string setting)
        : Setting<std::string>(config, setting)
      {
      }

      Setting(workrave::config::IConfigurator::Ptr config, std::string setting, std::vector<T> default_value)
        : Setting<std::string>(config, setting, convert(default_value))
      {
      }

      ~Setting() override = default;

      std::vector<T> operator()() const
      {
        return get();
      }

      std::vector<T> get() const
      {
        std::string value = base::get();
        std::vector<std::string> items;
        std::vector<T> ret;
        boost::split(items, value, boost::is_any_of(";"));
        std::transform(items.begin(), items.end(), std::back_inserter(ret), boost::lexical_cast<T, std::string>);

        return ret;
      }

      void set(const std::vector<T> &val)
      {
        base::set(convert(val));
      }

    private:
      std::string convert(const std::vector<T> &val)
      {
        using boost::adaptors::transformed;
        using boost::algorithm::join;

        using TT = std::decay_t<T>;

        if constexpr (!std::is_same_v<std::string, TT>)
          {
            return join(val | transformed([](T d) { return std::to_string(d); }), ";");
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
