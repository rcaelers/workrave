// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKRAVE_CORENEXT_RPC_DBUS_CONFIG_COMPAT_HH
#define WORKRAVE_CORENEXT_RPC_DBUS_CONFIG_COMPAT_HH

#include <cstdint>
#include <string>

#include "config/IConfigurator.hh"

namespace workrave::core::rpc::dbus_compat
{
  // Exact C++ facade for the legacy org.workrave.ConfigInterface wire API.
  // It supplies CONFIG_FLAG_NONE internally because ConfigFlags was never a
  // DBus argument, and uses explicit out parameters to preserve legacy reply
  // ordering (including GetString's historical found-before-value order).
  // @rpc(service="workrave.dbus.compat.LegacyConfigDBusService")
  // @rpc.dbus(interface="org.workrave.ConfigInterface")
  class ConfigCompat
  {
  public:
    explicit ConfigCompat(workrave::config::IConfigurator &configurator);

    // @rpc(name="SetString")
    void set_string(const std::string &key, const std::string &value);
    // @rpc(name="SetInt")
    void set_int(const std::string &key, int32_t value);
    // @rpc(name="SetInt64")
    void set_int64(const std::string &key, int64_t value);
    // @rpc(name="SetBool")
    void set_bool(const std::string &key, bool value);
    // @rpc(name="SetDouble")
    void set_double(const std::string &key, double value);

    // @rpc(name="GetString")
    // @rpc.param(found, dir=out)
    // @rpc.param(value, dir=out)
    void get_string(const std::string &key, bool &found, std::string &value) const;
    // @rpc(name="GetInt")
    // @rpc.param(value, dir=out)
    // @rpc.param(found, dir=out)
    void get_int(const std::string &key, int32_t &value, bool &found) const;
    // @rpc(name="GetBool")
    // @rpc.param(value, dir=out)
    // @rpc.param(found, dir=out)
    void get_bool(const std::string &key, bool &value, bool &found) const;
    // @rpc(name="GetDouble")
    // @rpc.param(value, dir=out)
    // @rpc.param(found, dir=out)
    void get_double(const std::string &key, double &value, bool &found) const;

  private:
    workrave::config::IConfigurator &configurator_;
  };
} // namespace workrave::core::rpc::dbus_compat

#endif // WORKRAVE_CORENEXT_RPC_DBUS_CONFIG_COMPAT_HH
