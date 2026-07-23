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

#ifndef WORKRAVE_RPC_TEST_RPCKEYEDSERVER_HH
#define WORKRAVE_RPC_TEST_RPCKEYEDSERVER_HH

#include <cstdint>

#include <boost/signals2/signal.hpp>

enum class WidgetId
{
  First,
  Second,
  Third
};

// A plain, Workrave-agnostic fixture proving the `keyed_by` mechanism end to
// end: several live instances of the same interface, distinguished by an id
// instead of a fixed reference — the gRPC analog of DBus's per-object-path
// routing (needed for e.g. Workrave's three Break instances).
// @rpc(service="WidgetService", keyed_by="WidgetId")
class RpcKeyedServer
{
public:
  explicit RpcKeyedServer(int32_t initial_value)
    : value_(initial_value)
  {
  }

  // @rpc(name="GetValue")
  [[nodiscard]] int32_t get_value() const
  {
    return value_;
  }

  // @rpc(name="SetValue")
  void set_value(int32_t v)
  {
    value_ = v;
  }

  // The gRPC analog of Break::signal_break_event() — a signal on a keyed
  // interface, proving events route to the subscriber of the right instance.
  // @rpc.signal(name="ValueChanged")
  boost::signals2::signal<void(int32_t)> &signal_value_changed()
  {
    return signal_value_changed_;
  }

  void fire_value_changed_for_test(int32_t v)
  {
    signal_value_changed_(v);
  }

private:
  int32_t value_;
  boost::signals2::signal<void(int32_t)> signal_value_changed_;
};

#endif // WORKRAVE_RPC_TEST_RPCKEYEDSERVER_HH
