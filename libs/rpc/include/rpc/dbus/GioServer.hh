// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <gio/gio.h>

#include "rpc/dbus/Registration.hh"

namespace workrave::rpc::dbus
{
  class GioInterface;

  class GioServer
  {
  public:
    ~GioServer();

    GioServer(const GioServer &) = delete;
    GioServer &operator=(const GioServer &) = delete;
    GioServer(GioServer &&) = delete;
    GioServer &operator=(GioServer &&) = delete;

    static std::unique_ptr<GioServer> session();

    [[nodiscard]] bool is_available() const noexcept;
    void request_name(std::string_view name);

    Registration register_interface(std::string path, std::shared_ptr<GioInterface> interface);
    Registration watch_name(std::string name, std::function<void(bool)> callback);
    void emit_signal(std::string_view path,
                     std::string_view interface_name,
                     std::string_view signal_name,
                     GVariant *parameters,
                     GUnixFDList *fd_list = nullptr);

  private:
    class State;
    explicit GioServer(std::shared_ptr<State> state);
    std::shared_ptr<State> state_;
  };
}
