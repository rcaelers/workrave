// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <string>

#include "config/IConfigurator.hh"

class Core;

// Owns the annotation-generated gRPC endpoints for Core and its existing
// Break objects. The generated adapters invoke those objects directly.
class RpcCoreServer
{
public:
  RpcCoreServer(Core &core, workrave::config::IConfigurator &configurator, std::string listen_address);
  ~RpcCoreServer();

  RpcCoreServer(const RpcCoreServer &) = delete;
  RpcCoreServer &operator=(const RpcCoreServer &) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
