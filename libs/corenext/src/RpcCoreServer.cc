// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "RpcCoreServer.hh"

#include <utility>

#include <spdlog/spdlog.h>

#include "rpc/RpcServer.hh"

#include "Core.hh"
#include "RpcBreakServiceImpl.hh"
#include "RpcConfigServiceImpl.hh"
#include "RpcCoreServiceImpl.hh"

struct RpcCoreServer::Impl
{
  Impl(Core &core, workrave::config::IConfigurator &configurator, std::string listen_address)
    : core_service(core)
    , break_service(core.get_break_registry())
    , config_service(configurator)
    , server(rpc::ServerConfig{.listen_address = listen_address})
  {
    server.register_service(core_service);
    server.register_service(break_service);
    server.register_service(config_service);
    server.start();

    // For TCP addresses the configured port may be "0" (pick any free port),
    // so report the actually bound port rather than echoing the request back.
    std::string bound_address = listen_address;
    if (!listen_address.starts_with("unix:"))
      {
        auto colon_pos = listen_address.rfind(':');
        std::string host = colon_pos != std::string::npos ? listen_address.substr(0, colon_pos) : listen_address;
        bound_address = host + ":" + std::to_string(server.bound_port());
      }
    spdlog::info("RPC server listening on {}", bound_address);
  }

  ~Impl()
  {
    server.shutdown();
  }

  Impl(const Impl &) = delete;
  Impl &operator=(const Impl &) = delete;
  Impl(Impl &&) = delete;
  Impl &operator=(Impl &&) = delete;

  workrave::core::rpc::CoreServiceServiceImpl core_service;
  workrave::core::rpc::BreakServiceServiceImpl break_service;
  workrave::config::rpc::ConfigServiceServiceImpl config_service;
  rpc::RpcServer server;
};

RpcCoreServer::RpcCoreServer(Core &core, workrave::config::IConfigurator &configurator, std::string listen_address)
  : impl_(std::make_unique<Impl>(core, configurator, std::move(listen_address)))
{
}

RpcCoreServer::~RpcCoreServer() = default;

int
RpcCoreServer::bound_port() const
{
  return impl_->server.bound_port();
}
