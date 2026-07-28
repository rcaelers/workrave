// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string_view>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>

namespace workrave::rpc::dbus
{
  class QtInterface
  {
  public:
    virtual ~QtInterface() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual std::string_view introspection() const noexcept = 0;
    virtual bool dispatch(const QDBusMessage &message, const QDBusConnection &connection) = 0;
  };
}
