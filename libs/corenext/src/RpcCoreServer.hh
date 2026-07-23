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

class Core;

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
// The constructor's *definition* lives in the -rpc library, so Core.o's
// reference to it is only resolved when the final executable links both
// workrave-libs-core-next and workrave-libs-core-next-rpc together — there is
// deliberately no CMake target_link_libraries edge from
// workrave-libs-core-next to workrave-libs-core-next-rpc (that would be
// circular, since the -rpc library already depends on the plain one). Any
// target that wants a working Core must therefore link both, same as the
// generated DBus glue (init_DBusWorkraveNext(), called from init_bus()) is a
// symbol Core.cc depends on without workrave-libs-core-next declaring a link
// dependency on whatever produces it.
class RpcCoreServer
{
public:
  RpcCoreServer(Core &core, workrave::config::IConfigurator &configurator, std::string listen_address);
  ~RpcCoreServer();

  RpcCoreServer(const RpcCoreServer &) = delete;
  RpcCoreServer &operator=(const RpcCoreServer &) = delete;

  [[nodiscard]] int bound_port() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

#endif // WORKRAVE_CORENEXT_RPCCORESERVER_HH
