// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKRAVE_CORENEXT_RPC_DBUS_NAMES_HH
#define WORKRAVE_CORENEXT_RPC_DBUS_NAMES_HH

#include <string_view>

struct RpcDBusNames
{
  std::string_view service;
  std::string_view root_path;

  [[nodiscard]] static constexpr RpcDBusNames select(bool legacy_dbus_enabled)
  {
    if (legacy_dbus_enabled)
      {
        return {"org.workrave.Workrave.Rpc", "/org/workrave/Workrave/Rpc"};
      }
    return {"org.workrave.Workrave", "/org/workrave/Workrave"};
  }
};

#endif // WORKRAVE_CORENEXT_RPC_DBUS_NAMES_HH
