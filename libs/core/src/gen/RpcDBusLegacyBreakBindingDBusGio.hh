// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's GDBus backend from an annotated C++ header.
#pragma once

#include <string>
#include <string_view>

#include <vector>

#include <boost/signals2/connection.hpp>


#include "rpc/dbus/GioInterface.hh"
#include "rpc/dbus/GioServer.hh"

#include "Break.hh"


namespace workrave::core::legacy_rpc
{

class org_workrave_BreakInterface final : public ::workrave::rpc::dbus::GioInterface
{
public:
  org_workrave_BreakInterface(::workrave::rpc::dbus::GioServer &server,
              std::string path,
              Break &implementation);
  ~org_workrave_BreakInterface() override = default;

  [[nodiscard]] std::string_view name() const noexcept override;
  [[nodiscard]] std::string_view introspection() const noexcept override;
  void dispatch(std::string_view method,
                GVariant *parameters,
                GDBusMethodInvocation *invocation) override;

private:

  void dispatch_GetName(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_IsEnabled(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_IsTimerRunning(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetTimerElapsed(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetTimerIdle(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetAutoReset(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_IsAutoResetEnabled(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetLimit(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_IsLimitEnabled(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_IsTaking(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_IsMaxPreludesReached(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_PostponeBreak(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_SkipBreak(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_IsActive(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetTimerRemaining(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetTimerOverdue(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetBreakState(GVariant *parameters, GDBusMethodInvocation *invocation);


  void emit_BreakEvent(workrave::BreakEvent value);

  void emit_BreakStateChanged(BreakStage value);



  ::workrave::rpc::dbus::GioServer &server_;

  std::string path_;
  Break &implementation_;

  std::vector<boost::signals2::scoped_connection> signal_connections_;

};

} // namespace workrave::core::legacy_rpc
