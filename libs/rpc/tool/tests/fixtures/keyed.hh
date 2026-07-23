#pragma once

#include <cstdint>

#include <boost/signals2/signal.hpp>

enum class WidgetId
{
  First,
  Second,
  Third
};

// Proves the `keyed_by` mechanism (multiple live C++ instances of the same
// interface, distinguished by an id — the gRPC analog of DBus's per-object-path
// routing, needed for e.g. Workrave's three Break instances).
// @rpc(service="WidgetService", keyed_by="WidgetId")
class RpcKeyedFixture
{
public:
  // @rpc(name="GetValue")
  int32_t get_value() const;

  // @rpc(name="SetValue")
  void set_value(int32_t v);

  // A signal on a keyed interface — proves the resolve-then-connect codegen
  // path used for e.g. Workrave's per-Break BreakEvent signal.
  // @rpc.signal(name="ValueChanged")
  boost::signals2::signal<void(int32_t)> &signal_value_changed()
  {
    return signal_value_changed_;
  }

private:
  boost::signals2::signal<void(int32_t)> signal_value_changed_;
};
