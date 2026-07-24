#pragma once

#include <cstdint>
#include <string>

#include <boost/signals2/signal.hpp>

// Exercises the DBus backend's scalar/string/enum/signal path:
// @rpc.dbus(interface=...) alongside @rpc(service=...), enum wire-encoding
// via @rpc.enum.value canonical names (required for DBus — see
// dbus_gen.rs::render_enum_marshall), a return value, and a signal.
// @rpc.enum(name="test_mode")
enum class TestMode
{
  // @rpc.enum.value(name="idle")
  Idle,
  // @rpc.enum.value(name="active")
  Active,
};

// @rpc(service="DBusFixtureService")
// @rpc.dbus(interface="org.workrave.TestInterface")
class RpcDBusFixture
{
public:
  // @rpc(name="Ping")
  std::string ping(std::string message);

  // @rpc(name="GetMode")
  TestMode get_mode();

  // @rpc(name="SetMode")
  void set_mode(TestMode mode);

  // @rpc.signal(name="ModeChanged")
  boost::signals2::signal<void(TestMode)> &signal_mode_changed()
  {
    return signal_mode_changed_;
  }

private:
  boost::signals2::signal<void(TestMode)> signal_mode_changed_;
};
