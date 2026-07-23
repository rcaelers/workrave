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

#ifndef WORKRAVE_RPC_TEST_RPCTESTSERVER_HH
#define WORKRAVE_RPC_TEST_RPCTESTSERVER_HH

#include <cstdint>
#include <string>

#include <boost/signals2/signal.hpp>

enum class TestMode
{
  Idle,
  Active
};

// A plain, Workrave-agnostic fixture used to verify the clang-rpc-gen
// pipeline end to end: annotate -> generate .proto + adapter -> protoc/grpc
// codegen -> build -> a real gRPC call that reaches this unmodified class.
// @rpc(service="TestService")
class RpcTestServer
{
public:
  //! Echoes a message back, appending " pong".
  // @rpc(name="Ping")
  std::string ping(std::string message);

  // @rpc(name="Add")
  int32_t add(int32_t a, int32_t b);

  // @rpc(name="SetFlag")
  void set_flag(bool value);

  // Not an RPC — used by the test to assert the real method executed
  // server-side, not just that a stub echoed something back.
  [[nodiscard]] bool get_flag() const
  {
    return flag_;
  }

  // @rpc(name="GetMode")
  // @rpc.param(mode, dir=out)
  bool get_mode(TestMode &mode);

  // @rpc(name="Greet")
  std::string greet(const std::string &name);

  void set_mode_for_test(TestMode mode)
  {
    mode_ = mode;
  }

  // The gRPC analog of a DBus signal — see Core::signal_operation_mode_changed()
  // for the real-world equivalent this fixture proves out.
  // @rpc.signal(name="ModeChanged")
  boost::signals2::signal<void(TestMode)> &signal_mode_changed()
  {
    return signal_mode_changed_;
  }

  void fire_mode_changed_for_test(TestMode mode)
  {
    signal_mode_changed_(mode);
  }

private:
  bool flag_{false};
  TestMode mode_{TestMode::Idle};
  boost::signals2::signal<void(TestMode)> signal_mode_changed_;
};

#endif // WORKRAVE_RPC_TEST_RPCTESTSERVER_HH
