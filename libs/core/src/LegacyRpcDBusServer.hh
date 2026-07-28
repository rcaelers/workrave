// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include "config/IConfigurator.hh"

class Core;

// Owns the standalone generated D-Bus endpoints for libs/core.
class LegacyRpcDBusServer
{
public:
  LegacyRpcDBusServer(Core &core, workrave::config::IConfigurator &configurator);
  ~LegacyRpcDBusServer();

  LegacyRpcDBusServer(const LegacyRpcDBusServer &) = delete;
  LegacyRpcDBusServer &operator=(const LegacyRpcDBusServer &) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
