// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's QtDBus backend from an annotated C++ header.
#pragma once

#include <string>
#include <string_view>

#include <vector>

#include <boost/signals2/connection.hpp>


#include "rpc/dbus/QtInterface.hh"
#include "rpc/dbus/QtServer.hh"

#include "Core.hh"


namespace workrave::core::rpc
{

class org_workrave_CoreInterface final : public ::workrave::rpc::dbus::QtInterface
{
public:
  org_workrave_CoreInterface(::workrave::rpc::dbus::QtServer &server,
              std::string path,
              Core &implementation);
  ~org_workrave_CoreInterface() override = default;

  [[nodiscard]] std::string_view name() const noexcept override;
  [[nodiscard]] std::string_view introspection() const noexcept override;
  bool dispatch(const QDBusMessage &message, const QDBusConnection &connection) override;

private:

  void dispatch_ForceBreak(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_IsActive(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_IsTaking(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetActiveOperationMode(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetOperationMode(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_IsOperationModeAnOverride(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_SetOperationMode(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_SetOperationModeFor(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetUsageMode(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_SetUsageMode(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_ReportActivity(const QDBusMessage &message, const QDBusConnection &connection);


  void emit_OperationModeChanged(workrave::OperationMode value);

  void emit_UsageModeChanged(workrave::UsageMode value);



  ::workrave::rpc::dbus::QtServer &server_;

  std::string path_;
  Core &implementation_;

  std::vector<boost::signals2::scoped_connection> signal_connections_;

};

} // namespace workrave::core::rpc
