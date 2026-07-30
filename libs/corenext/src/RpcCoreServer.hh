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

#ifndef WORKRAVE_CORENEXT_RPCCORESERVER_HH
#define WORKRAVE_CORENEXT_RPCCORESERVER_HH

#include <memory>
#include <string>

#include "config/IConfigurator.hh"
#include "core/CoreTypes.hh"

class Core;
class Break;
namespace rpc
{
  template<typename Key, typename T>
  class InstanceRegistry;
}

// Owns the gRPC server exposing CoreService/BreakService/ConfigService for
// the real, running Core/BreaksControl/Configurator instances (constructed
// from Core::init_rpc(), the earliest point a concrete Core& is available —
// see CoreFactory::create()).
//
// Implemented in RpcCoreServer.cc, which is compiled into
// workrave-libs-core-next-rpc rather than workrave-libs-core-next: the
// generated ServiceImpl classes (and therefore grpc++/protobuf) are
// deliberately kept out of the main corenext library, see
// libs/corenext/src/CMakeLists.txt. This header itself stays free of all of
// that (PIMPL), so Core.hh/Core.cc — compiled as part of the plain
// workrave-libs-core-next — can #include this header and hold a pointer to
// it without pulling gRPC/protobuf onto every consumer's link line.
//
// workrave-libs-core-next depends on workrave-libs-core-next-rpc (this is
// the one allowed direction: Core::init_rpc() constructs a RpcCoreServer).
// The reverse never happens: RpcCoreServer.cc only calls virtual
// (ICore/IBreak-overriding, or explicitly `virtual`) methods through the
// Core&/Break& references it's given, so it needs no real symbol from
// workrave-libs-core-next — the break registry is passed in explicitly
// below rather than obtained via Core::get_break_registry() (a non-virtual
// method) precisely to avoid reintroducing such a dependency.
class RpcCoreServer
{
public:
  RpcCoreServer(Core &core,
                rpc::InstanceRegistry<workrave::BreakId, Break> &break_registry,
                workrave::config::IConfigurator &configurator,
                std::string listen_address);
  ~RpcCoreServer();

  RpcCoreServer(const RpcCoreServer &) = delete;
  RpcCoreServer &operator=(const RpcCoreServer &) = delete;

  [[nodiscard]] int bound_port() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

#endif // WORKRAVE_CORENEXT_RPCCORESERVER_HH
