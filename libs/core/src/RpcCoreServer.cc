// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "RpcCoreServer.hh"

#include <stdexcept>
#include <utility>

#include <spdlog/spdlog.h>

#include "Break.hh"
#include "Core.hh"
#include "RpcBreakServiceImpl.hh"
#include "RpcConfigServiceImpl.hh"
#include "RpcCoreServiceImpl.hh"
#include "rpc/InstanceRegistry.hh"
#include "rpc/RpcServer.hh"

namespace generated = workrave::core::rpc;

struct RpcCoreServer::Impl
{
  Impl(Core &core, workrave::config::IConfigurator &configurator, std::string listen_address)
    : core_service(core)
    , break_service(break_registry)
    , config_service(configurator)
    , server(::rpc::ServerConfig{.listen_address = listen_address})
  {
    for (workrave::BreakId id = workrave::BREAK_ID_MICRO_BREAK; id < workrave::BREAK_ID_SIZEOF; id++)
      {
        Break *break_controller = core.get_break(id);
        if (break_controller == nullptr)
          {
            throw std::runtime_error("Core returned no Break implementation");
          }
        break_registry.register_instance(id, *break_controller);
      }

    server.register_service(core_service);
    server.register_service(break_service);
    server.register_service(config_service);
    server.start();

    std::string bound_address = listen_address;
    if (!listen_address.starts_with("unix:"))
      {
        auto colon_pos = listen_address.rfind(':');
        std::string host = colon_pos != std::string::npos ? listen_address.substr(0, colon_pos) : listen_address;
        bound_address = host + ":" + std::to_string(server.bound_port());
      }
    spdlog::info("Core gRPC server listening on {}", bound_address);
  }

  ~Impl()
  {
    server.shutdown();
  }

  ::rpc::InstanceRegistry<workrave::BreakId, Break> break_registry;
  generated::CoreServiceServiceImpl core_service;
  generated::BreakServiceServiceImpl break_service;
  workrave::config::rpc::ConfigServiceServiceImpl config_service;
  ::rpc::RpcServer server;
};

RpcCoreServer::RpcCoreServer(Core &core, workrave::config::IConfigurator &configurator, std::string listen_address)
  : impl_(std::make_unique<Impl>(core, configurator, std::move(listen_address)))
{
}

RpcCoreServer::~RpcCoreServer() = default;
