// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <QtCore/QVariantList>
#include <QtDBus/QDBusConnection>

#include "rpc/dbus/Registration.hh"

namespace workrave::rpc::dbus
{
  class QtInterface;

  class QtServer
  {
  public:
    explicit QtServer(QDBusConnection connection);
    ~QtServer();

    QtServer(const QtServer &) = delete;
    QtServer &operator=(const QtServer &) = delete;
    QtServer(QtServer &&) = delete;
    QtServer &operator=(QtServer &&) = delete;

    static std::unique_ptr<QtServer> session();

    [[nodiscard]] bool is_available() const noexcept;
    void request_name(std::string_view name);

    Registration register_interface(std::string path, std::shared_ptr<QtInterface> interface);
    Registration watch_name(std::string name, std::function<void(bool)> callback);
    void emit_signal(std::string_view path,
                     std::string_view interface_name,
                     std::string_view signal_name,
                     const QVariantList &arguments = {});

  private:
    class State;
    std::shared_ptr<State> state_;
  };
}
