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

#include "rpc/RpcServer.hh"

#include <filesystem>
#include <string_view>
#include <system_error>

#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "rpc/RpcException.hh"

namespace rpc
{
  RpcServer::RpcServer(ServerConfig config)
    : config_(std::move(config))
  {
  }

  RpcServer::~RpcServer()
  {
    shutdown();
  }

  void RpcServer::register_service(grpc::Service &service)
  {
    // Deferred to start(): grpc::ServerBuilder collects services and the
    // listen address together, so we stash the builder-affecting state and
    // apply it all in start().
    pending_services_.push_back(&service);
  }

  namespace
  {
    constexpr std::string_view unix_prefix = "unix:";
  }

  // A unix domain socket left behind by a previous, uncleanly-terminated
  // process would otherwise make AddListeningPort() fail with "Address
  // already in use" even though nothing is actually listening. Only ever
  // removes a genuine socket special file, never an arbitrary file that
  // happens to sit at the configured path.
  void RpcServer::remove_stale_unix_socket() const
  {
    if (!config_.listen_address.starts_with(unix_prefix))
      {
        return;
      }
    std::filesystem::path socket_path = config_.listen_address.substr(unix_prefix.size());
    std::error_code ec;
    if (std::filesystem::is_socket(socket_path, ec))
      {
        std::filesystem::remove(socket_path, ec);
      }
  }

  void RpcServer::start()
  {
    if (config_.enable_reflection)
      {
        // grpc::ServerBuilderPlugin registration is process-global, not
        // per-builder: this makes every ServerBuilder built anywhere in the
        // process (including ones from other RpcServer instances) reflection-
        // capable, with no way to selectively exclude one. Fine as long as a
        // process runs at most one RpcServer (the expected usage here); calling
        // this repeatedly is safe (idempotent — grpc guards against double
        // registration internally).
        grpc::reflection::InitProtoReflectionServerBuilderPlugin();
      }

    remove_stale_unix_socket();

    grpc::ServerBuilder builder;
    builder.AddListeningPort(config_.listen_address, config_.credentials, &bound_port_);
    for (grpc::Service *service: pending_services_)
      {
        builder.RegisterService(service);
      }

    server_ = builder.BuildAndStart();
    if (!server_)
      {
        throw RpcException("failed to start gRPC server on " + config_.listen_address);
      }
  }

  void RpcServer::shutdown()
  {
    if (server_)
      {
        server_->Shutdown();
        server_->Wait();
        server_.reset();
      }
  }
} // namespace rpc
