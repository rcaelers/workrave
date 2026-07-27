// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "RpcDBusConfigCompat.hh"

namespace workrave::core::rpc::dbus_compat
{
  ConfigCompat::ConfigCompat(workrave::config::IConfigurator &configurator)
    : configurator_(configurator)
  {
  }

  void ConfigCompat::set_string(const std::string &key, const std::string &value)
  {
    configurator_.set_value(key, value, workrave::config::CONFIG_FLAG_NONE);
  }

  void ConfigCompat::set_int(const std::string &key, int32_t value)
  {
    configurator_.set_value(key, value, workrave::config::CONFIG_FLAG_NONE);
  }

  void ConfigCompat::set_int64(const std::string &key, int64_t value)
  {
    configurator_.set_value(key, value, workrave::config::CONFIG_FLAG_NONE);
  }

  void ConfigCompat::set_bool(const std::string &key, bool value)
  {
    configurator_.set_value(key, value, workrave::config::CONFIG_FLAG_NONE);
  }

  void ConfigCompat::set_double(const std::string &key, double value)
  {
    configurator_.set_value(key, value, workrave::config::CONFIG_FLAG_NONE);
  }

  void ConfigCompat::get_string(const std::string &key, bool &found, std::string &value) const
  {
    found = configurator_.get_value(key, value);
  }

  void ConfigCompat::get_int(const std::string &key, int32_t &value, bool &found) const
  {
    found = configurator_.get_value(key, value);
  }

  void ConfigCompat::get_bool(const std::string &key, bool &value, bool &found) const
  {
    found = configurator_.get_value(key, value);
  }

  void ConfigCompat::get_double(const std::string &key, double &value, bool &found) const
  {
    found = configurator_.get_value(key, value);
  }
} // namespace workrave::core::rpc::dbus_compat
