// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's QtDBus backend from an annotated C++ header.
#pragma once

#include <string>
#include <string_view>


#include "rpc/dbus/QtInterface.hh"
#include "rpc/dbus/QtServer.hh"

#include "dbus_struct_sequence.hh"


class org_workrave_TestInterface2 final : public ::workrave::rpc::dbus::QtInterface
{
public:
  org_workrave_TestInterface2(::workrave::rpc::dbus::QtServer &server,
              std::string path,
              RpcDBusFixture2 &implementation);
  ~org_workrave_TestInterface2() override = default;

  [[nodiscard]] std::string_view name() const noexcept override;
  [[nodiscard]] std::string_view introspection() const noexcept override;
  bool dispatch(const QDBusMessage &message, const QDBusConnection &connection) override;

private:

  void dispatch_SetPoint(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetPoint(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_SetTags(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetPoints(const QDBusMessage &message, const QDBusConnection &connection);




  std::string path_;
  RpcDBusFixture2 &implementation_;

};
