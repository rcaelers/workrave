// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's QtDBus backend from an annotated C++ header.
#pragma once

#include <string>
#include <string_view>

#include <vector>

#include <boost/signals2/connection.hpp>


#include "rpc/dbus/QtInterface.hh"
#include "rpc/dbus/QtServer.hh"

#include "dbus_scalar.hh"


class org_workrave_TestInterface final : public ::workrave::rpc::dbus::QtInterface
{
public:
  org_workrave_TestInterface(::workrave::rpc::dbus::QtServer &server,
              std::string path,
              RpcDBusFixture &implementation);
  ~org_workrave_TestInterface() override = default;

  [[nodiscard]] std::string_view name() const noexcept override;
  [[nodiscard]] std::string_view introspection() const noexcept override;
  bool dispatch(const QDBusMessage &message, const QDBusConnection &connection) override;

private:

  void dispatch_Ping(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetMode(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_SetMode(const QDBusMessage &message, const QDBusConnection &connection);


  void emit_ModeChanged(TestMode value);



  ::workrave::rpc::dbus::QtServer &server_;

  std::string path_;
  RpcDBusFixture &implementation_;

  std::vector<boost::signals2::scoped_connection> signal_connections_;

};
