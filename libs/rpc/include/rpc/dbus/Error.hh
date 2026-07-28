// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace workrave::rpc::dbus
{
  namespace error_names
  {
    inline constexpr std::string_view failed = "org.freedesktop.DBus.Error.Failed";
    inline constexpr std::string_view invalid_args = "org.freedesktop.DBus.Error.InvalidArgs";
    inline constexpr std::string_view unknown_method = "org.freedesktop.DBus.Error.UnknownMethod";
  }

  class Error : public std::runtime_error
  {
  public:
    Error(std::string name, std::string message)
      : std::runtime_error(std::move(message))
      , name_(std::move(name))
    {
    }

    [[nodiscard]] const std::string &name() const noexcept
    {
      return name_;
    }

  private:
    std::string name_;
  };
}
