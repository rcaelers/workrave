// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string_view>

#include <gio/gio.h>

namespace workrave::rpc::dbus
{
  class GioInterface
  {
  public:
    virtual ~GioInterface() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual std::string_view introspection() const noexcept = 0;
    virtual void dispatch(std::string_view method,
                          GVariant *parameters,
                          GDBusMethodInvocation *invocation) = 0;
  };
}
