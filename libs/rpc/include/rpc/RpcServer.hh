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

#ifndef WORKRAVE_RPC_RPCSERVER_HH
#define WORKRAVE_RPC_RPCSERVER_HH

#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

// A thin, Workrave-agnostic lifecycle wrapper around grpc::Server. Unlike
// DBus's IDBus (workrave::dbus::IDBus), this has no object-path/interface-name
// registry: each generated <Interface>ServiceImpl is already a concrete
// grpc::Service constructed directly with a reference to the real object, and
// grpc::ServerBuilder::RegisterService() dispatches through the statically
// generated protobuf service descriptor. DBus needs that registry because
// dispatch is dynamic (calls arrive keyed by a wire-level interface name on a
// shared bus connection); that doesn't apply here.
namespace rpc
{
  struct ServerConfig
  {
    // e.g. "127.0.0.1:0" (ephemeral loopback, used by tests) or
    // "unix:/run/workrave/rpc.sock". A configuration point, not hardcoded —
    // the production transport choice is deliberately deferred, see the plan.
    std::string listen_address;
    std::shared_ptr<grpc::ServerCredentials> credentials = grpc::InsecureServerCredentials();

    // gRPC server reflection (grpc.reflection.v1.ServerReflection) — the
    // gRPC analog of DBus's Introspectable interface: lets generic clients
    // (grpcurl, evans, Postman, ...) discover registered services/methods
    // and message schemas at runtime without needing the .proto files on
    // hand. On by default, same as DBus introspection; set false to turn it
    // off for a deployment that shouldn't expose its own API surface.
    bool enable_reflection = true;
  };

  class RpcServer
  {
  public:
    explicit RpcServer(ServerConfig config);
    ~RpcServer();

    RpcServer(const RpcServer &) = delete;
    RpcServer &operator=(const RpcServer &) = delete;

    // The caller owns the generated ServiceImpl instance and must keep it
    // alive for as long as the server runs.
    void register_service(grpc::Service &service);

    // Starts listening. Throws RpcException if the port could not be bound.
    void start();
    void shutdown();

    // The actual bound port, e.g. useful when ServerConfig::listen_address
    // requested an ephemeral port ("...:0") and the real port is needed
    // (tests, discovery).
    [[nodiscard]] int bound_port() const
    {
      return bound_port_;
    }

  private:
    void remove_stale_unix_socket() const;

  private:
    ServerConfig config_;
    std::vector<grpc::Service *> pending_services_;
    std::unique_ptr<grpc::Server> server_;
    int bound_port_{0};
  };
} // namespace rpc

#endif // WORKRAVE_RPC_RPCSERVER_HH
