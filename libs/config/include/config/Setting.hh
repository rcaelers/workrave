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
#include <utility>

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

      std::string key() const { return setting; }

      boost::signals2::connection connect(NotifyType::slot_type slot)
      {
        config->add_listener(key(), this);
        return signal.connect(slot);
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
      explicit Setting(workrave::config::IConfigurator::Ptr config, std::string setting)
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

      std::string key() const { return setting; }

      R operator()() const { return get(); }

      R operator()(const T def) const { return get(def); }

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

      void set(const R &val) { config->set_value(setting, static_cast<T>(val)); }

      boost::signals2::connection connect(typename NotifyType::slot_type slot)
      {
        config->add_listener(key(), this);
        return signal.connect(slot);
      }

      boost::signals2::connection attach(typename NotifyType::slot_type slot)
      {
        config->add_listener(key(), this);
        boost::signals2::connection ret = signal.connect(slot);
        signal(get());
        return ret;
      }

      void config_changed_notify(const std::string &key) override
      {
        (void)key;

        if (signal.empty())
          {
            config->remove_listener(this);
          }

        signal(get());
      }

    private:
      workrave::config::IConfigurator::Ptr config;
      std::string setting;
      bool has_default_value;
      R default_value;
      NotifyType signal;
    };
  } // namespace config
} // namespace workrave

#endif // WORKRAVE_CONFIG_SETTING_HH
