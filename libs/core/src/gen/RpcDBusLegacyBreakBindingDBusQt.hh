// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's QtDBus backend from an annotated C++ header.
#pragma once

#include <string>
#include <string_view>

#include <vector>

#include <boost/signals2/connection.hpp>


#include "rpc/dbus/QtInterface.hh"
#include "rpc/dbus/QtServer.hh"

#include "Break.hh"


namespace workrave::core::legacy_rpc
{

class org_workrave_BreakInterface final : public ::workrave::rpc::dbus::QtInterface
{
public:
  org_workrave_BreakInterface(::workrave::rpc::dbus::QtServer &server,
              std::string path,
              Break &implementation);
  ~org_workrave_BreakInterface() override = default;

  [[nodiscard]] std::string_view name() const noexcept override;
  [[nodiscard]] std::string_view introspection() const noexcept override;
  bool dispatch(const QDBusMessage &message, const QDBusConnection &connection) override;

private:

  void dispatch_GetName(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_IsEnabled(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_IsTimerRunning(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetTimerElapsed(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetTimerIdle(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetAutoReset(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_IsAutoResetEnabled(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetLimit(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_IsLimitEnabled(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_IsTaking(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_IsMaxPreludesReached(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_PostponeBreak(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_SkipBreak(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_IsActive(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetTimerRemaining(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetTimerOverdue(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetBreakState(const QDBusMessage &message, const QDBusConnection &connection);


  void emit_BreakEvent(workrave::BreakEvent value);

  void emit_BreakStateChanged(BreakStage value);



  ::workrave::rpc::dbus::QtServer &server_;

  std::string path_;
  Break &implementation_;

  std::vector<boost::signals2::scoped_connection> signal_connections_;

};

} // namespace workrave::core::legacy_rpc
